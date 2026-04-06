#include "connection_manager.h"
#include "cJSON.h"
#include "esp_log.h"
#include <sstream>

namespace Cefet {

static const char* TAG = "CONNECTION_MANAGER";

/**
 * @brief Funcao auxiliar para buscar um bloco na RAM pelo seu ID.
 */
static IFunctionBlock* findBlock(const std::vector<IFunctionBlock*>& blocks, const std::string& id) {
    for (auto* b : blocks) {
        if (b->getId() == id) {
            return b;
        }
    }
    return nullptr;
}

bool ConnectionManager::wireConnections(const std::string& json_payload, const std::vector<IFunctionBlock*>& blocks) {
    cJSON* root = cJSON_Parse(json_payload.c_str());
    if (root == nullptr) {
        ESP_LOGE(TAG, "Falha na conversao do JSON para o roteamento.");
        return false;
    }

    cJSON* conns = cJSON_GetObjectItem(root, "connections");
    if (!cJSON_IsArray(conns)) {
        ESP_LOGW(TAG, "O manifesto nao contem um array 'connections'. Roteamento abortado.");
        cJSON_Delete(root);
        return false;
    }

    ESP_LOGI(TAG, "Iniciando Roteamento (Wiring) IEC 61499...");

    cJSON* conn = nullptr;
    cJSON_ArrayForEach(conn, conns) {
        cJSON* src_node = cJSON_GetObjectItem(conn, "source");
        cJSON* tgt_node = cJSON_GetObjectItem(conn, "target");

        if (!cJSON_IsString(src_node) || !cJSON_IsString(tgt_node)) {
            continue;
        }

        std::string source_full = src_node->valuestring;
        std::string target_full = tgt_node->valuestring;

        // Procura pelo ponto (.) que separa o nome do bloco do nome da porta
        size_t src_dot = source_full.find('.');
        size_t tgt_dot = target_full.find('.');

        if (src_dot == std::string::npos || tgt_dot == std::string::npos) {
            ESP_LOGE(TAG, "Erro de sintaxe na conexao (Falta o ponto): %s -> %s", source_full.c_str(), target_full.c_str());
            continue;
        }

        std::string src_id = source_full.substr(0, src_dot);
        std::string src_port = source_full.substr(src_dot + 1);

        std::string tgt_id = target_full.substr(0, tgt_dot);
        std::string tgt_port = target_full.substr(tgt_dot + 1);

        IFunctionBlock* src_block = findBlock(blocks, src_id);
        IFunctionBlock* tgt_block = findBlock(blocks, tgt_id);

        if (src_block == nullptr || tgt_block == nullptr) {
            ESP_LOGE(TAG, "Bloco ausente na placa. Falha ao plugar: %s -> %s", source_full.c_str(), target_full.c_str());
            continue;
        }

        // Tenta conectar como DADOS primeiro
        void* data_ptr = src_block->getDataOutput(src_port);
        if (data_ptr != nullptr) {
            if (tgt_block->connectDataInput(tgt_port, data_ptr)) {
                ESP_LOGI(TAG, "Fio de DADOS ligado: [%s].%s ---> [%s].%s", src_id.c_str(), src_port.c_str(), tgt_id.c_str(), tgt_port.c_str());
            } else {
                ESP_LOGE(TAG, "A porta de entrada de dados %s rejeitou a conexao.", target_full.c_str());
            }
        } else {
            // Se o bloco nao cuspiu um ponteiro de dados, e uma conexao de EVENTO
            src_block->connectEventOutput(src_port, tgt_block, tgt_port);
            ESP_LOGI(TAG, "Fio de EVENTO ligado: [%s].%s ---> [%s].%s", src_id.c_str(), src_port.c_str(), tgt_id.c_str(), tgt_port.c_str());
        }
    }

    cJSON_Delete(root);
    return true;
}

} // namespace Cefet