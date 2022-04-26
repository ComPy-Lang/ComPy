#ifndef LFORTRAN_COMPY_EVALUATOR_H
#define LFORTRAN_COMPY_EVALUATOR_H

#include <iostream>
#include <memory>

#include <libasr/alloc.h>
#include <libasr/asr_scopes.h>
#include <libasr/asr.h>
#include <compy/utils.h>
#include <libasr/config.h>
#include <libasr/diagnostics.h>

namespace LFortran {

class LLVMModule;
class LLVMEvaluator;

/*
   ComPyCompiler is the main class to access the ComPy compiler.

   This class is used for both interactive (.evaluate()) and non-interactive
   (.get_llvm2()) compilation. The methods return diagnostic messages (errors,
   warnings, style suggestions, ...) as an argument. One can use
   Diagnostic::render to render them.

   One can use get_asr2() to obtain the ASR and then hand it over to other
   backends by hand.
*/
class ComPyCompiler
{
public:
    ComPyCompiler(CompilerOptions compiler_options);
    ~ComPyCompiler();

    struct EvalResult {
        enum {
            integer4, integer8, real4, real8, complex4, complex8, statement, none
        } type;
        union {
            int32_t i32;
            int64_t i64;
            float f32;
            double f64;
            struct {float re, im;} c32;
            struct {double re, im;} c64;
        };
        std::string ast;
        std::string asr;
        std::string llvm_ir;
    };

    Result<std::unique_ptr<LLVMModule>> get_llvm3(ASR::TranslationUnit_t &asr,
        diag::Diagnostics &diagnostics);

private:
    Allocator al;
#ifdef HAVE_LFORTRAN_LLVM
    std::unique_ptr<LLVMEvaluator> e;
    int eval_count;
#endif
    CompilerOptions compiler_options;
//    SymbolTable *symbol_table;
    std::string run_fn;
};

} // namespace LFortran

#endif // LFORTRAN_COMPY_EVALUATOR_H
