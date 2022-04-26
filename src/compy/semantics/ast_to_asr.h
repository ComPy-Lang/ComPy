#ifndef LFORTRAN_COMPY_AST_TO_ASR_H
#define LFORTRAN_COMPY_AST_TO_ASR_H

#include <compy/ast.h>
#include <libasr/asr.h>

namespace LFortran::ComPy {

    std::string pickle_ast(AST::ast_t &ast, bool colors=false, bool indent=false);
    Result<ASR::TranslationUnit_t*> ast_to_asr(Allocator &al,
    ComPy::AST::ast_t &ast, diag::Diagnostics &diagnostics, bool main_module,
    bool symtab_only);

} // namespace LFortran

#endif // LFORTRAN_COMPY_AST_TO_ASR_H
