#include "analog_input_block.h"
#include "block_registry.h"
#include "esp_log.h"

namespace Cefet {

static const char* TAG = "ANALOG_INPUT_BLOCK";

AnalogInputBlock::AnalogInputBlock(const std::string& block_id, adc_unit_t adc_unit, adc_channel_t adc_channel)
    : m_id(block_id), m_unit(adc_unit), m_channel(adc_channel), m_adc_handle(nullptr), m_initialized(false)
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
    if (m_initialized) {
        return true;
    }

    adc_oneshot_unit_init_cfg_t init_config = {};
    init_config.unit_id = m_unit;
    init_config.clk_src = ADC_RTC_CLK_SRC_DEFAULT;

    esp_err_t err = adc_oneshot_new_unit(&init_config, &m_adc_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "[%s] Failed to allocate ADC unit.", m_id.c_str());
        return false;
    }

    adc_oneshot_chan_cfg_t config = {};
    config.bitwidth = ADC_BITWIDTH_DEFAULT;
    config.atten = ADC_ATTEN_DB_12; 

    err = adc_oneshot_config_channel(m_adc_handle, m_channel, &config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "[%s] Failed to configure ADC channel.", m_id.c_str());
        return false;
    }

    m_initialized = true;
    ESP_LOGI(TAG, "[%s] ADC Block initialized successfully.", m_id.c_str());
    return true;
}

std::string AnalogInputBlock::getId() const
{
    return m_id;
}

bool AnalogInputBlock::readRaw(int* out_value)
{
    if (!m_initialized || m_adc_handle == nullptr || out_value == nullptr) {
        return false;
    }

    esp_err_t err = adc_oneshot_read(m_adc_handle, m_channel, out_value);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "[%s] Failed to read ADC channel.", m_id.c_str());
        return false;
    }

    return true;
}

IFunctionBlock* AnalogInputBlock::create(const std::string& block_id, cJSON* config)
{
    int unit = 1;    // Fallback default: ADC_UNIT_1
    int channel = 4; // Fallback default: ADC_CHANNEL_4

    if (config != nullptr) {
        cJSON* unit_item = cJSON_GetObjectItem(config, "unit");
        if (cJSON_IsNumber(unit_item)) {
            unit = unit_item->valueint;
        }

        cJSON* channel_item = cJSON_GetObjectItem(config, "channel");
        if (cJSON_IsNumber(channel_item)) {
            channel = channel_item->valueint;
        }
    }

    return new AnalogInputBlock(block_id, static_cast<adc_unit_t>(unit), static_cast<adc_channel_t>(channel));
}

/**
 * @brief Static block registration.
 * Executes prior to app_main to register the factory method into the BlockRegistry.
 */
static bool registered = []() {
    BlockRegistry::registerBlock("AnalogInput", AnalogInputBlock::create);
    return true;
}();

} // namespace Cefet