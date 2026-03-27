#pragma once

#include <string>
#include "i_function_block.h"
#include "lwip/sockets.h"

namespace Cefet {

/**
 * @brief Bloco Funcional de Publicacao UDP P2P (CSIFB).
 *
 * Envia datagramas UDP ultrarrapidos diretamente para outro IP
 * ou para um endereco de Broadcast/Multicast na rede local.
 * Ideal para sincronizacao entre CLPs (ESP32) na mesma celula.
 */
class UdpPublisherBlock : public IFunctionBlock {
public:
    /**
     * @brief Construtor do publicador UDP.
     *
     * @param block_id Identificador unico do bloco.
     * @param target_ip Endereco IP de destino (ex: "192.168.1.50" ou "255.255.255.255" para broadcast).
     * @param target_port Porta UDP de destino (ex: 5000).
     */
    UdpPublisherBlock(const std::string& block_id, const std::string& target_ip, uint16_t target_port);

    ~UdpPublisherBlock() override;

    /**
     * @brief Cria o socket UDP no sistema operacional.
     */
    bool initialize() override;

    std::string getId() const override;

    /**
     * @brief Dispara o pacote UDP imediatamente para a rede.
     *
     * @param payload Texto ou serializacao a ser transmitida.
     * @return true se os bytes foram entregues a camada de rede.
     */
    bool publish(const std::string& payload);

private:
    std::string m_id;
    std::string m_target_ip;
    uint16_t m_port;
    int m_socket;
    struct sockaddr_in m_dest_addr;
};

} // namespace Cefet