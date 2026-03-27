#pragma once

#include <string>
#include "i_function_block.h"
#include "driver/mcpwm_prelude.h"

namespace Cefet {

/**
 * @brief Bloco Funcional de Controle de Motor DC (SIFB).
 *
 * Encapsula a API moderna do MCPWM para acionamento de Pontes H.
 * Gerencia automaticamente os sinais complementares de direcao e
 * a largura de pulso, isolando a logica de controle da complexidade
 * de alocacao de hardware do ESP-IDF v6.0.
 */
class McpwmMotorBlock : public IFunctionBlock {
public:
    /**
     * @brief Construtor do bloco de motor MCPWM.
     *
     * @param block_id Identificador unico do bloco na rede.
     * @param gpio_pwm_a Pino GPIO conectado a entrada IN1/PWM_R da Ponte H.
     * @param gpio_pwm_b Pino GPIO conectado a entrada IN2/PWM_L da Ponte H.
     * @param freq_hz Frequencia de comutacao em Hertz (ex: 20000 para 20kHz).
     */
    McpwmMotorBlock(const std::string& block_id, int gpio_pwm_a, int gpio_pwm_b, uint32_t freq_hz);

    /**
     * @brief Destrutor padrao.
     */
    ~McpwmMotorBlock() override;

    /**
     * @brief Inicializa o pipeline do MCPWM (Timer, Operador, Comparador e Geradores).
     *
     * @return true Se todos os modulos de hardware foram alocados com sucesso.
     * @return false Se ocorreu falha de memoria ou GPIO invalido.
     */
    bool initialize() override;

    /**
     * @brief Recupera a identificacao do bloco.
     *
     * @return std::string contendo o ID configurado no construtor.
     */
    std::string getId() const override;

    /**
     * @brief Atualiza a velocidade e direcao do motor DC.
     *
     * @param speed_percent Valor percentual da velocidade (-100.0 a 100.0).
     * Valores positivos giram em um sentido, negativos no oposto.
     * O valor 0.0 desativa ambas as saidas (freio/roda livre).
     * @return true Se o comando foi aceito pelo hardware.
     * @return false Se os ponteiros de hardware forem nulos.
     */
    bool setSpeed(float speed_percent);

private:
    std::string m_id;
    int m_gpio_a;
    int m_gpio_b;
    uint32_t m_freq_hz;
    uint32_t m_period_ticks;

    mcpwm_timer_handle_t m_timer;
    mcpwm_oper_handle_t m_oper;
    mcpwm_cmpr_handle_t m_cmpr;
    mcpwm_gen_handle_t m_gen_a;
    mcpwm_gen_handle_t m_gen_b;
};

} // namespace Cefet