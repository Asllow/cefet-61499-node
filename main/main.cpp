#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cefet_node_engine.h"
#include "network_manager.h"

extern "C" void app_main(void)
{
    // 1. Inicializa a infraestrutura base (Eventos e Pilha TCP/IP)
    Cefet::CefetEngine::start();
    Cefet::NetworkManager::connect();

    // 2. Transfere o controle para o Motor carregar a Planta Fisica
    Cefet::CefetEngine::startFromManifest("/spiffs/config.json");

    // 3. O FreeRTOS mantem a placa viva enquanto os eventos acontecem em background
    while (1) {
        vTaskDelay(portMAX_DELAY);
    }
}