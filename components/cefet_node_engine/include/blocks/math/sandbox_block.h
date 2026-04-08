/**
 * @file sandbox_block.h
 * @brief Bloco de Sandbox executando a Maquina Virtual Lua 5.4 de forma hibrida e com suporte a Base64.
 */
#pragma once

#include <string>
#include <vector>
#include "i_function_block.h"
#include "cJSON.h"

extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

namespace Cefet {

/**
 * @brief Sandbox Service Interface Function Block (SIFB).
 *
 * Embute um interpretador Lua para execucao de logicas de controlo customizaveis.
 * Possui inicializacao hibrida: executa codigo decodificado em Base64 diretamente da RAM
 * (enviado pela IHM) ou faz o fallback seguro para a leitura de um ficheiro fisico (.lua) no SPIFFS.
 */
class SandboxBlock : public IFunctionBlock {
public:
    /**
     * @brief Construtor do Sandbox hibrido com alocacao vetorial.
     * @param block_id ID unico do bloco na rede.
     * @param script_b64 String contendo o codigo fonte Lua em Base64 (pode ser vazio).
     * @param script_path Caminho de fallback para o ficheiro no SPIFFS.
     * @param num_in Quantidade de portas de entrada dinamicas.
     * @param num_out Quantidade de portas de saida dinamicas.
     */
    SandboxBlock(const std::string& block_id, const std::string& script_b64, const std::string& script_path, size_t num_in, size_t num_out);

    /**
     * @brief Destrutor. Encerra a Maquina Virtual Lua e liberta a memoria alocada.
     */
    ~SandboxBlock() override;

    /**
     * @brief Inicializa a VM Lua e carrega o script (via RAM decodificada ou SPIFFS).
     * @return true se a compilacao e alocacao foram bem-sucedidas.
     */
    bool initialize() override;

    /**
     * @brief Recupera a identificacao do bloco.
     * @return std::string O ID configurado.
     */
    std::string getId() const override;

    /* ========================================================================= */
    /* PORTAS IEC 61499                                                          */
    /* ========================================================================= */

    /**
     * @brief Expoe o endereco de memoria das variaveis de saida calculadas.
     */
    void* getDataOutput(const std::string& port_name) override;

    /**
     * @brief Conecta os ponteiros de entrada de dados originados de outros blocos.
     */
    bool connectDataInput(const std::string& port_name, void* data_pointer) override;

    /**
     * @brief Processa os eventos de execucao (Ex: REQ) chamando a VM Lua.
     */
    void triggerEventInput(const std::string& event_name) override;

    /**
     * @brief Factory method para instanciacao via JSON.
     */
    static IFunctionBlock* create(const std::string& block_id, cJSON* config);

private:
    std::string m_id;
    std::string m_script_b64;
    std::string m_script_path;

    size_t m_num_in;
    size_t m_num_out;

    /** @brief Ponteiro de contexto para a Maquina Virtual Lua isolada. */
    lua_State* L;

    /** @brief Arrays dinamicos de E/S tipados em float (Polimorfismo). */
    std::vector<float*> m_inputs;
    std::vector<float> m_outputs;
};

} /* namespace Cefet */