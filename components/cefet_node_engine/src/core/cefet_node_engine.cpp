#include "cefet_node_engine.h"
#include "esp_log.h"
#include "sdkconfig.h"

// Inclusões transferidas da main.cpp para o encapsulamento:
#include "spiffs_manager.h"
#include "json_parser.h"
#include "connection_manager.h"
#include "e_cycle_block.h"

namespace Cefet {

ESP_EVENT_DEFINE_BASE(CEFET_CORE_EVENTS);

static const char* TAG = "CEFET_ENGINE";

esp_err_t CefetEngine::start() {
    setupTelemetry();

    ESP_LOGI(TAG, "Inicializando o Motor de Eventos IEC-61499...");

    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK) {
        if (err == ESP_ERR_INVALID_STATE) {
            ESP_LOGW(TAG, "Loop de eventos ja estava rodando.");
        } else {
            ESP_LOGE(TAG, "Falha critica ao criar o Event Loop!");
            return err;
        }
    }

    postEvent(EV_SYSTEM_BOOT);

    ESP_LOGI(TAG, "Motor inicializado e aguardando manifestos (JSON).");
    return ESP_OK;
}

esp_err_t CefetEngine::startFromManifest(const std::string& manifest_path) {
    if (SpiffsManager::mount() != ESP_OK) {
        ESP_LOGE(TAG, "Parando execucao. Sistema de arquivos inoperante.");
        return ESP_FAIL;
    }

    std::string manifest = SpiffsManager::readFile(manifest_path);
    if (manifest.empty()) {
        ESP_LOGE(TAG, "Manifesto vazio ou nao encontrado: %s", manifest_path.c_str());
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Processando Manifesto JSON...");
    auto blocks = JsonParser::parseManifest(manifest);

    if (ConnectionManager::wireConnections(manifest, blocks)) {
        ESP_LOGI(TAG, "Cabeamento da malha (Wiring) concluido com sucesso!");
    } else {
        ESP_LOGE(TAG, "Falha ao rotear a malha.");
        return ESP_FAIL;
    }

    // Busca o bloco de Clock para dar o Play inicial da malha
    for (auto* block : blocks) {
        if (block->getId() == "CLOCK_MALHA") {
            auto* clock = static_cast<ECycleBlock*>(block);
            clock->startTimer();
            break;
        }
    }

    return ESP_OK;
}

esp_err_t CefetEngine::postEvent(EventIds event_id, void* event_data, size_t event_data_size) {
    return esp_event_post(CEFET_CORE_EVENTS, event_id, event_data, event_data_size, portMAX_DELAY);
}

esp_err_t CefetEngine::subscribeEvent(EventIds event_id, esp_event_handler_t event_handler, void* event_handler_arg) {
    return esp_event_handler_register(CEFET_CORE_EVENTS, event_id, event_handler, event_handler_arg);
}

void CefetEngine::setupTelemetry() {
#if defined(CONFIG_CEFET_LOG_MODE_DISABLED)
    esp_log_level_set("*", ESP_LOG_NONE);
#elif defined(CONFIG_CEFET_LOG_MODE_NETWORK)
    esp_log_set_vprintf(&CefetEngine::networkLogRoute);
#endif
}

int CefetEngine::networkLogRoute(const char* fmt, va_list args) {
    return vprintf(fmt, args);
}

} // namespace Cefet