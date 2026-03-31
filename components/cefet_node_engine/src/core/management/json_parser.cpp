#include "json_parser.h"
#include "block_registry.h"
#include "cJSON.h"
#include "esp_log.h"

namespace Cefet {

static const char* TAG = "JSON_PARSER";

std::vector<IFunctionBlock*> JsonParser::parseManifest(const std::string& json_payload)
{
    std::vector<IFunctionBlock*> instantiated_blocks;

    cJSON* root = cJSON_Parse(json_payload.c_str());
    if (root == nullptr) {
        ESP_LOGE(TAG, "Falha na conversao do JSON. Sintaxe invalida.");
        return instantiated_blocks;
    }

    cJSON* blocks_array = cJSON_GetObjectItem(root, "blocks");
    if (cJSON_IsArray(blocks_array)) {
        cJSON* block_item = nullptr;
        
        cJSON_ArrayForEach(block_item, blocks_array) {
            cJSON* id_obj = cJSON_GetObjectItem(block_item, "id");
            cJSON* type_obj = cJSON_GetObjectItem(block_item, "type");
            cJSON* config_obj = cJSON_GetObjectItem(block_item, "config");

            if (cJSON_IsString(id_obj) && cJSON_IsString(type_obj)) {
                std::string block_id = id_obj->valuestring;
                std::string block_type = type_obj->valuestring;

                ESP_LOGI(TAG, "Requisitando criacao do bloco: [%s] do tipo <%s>", block_id.c_str(), block_type.c_str());

                IFunctionBlock* new_block = BlockRegistry::createBlock(block_type, block_id, config_obj);
                
                if (new_block != nullptr) {
                    if (new_block->initialize()) {
                        instantiated_blocks.push_back(new_block);
                    } else {
                        ESP_LOGE(TAG, "Falha de hardware/inicializacao no bloco [%s]. Abortando instancia.", block_id.c_str());
                        delete new_block;
                    }
                }
            } else {
                ESP_LOGW(TAG, "Entrada de bloco malformada no JSON. Ignorando.");
            }
        }
    } else {
        ESP_LOGW(TAG, "O manifesto nao contem um array 'blocks' valido.");
    }

    cJSON_Delete(root);
    return instantiated_blocks;
}

} // namespace Cefet