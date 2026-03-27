#pragma once

#include <string>

namespace Cefet {

/**
 * @brief Interface base para todos os Function Blocks do padrão CEFET-61499.
 * * Esta interface garante o polimorfismo do sistema. Qualquer bloco de controle, 
 * I/O de hardware ou interface de rede deve herdar desta classe. Isso permite 
 * que o Gerenciador de Instâncias (JSON Parser) trate todos os blocos de forma 
 * genérica, alocando-os dinamicamente na memória RAM do ESP32.
 */
class IFunctionBlock {
public:
    /**
     * @brief Destrutor virtual padrão.
     * Garante a liberação correta de memória das classes derivadas.
     */
    virtual ~IFunctionBlock() = default;

    /**
     * @brief Rotina de inicialização do Bloco Funcional.
     * Deve ser chamada logo após a instânciação para configurar periféricos
     * (ex: pinos de GPIO) ou alocar recursos de rede.
     * * @return true Se inicializado com sucesso.
     * @return false Se ocorreu falha (ex: hardware não responde).
     */
    virtual bool initialize() = 0;

    /**
     * @brief Recupera a identificação única do bloco instanciado.
     * Este ID é utilizado para criar as conexões (Wiring) entre os blocos.
     * * @return std::string contendo o ID (ex: "PID_NIVEL_TANQUE_1").
     */
    virtual std::string getId() const = 0;
};

} // namespace Cefet