#pragma once

#include <string>
#include "i_function_block.h"
#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"

namespace Cefet {

/**
 * @brief Bloco Funcional de Entrada Analogica (SIFB).
 *
 * Encapsula o driver adc_oneshot introduzido no ESP-IDF moderno,
 * substituindo as APIs legadas removidas. Atua como interface de 
 * leitura para sensores de campo.
 */
class AnalogInputBlock : public IFunctionBlock {
public:
    /**
     * @brief Construtor do bloco de entrada analogica.
     *
     * @param block_id Identificador unico do bloco na rede.
     * @param adc_unit Unidade ADC do hardware (ADC_UNIT_1 ou ADC_UNIT_2).
     * @param adc_channel Canal especifico do ADC associado ao pino GPIO.
     */
    AnalogInputBlock(const std::string& block_id, adc_unit_t adc_unit, adc_channel_t adc_channel);

    /**
     * @brief Destrutor padrao.
     * * Libera os recursos de hardware alocados.
     */
    ~AnalogInputBlock() override;

    /**
     * @brief Inicializa o periferico ADC e configura a atenuacao.
     *
     * @return true Se o driver foi configurado com sucesso.
     * @return false Se ocorreu falha na alocacao do periferico.
     */
    bool initialize() override;

    /**
     * @brief Recupera a identificacao do bloco.
     *
     * @return std::string contendo o ID configurado no construtor.
     */
    std::string getId() const override;

    /**
     * @brief Realiza a leitura instantanea do barramento ADC.
     *
     * @param out_value Ponteiro para armazenar o valor digitalizado.
     * @return true Se a leitura for bem sucedida.
     * @return false Se houver timeout ou falha de hardware.
     */
    bool readRaw(int* out_value);

private:
    std::string m_id;
    adc_unit_t m_unit;
    adc_channel_t m_channel;
    adc_oneshot_unit_handle_t m_adc_handle;
};

} // namespace Cefet