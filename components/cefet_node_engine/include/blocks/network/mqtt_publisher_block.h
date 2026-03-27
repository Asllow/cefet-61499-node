#pragma once

#include <string>
#include "i_function_block.h"
#include "mqtt_client.h"

namespace Cefet {

/**
 * @brief Bloco Funcional de Publicacao MQTT (CSIFB).
 *
 * Encapsula o cliente MQTT da Espressif. Responsavel por enviar dados
 * (telemetria, logs ou estados) do no para o Broker da rede OT.
 */
class MqttPublisherBlock : public IFunctionBlock {
public:
    /**
     * @brief Construtor do publicador MQTT.
     *
     * @param block_id Identificador unico do bloco.
     * @param broker_uri Endereco do Broker MQTT (ex: "mqtt://192.168.1.100" ou "mqtt://test.mosquitto.org").
     * @param target_topic Topico MQTT onde os dados serao publicados.
     */
    MqttPublisherBlock(const std::string& block_id, const std::string& broker_uri, const std::string& target_topic);

    /**
     * @brief Destrutor padrao. Desconecta do Broker se necessario.
     */
    ~MqttPublisherBlock() override;

    /**
     * @brief Inicializa o cliente MQTT e inicia a conexao com o Broker.
     *
     * @return true Se o cliente foi instanciado.
     * @return false Se a URI for invalida ou faltar memoria.
     */
    bool initialize() override;

    /**
     * @brief Recupera a identificacao do bloco.
     *
     * @return std::string ID do bloco.
     */
    std::string getId() const override;

    /**
     * @brief Publica uma mensagem de texto (payload) no topico configurado.
     *
     * @param payload A string contendo os dados (ex: um JSON ou numero).
     * @return true Se a mensagem foi enfileirada para envio.
     * @return false Se o cliente nao estiver conectado.
     */
    bool publish(const std::string& payload);

private:
    std::string m_id;
    std::string m_broker_uri;
    std::string m_topic;
    esp_mqtt_client_handle_t m_client;
    bool m_is_connected;

    /**
     * @brief Callback estatica para lidar com eventos internos do protocolo MQTT.
     */
    static void mqttEventHandler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data);
};

} // namespace Cefet