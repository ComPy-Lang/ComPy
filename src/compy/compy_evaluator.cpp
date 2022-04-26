#include <iostream>
#include <fstream>

#include <compy/compy_evaluator.h>
#include <libasr/codegen/asr_to_cpp.h>
#include <libasr/exception.h>
#include <libasr/asr.h>

#ifdef HAVE_LFORTRAN_LLVM
#include <libasr/codegen/evaluator.h>
#include <libasr/codegen/asr_to_llvm.h>
#else
namespace LFortran {
    class LLVMEvaluator {};
}
#endif

namespace LFortran {


/* ------------------------------------------------------------------------- */
// ComPyEvaluator

ComPyCompiler::ComPyCompiler(CompilerOptions compiler_options)
    :
    al{1024*1024},
#ifdef HAVE_LFORTRAN_LLVM
    e{std::make_unique<LLVMEvaluator>()},
    eval_count{0},
#endif
    compiler_options{compiler_options}
//    symbol_table{nullptr}
{
}

ComPyCompiler::~ComPyCompiler() = default;


Result<std::unique_ptr<LLVMModule>> ComPyCompiler::get_llvm3(
#ifdef HAVE_LFORTRAN_LLVM
    ASR::TranslationUnit_t &asr, diag::Diagnostics &diagnostics
#else
    ASR::TranslationUnit_t &/*asr*/, diag::Diagnostics &/*diagnostics*/
#endif
    )
{
#ifdef HAVE_LFORTRAN_LLVM
    eval_count++;
    run_fn = "__lfortran_evaluate_" + std::to_string(eval_count);

    // ASR -> LLVM
    std::unique_ptr<LFortran::LLVMModule> m;
    Result<std::unique_ptr<LFortran::LLVMModule>> res
        = asr_to_llvm(asr, diagnostics,
            e->get_context(), al, compiler_options.platform,
            compiler_options.fast, get_runtime_library_dir(),
            run_fn);
    if (res.ok) {
        m = std::move(res.result);
    } else {
        LFORTRAN_ASSERT(diagnostics.has_error())
        return res.error;
    }

    if (compiler_options.fast) {
        e->opt(*m->m_m);
    }

    return m;
#else
    throw LFortranException("LLVM is not enabled");
#endif
}

} // namespace LFortran
