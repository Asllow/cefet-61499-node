#pragma once

#include <string>
#include "i_function_block.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"

namespace Cefet {

/**
 * @brief Modbus TCP Server Service Interface Function Block (CSIFB).
 *
 * Implementa um servidor Modbus TCP frugal diretamente sobre sockets BSD (lwIP).
 * Ao ignorar o peso da biblioteca FreeModbus oficial, este bloco garante 
 * baixo consumo de memória RAM e alta velocidade de resposta (determinismo),
 * características essenciais para manter a estabilidade da planta Fanplate.
 * * Mantém um banco interno de Holding Registers que podem ser acedidos 
 * por um Mestre (CLP ou Servidor Orquestrador OT).
 */
class ModbusTcpServerBlock : public IFunctionBlock {
public:
    /**
     * @brief Instancia o bloco do Servidor Modbus TCP Nativo.
     *
     * @param block_id Identificador único da instância do bloco.
     * @param port Porta TCP para escuta (Padrão industrial: 502).
     */
    ModbusTcpServerBlock(const std::string& block_id, uint16_t port);

    /**
     * @brief Destrutor virtual. Encerra a Task de escuta e liberta o socket.
     */
    ~ModbusTcpServerBlock() override;

    /**
     * @brief Aloca o socket TCP, faz o bind à porta e lança a Task do FreeRTOS.
     *
     * @return true se o socket e a thread forem criados com sucesso.
     */
    bool initialize() override;

    /**
     * @brief Recupera o ID do bloco.
     * @return std::string contendo o ID configurado.
     */
    std::string getId() const override;

    // =========================================================================
    // PORTAS IEC 61499
    // =========================================================================

    /**
     * @brief Fornece os ponteiros de saída (dados escritos pelo Mestre Modbus).
     * Suporta "REG_OUT_0" e "REG_OUT_1".
     */
    void* getDataOutput(const std::string& port_name) override;

    /**
     * @brief Conecta os ponteiros de entrada (dados gerados pelo ESP para o Mestre ler).
     * Suporta "REG_IN_0" e "REG_IN_1".
     */
    bool connectDataInput(const std::string& port_name, void* data_pointer) override;

    /**
     * @brief Sincroniza os ponteiros internos com os registos Modbus.
     * Deve ser engatilhado pelo evento "UPDATE".
     */
    void triggerEventInput(const std::string& event_name) override;

    /**
     * @brief Factory method para instanciação dinâmica via JSON manifest.
     *
     * @param block_id Identificador único para a nova instância.
     * @param config Ponteiro cJSON contendo os parâmetros específicos do bloco.
     * @return IFunctionBlock* Ponteiro para a instância alocada na memória.
     */
    static IFunctionBlock* create(const std::string& block_id, cJSON* config);

private:
    std::string m_id;
    uint16_t m_port;
    int m_listen_sock;
    TaskHandle_t m_server_task;
    
    /** @brief Tabela de Holding Registers. [0,1] = Entradas do ESP. [2,3] = Saídas do ESP. */
    uint16_t m_holding_registers[4];

    int* m_reg_in_0;
    int* m_reg_in_1;
    int m_reg_out_0;
    int m_reg_out_1;

    /**
     * @brief Função da Task do FreeRTOS que bloqueia à espera de conexões TCP.
     * Realiza o parseamento minimalista do cabeçalho MBAP e da PDU Modbus.
     *
     * @param arg Ponteiro opaco para a instância da classe (this).
     */
    static void serverTask(void* arg);
};

} // namespace Cefet