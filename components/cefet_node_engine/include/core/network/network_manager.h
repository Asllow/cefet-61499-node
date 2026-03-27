#pragma once

#include "esp_err.h"
#include "esp_event.h"

namespace Cefet {

/**
 * @brief Gerenciador da Pilha de Conectividade do No.
 *
 * Encapsula a inicializacao da memoria NVS, da interface de rede (esp_netif)
 * e do driver Wi-Fi nativo do ESP32. Opera de forma assincrona e publica
 * o evento EV_NETWORK_CONNECTED no barramento central quando o IP e obtido.
 */
class NetworkManager {
public:
    /**
     * @brief Inicializa o hardware Wi-Fi e inicia a tentativa de conexao.
     * Utiliza as credenciais definidas via menuconfig.
     *
     * @return esp_err_t ESP_OK se o processo de conexao foi iniciado com sucesso.
     */
    static esp_err_t connect();

private:
    /**
     * @brief Callback interna para tratamento de eventos nativos do driver Wi-Fi.
     *
     * @param arg Argumentos de contexto.
     * @param event_base Familia do evento (WIFI_EVENT ou IP_EVENT).
     * @param event_id Identificador especifico do evento.
     * @param event_data Dados associados (ex: IP recebido).
     */
    static void wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
};

} // namespace Cefet