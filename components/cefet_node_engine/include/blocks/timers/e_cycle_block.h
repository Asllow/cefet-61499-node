#pragma once

#include <string>
#include "esp_timer.h"
#include "i_function_block.h"
#include "cefet_events.h"

namespace Cefet {

/**
 * @brief Bloco de Evento Ciclico (E_CYCLE) da norma IEC 61499.
 *
 * Utiliza o temporizador de alta resolucao do ESP-IDF para gerar
 * eventos periodicos no barramento central. Essencial para iniciar
 * cadeias de execucao, como a amostragem periodica de sensores 
 * ou rotinas de calculo de malhas de controle PID.
 */
class ECycleBlock : public IFunctionBlock {
public:
    /**
     * @brief Construtor do gerador de eventos ciclicos.
     *
     * @param block_id Identificacao unica do bloco na rede.
     * @param period_ms Periodo de disparo do evento em milissegundos.
     * @param target_event ID do evento que sera publicado no barramento a cada ciclo.
     */
    ECycleBlock(const std::string& block_id, uint64_t period_ms, EventIds target_event);

    /**
     * @brief Destrutor padrao. Para o temporizador e libera recursos.
     */
    ~ECycleBlock() override;

    /**
     * @brief Inicializa e aloca os recursos do temporizador de hardware.
     *
     * @return true Se alocado com sucesso.
     * @return false Se os recursos do sistema estiverem esgotados.
     */
    bool initialize() override;

    /**
     * @brief Recupera a identificacao do bloco.
     *
     * @return std::string contendo o ID configurado.
     */
    std::string getId() const override;

    /**
     * @brief Inicia a contagem do temporizador.
     */
    void startTimer();

    /**
     * @brief Interrompe a contagem do temporizador.
     */
    void stopTimer();

private:
    std::string m_id;
    uint64_t m_period_ms;
    EventIds m_target_event;
    esp_timer_handle_t m_timer_handle;

    /**
     * @brief Callback estatica exigida pela API esp_timer do ESP-IDF.
     *
     * @param arg Ponteiro opaco (void*) que recebe a propria instancia (this) da classe.
     */
    static void timerCallback(void* arg);
};

} // namespace Cefet