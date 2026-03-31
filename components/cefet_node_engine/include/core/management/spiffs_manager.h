#pragma once

#include <string>
#include "esp_err.h"

namespace Cefet {

/**
 * @brief Gerenciador do Sistema de Arquivos da Memoria Flash (SPIFFS).
 *
 * Responsavel por montar a particao de dados interna do ESP32 e
 * prover metodos de leitura para os arquivos de manifesto JSON.
 */
class SpiffsManager {
public:
    /**
     * @brief Monta a particao SPIFFS no caminho virtual "/spiffs".
     *
     * @return esp_err_t ESP_OK em caso de sucesso.
     */
    static esp_err_t mount();

    /**
     * @brief Desmonta a particao SPIFFS.
     */
    static void unmount();

    /**
     * @brief Le o conteudo integral de um arquivo de texto.
     *
     * @param path Caminho absoluto do arquivo (ex: "/spiffs/config.json").
     * @return std::string Conteudo do arquivo (ou string vazia em caso de erro).
     */
    static std::string readFile(const std::string& path);
};

} // namespace Cefet