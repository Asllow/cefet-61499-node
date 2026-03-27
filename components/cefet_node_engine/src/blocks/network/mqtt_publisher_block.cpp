#include "mqtt_publisher_block.h"
#include "esp_log.h"

namespace Cefet {

static const char* TAG = "MQTT_PUBLISHER_BLOCK";

MqttPublisherBlock::MqttPublisherBlock(const std::string& block_id, const std::string& broker_uri, const std::string& target_topic)
    : m_id(block_id), m_broker_uri(broker_uri), m_topic(target_topic), m_client(nullptr), m_is_connected(false)
{
}

MqttPublisherBlock::~MqttPublisherBlock()
{
    if (m_client != nullptr) {
        esp_mqtt_client_stop(m_client);
        esp_mqtt_client_destroy(m_client);
    }
}

bool MqttPublisherBlock::initialize()
{
    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.broker.address.uri = m_broker_uri.c_str();

    m_client = esp_mqtt_client_init(&mqtt_cfg);
    if (m_client == nullptr) {
        ESP_LOGE(TAG, "[%s] Falha ao alocar memoria para o cliente MQTT.", m_id.c_str());
        return false;
    }

    esp_mqtt_client_register_event(m_client, MQTT_EVENT_ANY, &MqttPublisherBlock::mqttEventHandler, this);

    esp_err_t err = esp_mqtt_client_start(m_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "[%s] Falha ao iniciar maquina de estado MQTT.", m_id.c_str());
        return false;
    }

    ESP_LOGI(TAG, "[%s] Bloco MQTT inicializado. Alvo: %s", m_id.c_str(), m_broker_uri.c_str());
    return true;
}

std::string MqttPublisherBlock::getId() const
{
    return m_id;
}

bool MqttPublisherBlock::publish(const std::string& payload)
{
    if (m_client == nullptr || !m_is_connected) {
        ESP_LOGW(TAG, "[%s] Tentativa de publicacao abortada: Cliente desconectado.", m_id.c_str());
        return false;
    }

    int msg_id = esp_mqtt_client_publish(m_client, m_topic.c_str(), payload.c_str(), 0, 1, 0);
    
    if (msg_id == -1) {
        ESP_LOGE(TAG, "[%s] Erro ao enfileirar mensagem MQTT.", m_id.c_str());
        return false;
    }

    return true;
}

void MqttPublisherBlock::mqttEventHandler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data)
{
    auto* instance = static_cast<MqttPublisherBlock*>(handler_args);
    auto* event = static_cast<esp_mqtt_event_handle_t>(event_data);

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            instance->m_is_connected = true;
            ESP_LOGI(TAG, "[%s] Conectado ao Broker MQTT.", instance->m_id.c_str());
            break;
        case MQTT_EVENT_DISCONNECTED:
            instance->m_is_connected = false;
            ESP_LOGW(TAG, "[%s] Desconectado do Broker MQTT.", instance->m_id.c_str());
            break;
        case MQTT_EVENT_PUBLISHED:
            // Log intencionalmente omitido para nao poluir o terminal em alta frequencia
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "[%s] Erro interno no protocolo MQTT.", instance->m_id.c_str());
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                ESP_LOGE(TAG, "[%s] Falha na camada de transporte (TCP/IP).", instance->m_id.c_str());
            }
            break;
        default:
            break;
    }
}

} // namespace Cefet