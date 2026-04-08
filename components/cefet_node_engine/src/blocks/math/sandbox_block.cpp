/**
 * @file sandbox_block.cpp
 * @brief Implementacao dinamica e hibrida da ponte C++ / Lua com suporte a Base64.
 */
#include "sandbox_block.h"
#include "block_registry.h"
#include "esp_log.h"

/* Inclusao da biblioteca nativa do ESP-IDF para decodificacao Base64 */
#include "mbedtls/base64.h"

namespace Cefet {

static const char* TAG = "SANDBOX_LUA";

SandboxBlock::SandboxBlock(const std::string& block_id, const std::string& script_b64, const std::string& script_path, size_t num_in, size_t num_out)
    : m_id(block_id), m_script_b64(script_b64), m_script_path(script_path), m_num_in(num_in), m_num_out(num_out), L(nullptr)
{
    /* Alocacao vetorial baseada nas dimensoes pedidas no manifesto JSON */
    m_inputs.resize(m_num_in, nullptr);
    m_outputs.resize(m_num_out, 0.0f);
}

SandboxBlock::~SandboxBlock()
{
    if (L != nullptr) {
        lua_close(L);
    }
}

bool SandboxBlock::initialize()
{
    L = luaL_newstate();
    if (L == nullptr) {
        ESP_LOGE(TAG, "[%s] Falha critica: Impossivel alocar memoria para a VM Lua.", m_id.c_str());
        return false;
    }

    luaL_openlibs(L);

    /* ========================================================================= */
    /* ESTRATEGIA HIBRIDA DE CARREGAMENTO DE SCRIPT                              */
    /* ========================================================================= */

    /* 1. Tenta decodificar e ler o codigo Base64 direto da RAM primeiro (Producao/IHM) */
    if (!m_script_b64.empty()) {
        ESP_LOGI(TAG, "[%s] Decodificando script Base64 a partir do JSON (IHM).", m_id.c_str());
        
        size_t b64_len = m_script_b64.length();
        unsigned char* decoded_buffer = new (std::nothrow) unsigned char[b64_len + 1];

        if (decoded_buffer == nullptr) {
            ESP_LOGE(TAG, "[%s] Falha ao alocar memoria para decodificacao Base64.", m_id.c_str());
            return false;
        }

        size_t actual_out_len = 0;
        int ret = mbedtls_base64_decode(decoded_buffer, b64_len, &actual_out_len,
                                        (const unsigned char*)m_script_b64.c_str(), b64_len);

        if (ret == 0) {
            decoded_buffer[actual_out_len] = '\0'; /* Garante terminacao da string C */
            
            if (luaL_dostring(L, (const char*)decoded_buffer) != LUA_OK) {
                ESP_LOGE(TAG, "[%s] Erro de sintaxe Lua no codigo Base64: %s", m_id.c_str(), lua_tostring(L, -1));
                lua_pop(L, 1);
                delete[] decoded_buffer;
                return false;
            }
        } else {
            ESP_LOGE(TAG, "[%s] Falha na decodificacao Base64. Codigo mbedtls: %d", m_id.c_str(), ret);
            delete[] decoded_buffer;
            return false;
        }
        delete[] decoded_buffer;
    } 
    /* 2. Fallback: Se nao houver codigo na RAM, le o ficheiro fisico do SPIFFS (Desenvolvimento) */
    else {
        ESP_LOGI(TAG, "[%s] Compilando script a partir do ficheiro de fallback: %s", m_id.c_str(), m_script_path.c_str());
        if (luaL_dofile(L, m_script_path.c_str()) != LUA_OK) {
            ESP_LOGE(TAG, "[%s] Erro ao compilar o ficheiro Lua: %s", m_id.c_str(), lua_tostring(L, -1));
            lua_pop(L, 1);
            return false;
        }
    }

    ESP_LOGI(TAG, "[%s] Maquina Virtual Lua pronta. Portas IN: %zu | Portas OUT: %zu", m_id.c_str(), m_num_in, m_num_out);
    return true;
}

std::string SandboxBlock::getId() const
{
    return m_id;
}

/* ========================================================================= */
/* ALOCACAO DINAMICA DE PORTAS DE COMUNICACAO                                */
/* ========================================================================= */

void* SandboxBlock::getDataOutput(const std::string& port_name)
{
    if (port_name.find("OUT_") == 0) {
        size_t idx = std::stoi(port_name.substr(4));
        if (idx < m_num_out) {
            return &m_outputs[idx];
        }
    }
    return nullptr;
}

bool SandboxBlock::connectDataInput(const std::string& port_name, void* data_pointer)
{
    if (port_name.find("IN_") == 0) {
        size_t idx = std::stoi(port_name.substr(3));
        if (idx < m_num_in) {
            m_inputs[idx] = static_cast<float*>(data_pointer);
            return true;
        }
    }
    return false;
}

/* ========================================================================= */
/* NUCLEO DE EXECUCAO DINAMICA DO LUA                                        */
/* ========================================================================= */

void SandboxBlock::triggerEventInput(const std::string& event_name)
{
    if (event_name == "REQ" && L != nullptr) {
        
        lua_getglobal(L, "tick");

        if (!lua_isfunction(L, -1)) {
            ESP_LOGW(TAG, "[%s] A rotina 'tick' nao foi encontrada no script.", m_id.c_str());
            lua_pop(L, 1);
            return;
        }

        /* Empilha dinamicamente as variaveis de entrada configuradas */
        for (size_t i = 0; i < m_num_in; ++i) {
            float val = (m_inputs[i] != nullptr) ? *(m_inputs[i]) : 0.0f;
            lua_pushnumber(L, val);
        }

        /* Executa a maquina virtual com o numero exato de argumentos */
        if (lua_pcall(L, m_num_in, m_num_out, 0) != LUA_OK) {
            ESP_LOGE(TAG, "[%s] Erro na execucao do tick(): %s", m_id.c_str(), lua_tostring(L, -1));
            lua_pop(L, 1);
            return;
        }

        /* Recupera os resultados gerados pelo script na ordem inversa da pilha */
        for (int i = static_cast<int>(m_num_out) - 1; i >= 0; --i) {
            m_outputs[i] = static_cast<float>(lua_tonumber(L, -1));
            lua_pop(L, 1);
        }

        emitEvent("CNF");
    }
}

/* ========================================================================= */
/* FACTORY METHOD E DESSERIALIZACAO DO JSON                                  */
/* ========================================================================= */

IFunctionBlock* SandboxBlock::create(const std::string& block_id, cJSON* config)
{
    std::string script_b64 = "";
    std::string script_path = "/spiffs/script.lua"; 
    size_t in_ports = 4;  /* Valor padrao de seguranca */
    size_t out_ports = 4; /* Valor padrao de seguranca */
    
    if (config != nullptr) {
        /* 1. Extrai a string Base64 enviada pela futura IHM Web */
        cJSON* b64_item = cJSON_GetObjectItem(config, "script_b64");
        if (cJSON_IsString(b64_item) && b64_item->valuestring != nullptr) {
            script_b64 = b64_item->valuestring;
        }

        /* 2. Verifica o caminho de fallback do ficheiro */
        cJSON* path_item = cJSON_GetObjectItem(config, "script_path");
        if (cJSON_IsString(path_item) && path_item->valuestring != nullptr) {
            script_path = path_item->valuestring;
        }

        /* 3. Dimensionamento das portas */
        cJSON* in_item = cJSON_GetObjectItem(config, "num_in");
        if (cJSON_IsNumber(in_item)) {
            in_ports = static_cast<size_t>(in_item->valueint);
        }

        cJSON* out_item = cJSON_GetObjectItem(config, "num_out");
        if (cJSON_IsNumber(out_item)) {
            out_ports = static_cast<size_t>(out_item->valueint);
        }
    }
    
    return new SandboxBlock(block_id, script_b64, script_path, in_ports, out_ports);
}

/**
 * @brief Registo estatico na Factory.
 */
static bool registered = []() {
    BlockRegistry::registerBlock("Sandbox", SandboxBlock::create);
    return true;
}();

} /* namespace Cefet */