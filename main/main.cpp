#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "cefet_node_engine.h"
#include "network_manager.h"
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

    static Cefet::AnalogInputBlock adc_sensor("ADC_TACO", ADC_UNIT_1, ADC_CHANNEL_4);
    adc_sensor.initialize();

    // COLOQUE O IP DO SEU COMPUTADOR AQUI!
    static Cefet::UdpPublisherBlock udp_pub("PUB_UDP", "10.219.254.209", 5000); 
    udp_pub.initialize();

    static ControlLoopContext loop_ctx = {&adc_sensor, &udp_pub};
    Cefet::CefetEngine::subscribeEvent(Cefet::EV_SENSOR_DATA_READY, onReadAndPublish, &loop_ctx);

    // Rodando a 10Hz (100ms) para vermos a velocidade do UDP
    static Cefet::ECycleBlock master_clock("CLOCK_MALHA", 100, Cefet::EV_SENSOR_DATA_READY);
    master_clock.initialize();
    master_clock.startTimer();

    while (1) {
        vTaskDelay(portMAX_DELAY);
    }
}