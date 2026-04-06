#include "modbus_tcp_server_block.h"
#include "block_registry.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include <cstring>

namespace Cefet {

static const char* TAG = "MODBUS_NATIVO";

ModbusTcpServerBlock::ModbusTcpServerBlock(const std::string& block_id, uint16_t port)
    : m_id(block_id), m_port(port), m_listen_sock(-1), m_server_task(nullptr),
      m_reg_in_0(nullptr), m_reg_in_1(nullptr), m_reg_out_0(0), m_reg_out_1(0)
{
    memset(m_holding_registers, 0, sizeof(m_holding_registers));
}

ModbusTcpServerBlock::~ModbusTcpServerBlock()
{
    if (m_server_task != nullptr) {
        vTaskDelete(m_server_task);
    }
    if (m_listen_sock != -1) {
        close(m_listen_sock);
    }
}

bool ModbusTcpServerBlock::initialize()
{
    m_listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (m_listen_sock < 0) {
        ESP_LOGE(TAG, "[%s] Falha ao criar socket TCP.", m_id.c_str());
        return false;
    }

    int opt = 1;
    setsockopt(m_listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in dest_addr = {};
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_port = htons(m_port);

    if (bind(m_listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != 0) {
        ESP_LOGE(TAG, "[%s] Falha no bind da porta %d.", m_id.c_str(), m_port);
        close(m_listen_sock);
        return false;
    }

    if (listen(m_listen_sock, 1) != 0) {
        ESP_LOGE(TAG, "[%s] Falha ao colocar socket em modo listen.", m_id.c_str());
        return false;
    }

    BaseType_t task_created = xTaskCreate(
        &ModbusTcpServerBlock::serverTask, 
        m_id.c_str(), 
        4096, 
        this, 
        5, 
        &m_server_task
    );

    if (task_created != pdPASS) {
        ESP_LOGE(TAG, "[%s] Falha ao alocar a Task do Servidor.", m_id.c_str());
        return false;
    }
    
    ESP_LOGI(TAG, "[%s] Servidor Frugal Modbus TCP escutando na porta %d", m_id.c_str(), m_port);
    return true;
}

std::string ModbusTcpServerBlock::getId() const 
{ 
    return m_id; 
}

// =========================================================================
// IMPLEMENTACAO DAS PORTAS IEC 61499
// =========================================================================

void* ModbusTcpServerBlock::getDataOutput(const std::string& port_name)
{
    if (port_name == "REG_OUT_0") return &m_reg_out_0;
    if (port_name == "REG_OUT_1") return &m_reg_out_1;
    return nullptr;
}

bool ModbusTcpServerBlock::connectDataInput(const std::string& port_name, void* data_pointer)
{
    if (port_name == "REG_IN_0") { 
        m_reg_in_0 = static_cast<int*>(data_pointer); 
        return true; 
    }
    if (port_name == "REG_IN_1") { 
        m_reg_in_1 = static_cast<int*>(data_pointer); 
        return true; 
    }
    return false;
}

void ModbusTcpServerBlock::triggerEventInput(const std::string& event_name)
{
    if (event_name == "UPDATE") {
        // 1. Atualiza os Holding Registers com as leituras físicas mais recentes
        if (m_reg_in_0 != nullptr) m_holding_registers[0] = static_cast<uint16_t>(*m_reg_in_0);
        if (m_reg_in_1 != nullptr) m_holding_registers[1] = static_cast<uint16_t>(*m_reg_in_1);
        
        // 2. Transfere comandos Modbus escritos pelo Mestre para os pinos de saída virtuais
        m_reg_out_0 = static_cast<int>(m_holding_registers[2]);
        m_reg_out_1 = static_cast<int>(m_holding_registers[3]);
        
        // 3. Sinaliza aos blocos a jusante que os dados estão prontos
        emitEvent("CNF");
    }
}

// =========================================================================
// NUCLEO DA COMUNICACAO (PARSEAMENTO FRUGAL DO MBAP)
// =========================================================================

void ModbusTcpServerBlock::serverTask(void* arg)
{
    auto* instance = static_cast<ModbusTcpServerBlock*>(arg);
    while (1) {
        struct sockaddr_in source_addr;
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(instance->m_listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        
        if (sock < 0) continue;
        ESP_LOGI(TAG, "Mestre Modbus Conectado!");

        while (1) {
            uint8_t rx[256];
            int len = recv(sock, rx, sizeof(rx), 0);
            
            if (len <= 0) {
                break; // Conexão perdida ou encerrada pelo Mestre
            }

            // Validação mínima do Cabeçalho MBAP (Modbus Application Protocol)
            // MBAP Header = [0..1] TransID, [2..3] Protocol(0), [4..5] Len, [6] UnitID
            if (len >= 12 && rx[2] == 0 && rx[3] == 0) {
                uint8_t func = rx[7]; // Código da Função Modbus
                
                // -----------------------------------------------------------
                // Função 03: Read Holding Registers
                // -----------------------------------------------------------
                if (func == 3) {
                    uint16_t start_addr = (rx[8] << 8) | rx[9];
                    uint16_t quant = (rx[10] << 8) | rx[11];
                    
                    if (quant > 0 && quant <= 4) {
                        uint8_t tx[256];
                        memcpy(tx, rx, 7); // Copia o TransID e UnitID originais
                        
                        uint16_t out_len = 3 + (quant * 2);
                        tx[4] = out_len >> 8; 
                        tx[5] = out_len & 0xFF;
                        tx[7] = 3; // Mantém Função 03
                        tx[8] = quant * 2; // Byte count
                        
                        for (int i = 0; i < quant; i++) {
                            uint16_t val = (start_addr + i < 4) ? instance->m_holding_registers[start_addr + i] : 0;
                            tx[9 + i*2] = val >> 8;
                            tx[10 + i*2] = val & 0xFF;
                        }
                        send(sock, tx, 9 + (quant * 2), 0);
                    }
                } 
                // -----------------------------------------------------------
                // Função 16: Write Multiple Registers
                // -----------------------------------------------------------
                else if (func == 16) {
                    uint16_t start_addr = (rx[8] << 8) | rx[9];
                    uint16_t quant = (rx[10] << 8) | rx[11];
                    
                    if (quant > 0 && quant <= 4) {
                        for (int i = 0; i < quant; i++) {
                            if (start_addr + i < 4) {
                                instance->m_holding_registers[start_addr + i] = (rx[13 + i*2] << 8) | rx[14 + i*2];
                            }
                        }
                        uint8_t tx[12];
                        memcpy(tx, rx, 12); // A resposta é um eco dos primeiros 12 bytes
                        tx[4] = 0; 
                        tx[5] = 6;  // O tamanho do pacote de resposta da Func 16 é sempre 6
                        send(sock, tx, 12, 0);
                    }
                }
            }
        }
        close(sock);
        ESP_LOGI(TAG, "Mestre Modbus Desconectado.");
    }
}

// =========================================================================

IFunctionBlock* ModbusTcpServerBlock::create(const std::string& block_id, cJSON* config) 
{
    uint16_t port = 502; // Porta padrão Modbus TCP

    if (config != nullptr) {
        cJSON* port_item = cJSON_GetObjectItem(config, "port");
        if (cJSON_IsNumber(port_item)) {
            port = static_cast<uint16_t>(port_item->valueint);
        }
    }
    
    return new ModbusTcpServerBlock(block_id, port);
}

/**
 * @brief Registo Estático do Bloco.
 * Garante que a Factory reconhece o "ModbusTcpServer" ao ler o JSON.
 */
static bool registered = []() {
    BlockRegistry::registerBlock("ModbusTcpServer", ModbusTcpServerBlock::create);
    return true;
}();

} // namespace Cefet