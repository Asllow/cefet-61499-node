#include "pwm_output_block.h"
#include "esp_log.h"

namespace Cefet {

static const char* TAG = "PWM_OUTPUT_BLOCK";

PwmOutputBlock::PwmOutputBlock(const std::string& block_id, int gpio_num, ledc_timer_t timer_num, ledc_channel_t channel_num, uint32_t freq_hz)
    : m_id(block_id), m_gpio_num(gpio_num), m_timer_num(timer_num), m_channel_num(channel_num), m_freq_hz(freq_hz)
{
}

PwmOutputBlock::~PwmOutputBlock()
{
    ledc_stop(LEDC_LOW_SPEED_MODE, m_channel_num, 0);
}

bool PwmOutputBlock::initialize()
{
    ledc_timer_config_t ledc_timer = {};
    ledc_timer.speed_mode       = LEDC_LOW_SPEED_MODE;
    ledc_timer.timer_num        = m_timer_num;
    ledc_timer.duty_resolution  = LEDC_TIMER_13_BIT;
    ledc_timer.freq_hz          = m_freq_hz;
    ledc_timer.clk_cfg          = LEDC_AUTO_CLK;

    if (ledc_timer_config(&ledc_timer) != ESP_OK) {
        ESP_LOGE(TAG, "[%s] Falha ao configurar temporizador PWM.", m_id.c_str());
        return false;
    }

    ledc_channel_config_t ledc_channel = {};
    ledc_channel.speed_mode     = LEDC_LOW_SPEED_MODE;
    ledc_channel.channel        = m_channel_num;
    ledc_channel.timer_sel      = m_timer_num;
    ledc_channel.gpio_num       = m_gpio_num;
    ledc_channel.duty           = 0;
    ledc_channel.hpoint         = 0;

    if (ledc_channel_config(&ledc_channel) != ESP_OK) {
        ESP_LOGE(TAG, "[%s] Falha ao configurar canal PWM.", m_id.c_str());
        return false;
    }

    ESP_LOGI(TAG, "[%s] Bloco PWM inicializado. Pino: %d, Freq: %lu Hz", m_id.c_str(), m_gpio_num, m_freq_hz);
    return true;
}

std::string PwmOutputBlock::getId() const
{
    return m_id;
}

bool PwmOutputBlock::writePwm(uint32_t duty_cycle)
{
    if (ledc_set_duty(LEDC_LOW_SPEED_MODE, m_channel_num, duty_cycle) != ESP_OK) {
        return false;
    }

    if (ledc_update_duty(LEDC_LOW_SPEED_MODE, m_channel_num) != ESP_OK) {
        return false;
    }

    return true;
}

} // namespace Cefet