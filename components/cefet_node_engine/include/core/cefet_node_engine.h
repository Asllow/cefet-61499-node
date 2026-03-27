#pragma once

#include "esp_err.h"
#include "cefet_events.h"
#include <cstdarg>

namespace Cefet {

/**
 * @brief Motor Principal do Framework CEFET-61499.
 */
class CefetEngine {
public:
    /**
     * @brief Inicializa o motor de eventos e prepara a infraestrutura.
     *
     * @return esp_err_t ESP_OK se o motor iniciou com sucesso.
     */
    static esp_err_t start();

    /**
     * @brief Publica um evento no barramento interno.
     *
     * @param event_id ID do evento.
     * @param event_data Ponteiro para dados (opcional).
     * @param event_data_size Tamanho dos dados.
     * @return esp_err_t ESP_OK em caso de sucesso.
     */
    static esp_err_t postEvent(EventIds event_id, void* event_data = nullptr, size_t event_data_size = 0);

    /**
     * @brief Cria uma conexao de roteamento (Wiring) entre um evento e um bloco.
     *
     * @param event_id O ID do evento a ser monitorado.
     * @param event_handler A funcao estatica que sera executada quando o evento ocorrer.
     * @param event_handler_arg Ponteiro para a instancia do bloco (this) que recebera a acao.
     * @return esp_err_t ESP_OK se a conexao foi estabelecida.
     */
    static esp_err_t subscribeEvent(EventIds event_id, esp_event_handler_t event_handler, void* event_handler_arg);

private:
    /**
     * @brief Configura o roteamento de logs com base no Kconfig.
     */
    static void setupTelemetry();

    /**
     * @brief Interceptador de logs para transmissao em rede.
     *
     * @param fmt String de formatacao padrao C.
     * @param args Argumentos variadicos vinculados a string.
     * @return int Numero de caracteres processados.
     */
    static int networkLogRoute(const char* fmt, va_list args);
};

} // namespace Cefet