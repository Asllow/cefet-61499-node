#pragma once

#include <string>
#include "i_function_block.h"
#include "cJSON.h"
#include "tinyexpr.h"

namespace Cefet {

/**
 * @brief Bloco Matemático Genérico (MathNodeBlock).
 * * Capaz de processar equações matemáticas em tempo real utilizando a TinyExpr.
 * Ideal para construir controladores P, PI, PD e PID de forma frugível,
 * alterando apenas a "expression" no ficheiro JSON.
 */
class MathNodeBlock : public IFunctionBlock {
public:
    MathNodeBlock(const std::string& block_id, const std::string& expression);
    ~MathNodeBlock() override;

    bool initialize() override;
    std::string getId() const override;

    void* getDataOutput(const std::string& port_name) override;
    bool connectDataInput(const std::string& port_name, void* data_pointer) override;
    void triggerEventInput(const std::string& event_name) override;

    static IFunctionBlock* create(const std::string& block_id, cJSON* config);

private:
    std::string m_id;
    std::string m_expression;
    te_expr* m_compiled_expr;

    // Entradas (Ponteiros para os blocos a montante)
    float* m_in_a;
    float* m_in_b;
    float* m_in_c;
    float* m_in_d;

    // Variáveis internas para a TinyExpr (exige o formato double)
    double m_val_a, m_val_b, m_val_c, m_val_d;

    // Saída calculada
    float m_out;
};

} // namespace Cefet