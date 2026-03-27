#pragma once

#include <string>
#include "i_function_block.h"
#include "driver/ledc.h"

namespace Cefet {

/**
 * @brief Bloco Funcional de Saida PWM (SIFB).
 *
 * Encapsula o driver LEDC do ESP-IDF v6.0 para geracao de sinais PWM
 * em hardware. Utilizado para acionamento de atuadores de potencia,
 * como motores DC via Ponte H, inversores de frequencia ou aquecedores.
 */
class PwmOutputBlock : public IFunctionBlock {
public:
    /**
     * @brief Construtor do bloco de saida PWM.
     *
     * @param block_id Identificador unico do bloco na rede.
     * @param gpio_num Pino fisico do ESP32 onde o sinal PWM sera gerado.
     * @param timer_num Temporizador de hardware alocado para este sinal (ex: LEDC_TIMER_0).
     * @param channel_num Canal de controle PWM alocado (ex: LEDC_CHANNEL_0).
     * @param freq_hz Frequencia base do sinal PWM em Hertz.
     */
    PwmOutputBlock(const std::string& block_id, int gpio_num, ledc_timer_t timer_num, ledc_channel_t channel_num, uint32_t freq_hz);

    /**
     * @brief Destrutor padrao.
     */
    ~PwmOutputBlock() override;

    /**
     * @brief Inicializa o periférico LEDC e configura temporizador e canal.
     *
     * @return true Se o driver foi configurado com sucesso.
     * @return false Se ocorreu falha na alocacao do hardware.
     */
    bool initialize() override;

    /**
     * @brief Recupera a identificacao do bloco.
     *
     * @return std::string contendo o ID configurado no construtor.
     */
    std::string getId() const override;

    /**
     * @brief Atualiza o ciclo de trabalho (Duty Cycle) do sinal PWM.
     *
     * @param duty_cycle Valor do duty cycle (0 a 8191 para resolucao de 13 bits).
     * @return true Se a atualizacao foi aplicada ao hardware com sucesso.
     * @return false Se ocorreu erro na comunicacao com os registradores.
     */
    bool writePwm(uint32_t duty_cycle);

private:
    std::string m_id;
    int m_gpio_num;
    ledc_timer_t m_timer_num;
    ledc_channel_t m_channel_num;
    uint32_t m_freq_hz;
};

} // namespace Cefet