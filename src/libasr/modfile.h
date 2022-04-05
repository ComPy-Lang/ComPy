#ifndef LFORTRAN_MODFILE_H
#define LFORTRAN_MODFILE_H

#include <libasr/asr.h>

namespace LFortran {

    // Save a module to a modfile
    std::string save_modfile(const ASR::TranslationUnit_t &m);

    // Load a module from a modfile
    ASR::TranslationUnit_t* load_modfile(Allocator &al, const std::string &s,
        bool load_symtab_id, SymbolTable &symtab);

}

#endif // LFORTRAN_MODFILE_H
