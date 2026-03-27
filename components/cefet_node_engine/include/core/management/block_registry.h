#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include "i_function_block.h"
#include "cJSON.h"

namespace Cefet {

/**
 * @brief Assinatura da funcao fabrica (Factory Function).
 * Todo bloco deve prover uma funcao com esta assinatura para ser criado dinamicamente.
 */
using BlockFactoryFunc = std::function<IFunctionBlock*(const std::string& block_id, cJSON* config)>;

/**
 * @brief Registro Central de Blocos (Factory Method).
 * Mantem um dicionario mapeando nomes de blocos (strings) para suas funcoes construtoras.
 */
class BlockRegistry {
public:
    /**
     * @brief Registra um novo tipo de bloco no dicionario.
     *
     * @param block_type O nome do tipo (ex: "AnalogInput").
     * @param factory A funcao que sabe instanciar este bloco.
     */
    static void registerBlock(const std::string& block_type, BlockFactoryFunc factory);

    /**
     * @brief Instancia um bloco dinamicamente baseado no seu tipo.
     *
     * @param block_type O tipo do bloco solicitado pelo JSON.
     * @param block_id O ID unico (nome da instancia) deste bloco na rede.
     * @param config O ponteiro para o pedaco do JSON que contem as configuracoes especificas dele.
     * @return IFunctionBlock* Ponteiro para o bloco recem-criado (ou nullptr se nao existir).
     */
    static IFunctionBlock* createBlock(const std::string& block_type, const std::string& block_id, cJSON* config);

private:
    /**
     * @brief Singleton para proteger a ordem de inicializacao do dicionario em C++.
     */
    static std::unordered_map<std::string, BlockFactoryFunc>& getRegistry();
};

} // namespace Cefet