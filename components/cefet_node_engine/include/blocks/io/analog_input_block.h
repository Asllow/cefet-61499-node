#pragma once

#include <string>
#include "i_function_block.h"
#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"
#include "cJSON.h"

namespace Cefet {

/**
 * @brief Analog Input Service Interface Function Block (SIFB).
 *
 * Encapsulates the ESP-IDF v6.0 oneshot ADC driver. 
 * Provides continuous or single-shot analog readings for control loops.
 */
class AnalogInputBlock : public IFunctionBlock {
public:
    /**
     * @brief Instantiates the Analog Input Block.
     *
     * @param block_id Unique network identifier for the block instance.
     * @param adc_unit Hardware ADC unit (e.g., ADC_UNIT_1).
     * @param adc_channel Hardware ADC channel corresponding to the physical GPIO.
     */
    AnalogInputBlock(const std::string& block_id, adc_unit_t adc_unit, adc_channel_t adc_channel);

    /**
     * @brief Destroys the Analog Input Block and frees hardware resources.
     */
    ~AnalogInputBlock() override;

    /**
     * @brief Initializes the ADC hardware peripheral.
     *
     * @return true if hardware allocation and calibration succeed, false otherwise.
     */
    bool initialize() override;

    /**
     * @brief Retrieves the block's unique identifier.
     *
     * @return std::string The configured block ID.
     */
    std::string getId() const override;

    /**
     * @brief Performs a raw analog-to-digital conversion.
     *
     * @param out_value Pointer to store the conversion result.
     * @return true if conversion is successful, false if hardware error occurs.
     */
    bool readRaw(int* out_value);

    /**
     * @brief Factory method for dynamic instantiation via JSON manifest.
     *
     * @param block_id Unique identifier for the new instance.
     * @param config cJSON pointer containing block-specific parameters.
     * @return IFunctionBlock* Pointer to the newly allocated instance.
     */
    static IFunctionBlock* create(const std::string& block_id, cJSON* config);

private:
    std::string m_id;
    adc_unit_t m_unit;
    adc_channel_t m_channel;
    adc_oneshot_unit_handle_t m_adc_handle;
    bool m_initialized;
};

} // namespace Cefet