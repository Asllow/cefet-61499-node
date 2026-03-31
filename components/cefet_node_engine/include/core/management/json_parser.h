#pragma once

#include <string>
#include <vector>
#include "i_function_block.h"

namespace Cefet {

/**
 * @brief JSON Manifest Parser.
 *
 * Responsavel por desserializar o manifesto JSON recebido da rede,
 * iterar sobre a lista de blocos funcionais requeridos e coordenar
 * a sua instanciacao atraves do BlockRegistry.
 */
class JsonParser {
public:
    /**
     * @brief Analisa o manifesto e instancia os blocos correspondentes.
     *
     * @param json_payload String contendo o manifesto JSON no formato IEC 61499 adaptado.
     * @return std::vector<IFunctionBlock*> Lista de ponteiros para os blocos instanciados e inicializados.
     */
    static std::vector<IFunctionBlock*> parseManifest(const std::string& json_payload);
};

} // namespace Cefet