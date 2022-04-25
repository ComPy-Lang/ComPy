#ifndef LFORTRAN_COMPY_AST_TO_ASR_H
#define LFORTRAN_COMPY_AST_TO_ASR_H

#include <compy/ast.h>
#include <libasr/asr.h>

namespace LFortran::ComPy {

    std::string pickle_ast(AST::ast_t &ast, bool colors=false);

} // namespace LFortran

#endif // LFORTRAN_COMPY_AST_TO_ASR_H
