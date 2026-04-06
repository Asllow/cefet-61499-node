#pragma once

#include <string>
#include <vector>
#include "i_function_block.h"

namespace Cefet {

/**
 * @brief Gerenciador de Roteamento IEC 61499 (Connection Manager).
 * * Responsavel por ler o array "connections" do manifesto JSON e estabelecer
 * as pontes fisicas de memoria entre os blocos. Ele cria tanto as conexoes de 
 * Dados (Fios Azuis - compartilhamento de ponteiros) quanto as rotas de 
 * Eventos (Fios Vermelhos - chamadas de execucao em cascata).
 */
class ConnectionManager {
public:
    /**
     * @brief Executa o roteamento fisico na memoria do microcontrolador.
     * * @param json_payload A string contendo o manifesto JSON completo lido do SPIFFS.
     * @param blocks A lista contendo os blocos que ja foram instanciados pela Factory.
     * @return true Se o roteamento foi processado com sucesso.
     * @return false Se houve falha grave na sintaxe JSON.
     */
    static bool wireConnections(const std::string& json_payload, const std::vector<IFunctionBlock*>& blocks);
};

} // namespace Cefet