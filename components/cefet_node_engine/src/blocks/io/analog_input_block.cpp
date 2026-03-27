#include "analog_input_block.h"
#include "esp_log.h"

namespace Cefet {

static const char* TAG = "ANALOG_INPUT_BLOCK";

AnalogInputBlock::AnalogInputBlock(const std::string& block_id, adc_unit_t adc_unit, adc_channel_t adc_channel)
    : m_id(block_id), m_unit(adc_unit), m_channel(adc_channel), m_adc_handle(nullptr)
{
}

AnalogInputBlock::~AnalogInputBlock()
{
    if (m_adc_handle != nullptr) {
        adc_oneshot_del_unit(m_adc_handle);
    }
}

bool AnalogInputBlock::initialize()
{
    adc_oneshot_unit_init_cfg_t init_config = {};
    init_config.unit_id = m_unit;
    init_config.clk_src = ADC_RTC_CLK_SRC_DEFAULT;

    esp_err_t err = adc_oneshot_new_unit(&init_config, &m_adc_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "[%s] Falha ao inicializar unidade ADC.", m_id.c_str());
        return false;
    }

    adc_oneshot_chan_cfg_t config = {};
    config.bitwidth = ADC_BITWIDTH_DEFAULT;
    config.atten = ADC_ATTEN_DB_12; 

    err = adc_oneshot_config_channel(m_adc_handle, m_channel, &config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "[%s] Falha ao configurar canal ADC.", m_id.c_str());
        return false;
    }

    ESP_LOGI(TAG, "[%s] Bloco ADC inicializado com sucesso.", m_id.c_str());
    return true;
}

std::string AnalogInputBlock::getId() const
{
    return m_id;
}

bool AnalogInputBlock::readRaw(int* out_value)
{
    if (m_adc_handle == nullptr || out_value == nullptr) {
        return false;
    }

    esp_err_t err = adc_oneshot_read(m_adc_handle, m_channel, out_value);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "[%s] Falha na leitura do ADC.", m_id.c_str());
        return false;
    }

    return true;
}

} // namespace Cefet