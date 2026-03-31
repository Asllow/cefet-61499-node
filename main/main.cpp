#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "cefet_node_engine.h"
#include "network_manager.h"
#include "json_parser.h"
#include "spiffs_manager.h"
#include "analog_input_block.h"
#include "udp_publisher_block.h"
#include "e_cycle_block.h"

static const char* TAG = "MAIN_APP";

struct ControlLoopContext {
    Cefet::AnalogInputBlock* adc_sensor;
    Cefet::UdpPublisherBlock* udp_publisher;
};

static void onReadAndPublish(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    auto* ctx = static_cast<ControlLoopContext*>(handler_args);
    if (!ctx->adc_sensor || !ctx->udp_publisher) return;
    
    int raw_val = 0;
    if (ctx->adc_sensor->readRaw(&raw_val)) {
        ESP_LOGI(TAG, "Leitura: %d. Disparando UDP...", raw_val);
        ctx->udp_publisher->publish(std::to_string(raw_val));
    }
}

extern "C" void app_main(void)
{
    Cefet::CefetEngine::start();
    Cefet::NetworkManager::connect();

    // 1. Monta o sistema de arquivos
    if (Cefet::SpiffsManager::mount() != ESP_OK) {
        ESP_LOGE(TAG, "Parando execucao. Sistema de arquivos inoperante.");
        return;
    }

    // 2. Le o arquivo JSON gravado fisicamente na placa
    std::string manifest = Cefet::SpiffsManager::readFile("/spiffs/config.json");
    if (manifest.empty()) {
        ESP_LOGE(TAG, "Manifesto vazio ou nao encontrado.");
        return;
    }

    ESP_LOGI(TAG, "Processando Manifesto JSON...");
    auto blocks = Cefet::JsonParser::parseManifest(manifest);

    static ControlLoopContext loop_ctx = {nullptr, nullptr};
    Cefet::ECycleBlock* clock_block = nullptr;

    for (auto* block : blocks) {
        if (block->getId() == "ADC_TACO") {
            loop_ctx.adc_sensor = static_cast<Cefet::AnalogInputBlock*>(block);
        } else if (block->getId() == "PUB_UDP") {
            loop_ctx.udp_publisher = static_cast<Cefet::UdpPublisherBlock*>(block);
        } else if (block->getId() == "CLOCK_MALHA") {
            clock_block = static_cast<Cefet::ECycleBlock*>(block);
        }
    }

    if (loop_ctx.adc_sensor && loop_ctx.udp_publisher && clock_block) {
        Cefet::CefetEngine::subscribeEvent(Cefet::EV_SENSOR_DATA_READY, onReadAndPublish, &loop_ctx);
        clock_block->startTimer();
        ESP_LOGI(TAG, "Malha instanciada via config.json com sucesso!");
    } else {
        ESP_LOGE(TAG, "Falha ao rotear a malha. Blocos ausentes no manifesto.");
    }

    while (1) {
        vTaskDelay(portMAX_DELAY);
    }
}