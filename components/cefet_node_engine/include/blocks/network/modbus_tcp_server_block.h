/**
 * @file modbus_tcp_server_block.h
 * @brief Servidor Modbus TCP Dinâmico e Genérico.
 */
#pragma once

#include <string>
#include <vector>
#include "i_function_block.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"

namespace Cefet {

/**
 * @brief Modbus TCP Server Service Interface Function Block (CSIFB).
 *
 * Implementação dinâmica sobre sockets BSD. Suporta quantidade configurável
 * de Holding Registers. Realiza conversão automática de float (IEEE 754) 
 * para inteiros de 16-bits com Complemento de 2, permitindo tráfego de números negativos.
 */
class ModbusTcpServerBlock : public IFunctionBlock {
public:
    ModbusTcpServerBlock(const std::string& block_id, uint16_t port, uint16_t num_in, uint16_t num_out);
    ~ModbusTcpServerBlock() override;

    bool initialize() override;
    std::string getId() const override;

    void* getDataOutput(const std::string& port_name) override;
    bool connectDataInput(const std::string& port_name, void* data_pointer) override;
    void triggerEventInput(const std::string& event_name) override;

    static IFunctionBlock* create(const std::string& block_id, cJSON* config);

private:
    std::string m_id;
    uint16_t m_port;
    uint16_t m_num_in;
    uint16_t m_num_out;
    
    int m_listen_sock;
    TaskHandle_t m_server_task;
    
    // Arrays dinâmicos alocados no construtor
    std::vector<uint16_t> m_holding_registers;
    std::vector<float*> m_regs_in;  // Ponteiros para dados externos (Sensores/Math)
    std::vector<float> m_regs_out;  // Dados recebidos da rede e expostos para atuadores

    static void serverTask(void* arg);
};

} // namespace Cefet