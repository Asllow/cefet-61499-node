#pragma once

#include <string>
#include "i_function_block.h"
#include "mqtt_client.h"
#include "cJSON.h"

namespace Cefet {

/**
 * @brief MQTT Publisher Service Interface Function Block (CSIFB).
 *
 * Encapsulates the ESP-IDF MQTT client to act as a publisher node.
 * Routes internal control loop events to an external IT/OT broker.
 */
class MqttPublisherBlock : public IFunctionBlock {
public:
    /**
     * @brief Instantiates the MQTT Publisher block.
     *
     * @param block_id Unique identifier for the block instance.
     * @param broker_uri Full URI of the MQTT broker (e.g., "mqtt://192.168.1.100").
     * @param target_topic The MQTT topic where the payload will be published.
     */
    MqttPublisherBlock(const std::string& block_id, const std::string& broker_uri, const std::string& target_topic);

    /**
     * @brief Destroys the MQTT block and releases network resources.
     */
    ~MqttPublisherBlock() override;

    /**
     * @brief Initializes the MQTT client state machine and connects to the broker.
     *
     * @return true if hardware allocation and state machine start successfully.
     */
    bool initialize() override;

    /**
     * @brief Retrieves the block's unique identifier.
     *
     * @return std::string The configured block ID.
     */
    std::string getId() const override;

    /**
     * @brief Enqueues a payload for publication on the configured topic.
     *
     * @param payload String containing the data (e.g., JSON string or numeric value).
     * @return true if the message was successfully enqueued.
     * @return false if the client is disconnected or out of memory.
     */
    bool publish(const std::string& payload);

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
    std::string m_broker_uri;
    std::string m_topic;
    esp_mqtt_client_handle_t m_client;
    bool m_is_connected;

    /**
     * @brief Internal static callback to handle ESP-IDF MQTT events.
     *
     * @param handler_args Opaque pointer to the class instance (this).
     * @param base Event family base.
     * @param event_id Specific event identifier.
     * @param event_data Pointer to the raw MQTT event data.
     */
    static void mqttEventHandler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data);
};

} // namespace Cefet