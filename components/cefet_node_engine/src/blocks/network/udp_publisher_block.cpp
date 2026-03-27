#include "udp_publisher_block.h"
#include "esp_log.h"
#include <cstring>

namespace Cefet {

static const char* TAG = "UDP_PUB_BLOCK";

UdpPublisherBlock::UdpPublisherBlock(const std::string& block_id, const std::string& target_ip, uint16_t target_port)
    : m_id(block_id), m_target_ip(target_ip), m_port(target_port), m_socket(-1)
{
    memset(&m_dest_addr, 0, sizeof(m_dest_addr));
    m_dest_addr.sin_family = AF_INET;
    m_dest_addr.sin_port = htons(m_port);
    m_dest_addr.sin_addr.s_addr = inet_addr(m_target_ip.c_str());
}

UdpPublisherBlock::~UdpPublisherBlock()
{
    if (m_socket != -1) {
        close(m_socket);
    }
}

bool UdpPublisherBlock::initialize()
{
    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (m_socket < 0) {
        ESP_LOGE(TAG, "[%s] Falha ao criar socket UDP.", m_id.c_str());
        return false;
    }

    ESP_LOGI(TAG, "[%s] Bloco UDP inicializado. Alvo: %s:%d", m_id.c_str(), m_target_ip.c_str(), m_port);
    return true;
}

std::string UdpPublisherBlock::getId() const
{
    return m_id;
}

bool UdpPublisherBlock::publish(const std::string& payload)
{
    if (m_socket < 0) return false;

    int err = sendto(m_socket, payload.c_str(), payload.length(), 0, 
                     (struct sockaddr *)&m_dest_addr, sizeof(m_dest_addr));

    if (err < 0) {
        ESP_LOGE(TAG, "[%s] Falha ao enviar datagrama UDP.", m_id.c_str());
        return false;
    }
    
    return true;
}

} // namespace Cefet