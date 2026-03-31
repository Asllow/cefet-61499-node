#include "spiffs_manager.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include <fstream>
#include <sstream>

namespace Cefet {

static const char* TAG = "SPIFFS_MANAGER";

esp_err_t SpiffsManager::mount()
{
    ESP_LOGI(TAG, "Inicializando SPIFFS...");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Falha ao montar ou formatar o sistema de arquivos.");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Particao SPIFFS nao encontrada na tabela de particoes.");
        } else {
            ESP_LOGE(TAG, "Falha na inicializacao do SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Falha ao obter informacoes da particao SPIFFS (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Tamanho da particao: total: %d bytes, usado: %d bytes", total, used);
    }

    return ESP_OK;
}

void SpiffsManager::unmount()
{
    esp_vfs_spiffs_unregister(NULL);
    ESP_LOGI(TAG, "SPIFFS desmontado.");
}

std::string SpiffsManager::readFile(const std::string& path)
{
    ESP_LOGI(TAG, "Lendo arquivo: %s", path.c_str());
    std::ifstream file(path);
    
    if (!file.is_open()) {
        ESP_LOGE(TAG, "Falha ao abrir o arquivo: %s", path.c_str());
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}

} // namespace Cefet