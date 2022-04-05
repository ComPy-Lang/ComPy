#ifndef LFORTRAN_ASR_TO_LLVM_H
#define LFORTRAN_ASR_TO_LLVM_H

#include <libasr/asr.h>
#include <libasr/codegen/evaluator.h>

namespace LFortran {

    Result<std::unique_ptr<LLVMModule>> asr_to_llvm(ASR::TranslationUnit_t &asr,
            diag::Diagnostics &diagnostics,
            llvm::LLVMContext &context, Allocator &al, Platform platform,
            bool fast, const std::string &rl_path, const std::string &run_fn);

} // namespace LFortran

#endif // LFORTRAN_ASR_TO_LLVM_H
