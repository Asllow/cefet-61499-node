#include "e_cycle_block.h"
#include "cefet_node_engine.h"
#include "esp_log.h"

namespace Cefet {

static const char* TAG = "E_CYCLE_BLOCK";

ECycleBlock::ECycleBlock(const std::string& block_id, uint64_t period_ms, EventIds target_event)
    : m_id(block_id), m_period_ms(period_ms), m_target_event(target_event), m_timer_handle(nullptr)
{
}

ECycleBlock::~ECycleBlock()
{
    if (m_timer_handle != nullptr) {
        stopTimer();
        esp_timer_delete(m_timer_handle);
    }
}

bool ECycleBlock::initialize()
{
    esp_timer_create_args_t timer_args = {};
    timer_args.callback = &ECycleBlock::timerCallback;
    timer_args.arg = this; // Passa a instancia atual para a callback estatica
    timer_args.name = m_id.c_str();

    esp_err_t err = esp_timer_create(&timer_args, &m_timer_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "[%s] Falha ao criar temporizador de alta resolucao.", m_id.c_str());
        return false;
    }

    ESP_LOGI(TAG, "[%s] Temporizador inicializado (Periodo: %llu ms).", m_id.c_str(), m_period_ms);
    return true;
}

std::string ECycleBlock::getId() const
{
    return m_id;
}

void ECycleBlock::startTimer()
{
    if (m_timer_handle != nullptr) {
        // A API esp_timer trabalha em microssegundos
        esp_timer_start_periodic(m_timer_handle, m_period_ms * 1000);
        ESP_LOGI(TAG, "[%s] Temporizador iniciado.", m_id.c_str());
    }
}

void ECycleBlock::stopTimer()
{
    if (m_timer_handle != nullptr) {
        esp_timer_stop(m_timer_handle);
        ESP_LOGI(TAG, "[%s] Temporizador parado.", m_id.c_str());
    }
}

void ECycleBlock::timerCallback(void* arg)
{
    // Recupera a instancia da classe que disparou a callback
    ECycleBlock* instance = static_cast<ECycleBlock*>(arg);

    // Publica o evento configurado no barramento central
    CefetEngine::postEvent(instance->m_target_event);
}

} // namespace Cefet