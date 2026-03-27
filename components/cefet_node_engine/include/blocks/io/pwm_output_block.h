#pragma once

#include <string>
#include "i_function_block.h"
#include "driver/ledc.h"
#include "cJSON.h"

namespace Cefet {

/**
 * @brief PWM Output Service Interface Function Block (SIFB).
 *
 * Encapsulates the ESP-IDF LEDC driver for hardware-based PWM signal generation.
 * Utilized for driving power actuators, such as heaters, DC motors (via H-Bridge),
 * or frequency inverters.
 */
class PwmOutputBlock : public IFunctionBlock {
public:
    /**
     * @brief Instantiates the PWM Output Block.
     *
     * @param block_id Unique network identifier for the block instance.
     * @param gpio_num Physical ESP32 GPIO pin for the PWM signal.
     * @param timer_num Hardware timer allocated for this signal (e.g., LEDC_TIMER_0).
     * @param channel_num Allocated PWM control channel (e.g., LEDC_CHANNEL_0).
     * @param freq_hz Base frequency of the PWM signal in Hertz.
     */
    PwmOutputBlock(const std::string& block_id, int gpio_num, ledc_timer_t timer_num, ledc_channel_t channel_num, uint32_t freq_hz);

    /**
     * @brief Destroys the block and stops the PWM signal generation.
     */
    ~PwmOutputBlock() override;

    /**
     * @brief Initializes the LEDC peripheral, configuring timer and channel.
     *
     * @return true if the hardware driver was successfully configured.
     */
    bool initialize() override;

    /**
     * @brief Retrieves the block's unique identifier.
     *
     * @return std::string The configured block ID.
     */
    std::string getId() const override;

    /**
     * @brief Updates the duty cycle of the PWM signal.
     *
     * @param duty_cycle Duty cycle value (0 to 8191 for 13-bit resolution).
     * @return true if the hardware registers were updated successfully.
     */
    bool writePwm(uint32_t duty_cycle);

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
    int m_gpio_num;
    ledc_timer_t m_timer_num;
    ledc_channel_t m_channel_num;
    uint32_t m_freq_hz;
};

} // namespace Cefet