#ifndef LFORTRAN_ASR_TO_CPP_H
#define LFORTRAN_ASR_TO_CPP_H

#include <libasr/asr.h>

namespace LFortran {

    Result<std::string> asr_to_cpp(Allocator &al, ASR::TranslationUnit_t &asr,
        diag::Diagnostics &diagnostics);

} // namespace LFortran

#endif // LFORTRAN_ASR_TO_CPP_H
