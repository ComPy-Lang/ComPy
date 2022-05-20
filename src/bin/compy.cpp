#include <iostream>

#define CLI11_HAS_FILESYSTEM 0
#include <bin/CLI11.hpp>

#include <libasr/stacktrace.h>
#include <compy/utils.h>
#include <compy/pickle.h>
#include <compy/semantics/ast_to_asr.h>
#include <compy/parser/tokenizer.h>
#include <compy/parser/parser.h>
#include <compy/compy_evaluator.h>
#include <libasr/config.h>
#include <libasr/string_utils.h>
#include <libasr/codegen/evaluator.h>

namespace {

using LFortran::endswith;
using LFortran::CompilerOptions;
using LFortran::parse_file;

enum Backend {
    llvm
};

std::string remove_extension(const std::string& filename) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos) return filename;
    return filename.substr(0, lastdot);
}

std::string remove_path(const std::string& filename) {
    size_t lastslash = filename.find_last_of("/");
    if (lastslash == std::string::npos) return filename;
    return filename.substr(lastslash+1);
}

int emit_tokens(const std::string &infile, bool line_numbers,
                const CompilerOptions &compiler_options)
{
    std::string input = LFortran::read_file(infile);
    // Src -> Tokens
    Allocator al(64*1024*1024);
    std::vector<int> toks;
    std::vector<LFortran::YYSTYPE> stypes;
    std::vector<LFortran::Location> locations;
    LFortran::diag::Diagnostics diagnostics;
    auto res = LFortran::tokens(al, input, diagnostics, &stypes, &locations);
    LFortran::LocationManager lm;
    lm.in_filename = infile;
    lm.init_simple(input);
    std::cerr << diagnostics.render(input, lm, compiler_options);
    if (res.ok) {
        toks = res.result;
    } else {
        LFORTRAN_ASSERT(diagnostics.has_error())
        return 1;
    }

    for (size_t i=0; i < toks.size(); i++) {
        std::cout << LFortran::pickle_token(toks[i], stypes[i]);
        if (line_numbers) {
            std::cout << " " << locations[i].first << ":" << locations[i].last;
        }
        std::cout << std::endl;
    }
    return 0;
}

int emit_ast(const std::string &infile,
    CompilerOptions &compiler_options)
{
    Allocator al(4*1024);
    LFortran::diag::Diagnostics diagnostics;
    LFortran::Result<LFortran::ComPy::AST::ast_t*> r = parse_file(
        al, infile, diagnostics);
    if (diagnostics.diagnostics.size() > 0) {
        LFortran::LocationManager lm;
        lm.in_filename = infile;
        std::string input = LFortran::read_file(infile);
        lm.init_simple(input);
        std::cerr << diagnostics.render(input, lm, compiler_options);
    }
    if (!r.ok) {
        LFORTRAN_ASSERT(diagnostics.has_error())
        return 1;
    }
    LFortran::ComPy::AST::ast_t* ast = r.result;

    std::cout << LFortran::ComPy::pickle_ast(*ast,
        compiler_options.use_colors, compiler_options.indent) << std::endl;
    return 0;
}

int emit_asr(const std::string &infile, CompilerOptions &compiler_options)
{
    Allocator al(4*1024);
    LFortran::diag::Diagnostics diagnostics;
    LFortran::LocationManager lm;
    lm.in_filename = infile;
    std::string input = LFortran::read_file(infile);
    lm.init_simple(input);
    LFortran::Result<LFortran::ComPy::AST::ast_t*> r1 = parse_file(
        al, infile, diagnostics);
    std::cerr << diagnostics.render(input, lm, compiler_options);
    if (!r1.ok) {
        return 1;
    }
    LFortran::ComPy::AST::ast_t* ast = r1.result;

    diagnostics.diagnostics.clear();
    LFortran::Result<LFortran::ASR::TranslationUnit_t*>
        r = LFortran::ComPy::ast_to_asr(al, *ast, diagnostics, true,
            compiler_options.symtab_only);
    std::cerr << diagnostics.render(input, lm, compiler_options);
    if (!r.ok) {
        LFORTRAN_ASSERT(diagnostics.has_error())
        return 2;
    }
    LFortran::ASR::TranslationUnit_t* asr = r.result;

    std::cout << LFortran::pickle(*asr, compiler_options.use_colors,
            compiler_options.indent) << std::endl;
    return 0;
}

#ifdef HAVE_LFORTRAN_LLVM

int emit_llvm(const std::string &infile,
    CompilerOptions &compiler_options)
{
    Allocator al(4*1024);
    LFortran::diag::Diagnostics diagnostics;
    LFortran::LocationManager lm;
    lm.in_filename = infile;
    std::string input = LFortran::read_file(infile);
    lm.init_simple(input);
    LFortran::Result<LFortran::ComPy::AST::ast_t*> r = parse_file(
        al, infile, diagnostics);
    std::cerr << diagnostics.render(input, lm, compiler_options);
    if (!r.ok) {
        return 1;
    }
    LFortran::ComPy::AST::ast_t* ast = r.result;
    diagnostics.diagnostics.clear();

    // Src -> AST -> ASR
    LFortran::Result<LFortran::ASR::TranslationUnit_t*>
        r1 = LFortran::ComPy::ast_to_asr(al, *ast, diagnostics, true,
            compiler_options.symtab_only);
    std::cerr << diagnostics.render(input, lm, compiler_options);
    if (!r1.ok) {
        LFORTRAN_ASSERT(diagnostics.has_error())
        return 2;
    }
    LFortran::ASR::TranslationUnit_t* asr = r1.result;
    diagnostics.diagnostics.clear();


    // ASR -> LLVM
    LFortran::ComPyCompiler fe(compiler_options);
    LFortran::Result<std::unique_ptr<LFortran::LLVMModule>>
        res = fe.get_llvm3(*asr, diagnostics);
    std::cerr << diagnostics.render(input, lm, compiler_options);
    if (!res.ok) {
        LFORTRAN_ASSERT(diagnostics.has_error())
        return 3;
    }
    std::cout << (res.result)->str();
    return 0;
}

int compile_to_object_file(
        const std::string &infile,
        const std::string &outfile,
        CompilerOptions &compiler_options)
{
    Allocator al(4*1024);
    LFortran::diag::Diagnostics diagnostics;
    LFortran::LocationManager lm;
    lm.in_filename = infile;
    std::string input = LFortran::read_file(infile);
    lm.init_simple(input);
    LFortran::Result<LFortran::ComPy::AST::ast_t*> r = parse_file(
        al, infile, diagnostics);
    std::cerr << diagnostics.render(input, lm, compiler_options);
    if (!r.ok) {
        return 1;
    }

    // Src -> AST -> ASR
    LFortran::ComPy::AST::ast_t* ast = r.result;
    diagnostics.diagnostics.clear();
    LFortran::Result<LFortran::ASR::TranslationUnit_t*>
        r1 = LFortran::ComPy::ast_to_asr(al, *ast, diagnostics, true,
            compiler_options.symtab_only);
    std::cerr << diagnostics.render(input, lm, compiler_options);
    if (!r1.ok) {
        LFORTRAN_ASSERT(diagnostics.has_error())
        return 2;
    }
    LFortran::ASR::TranslationUnit_t* asr = r1.result;
    diagnostics.diagnostics.clear();

    // ASR -> LLVM
    LFortran::ComPyCompiler fe(compiler_options);
    LFortran::LLVMEvaluator e(compiler_options.target);
    std::unique_ptr<LFortran::LLVMModule> m;
    LFortran::Result<std::unique_ptr<LFortran::LLVMModule>>
        res = fe.get_llvm3(*asr, diagnostics);
    std::cerr << diagnostics.render(input, lm, compiler_options);
    if (!res.ok) {
        LFORTRAN_ASSERT(diagnostics.has_error())
        return 3;
    }
    m = std::move(res.result);
    e.save_object_file(*(m->m_m), outfile);
    return 0;
}

#endif

int link_executable(const std::vector<std::string> &infiles,
    const std::string &outfile,
    const std::string &runtime_library_dir, Backend backend,
    CompilerOptions &compiler_options)
{

#ifdef HAVE_LFORTRAN_LLVM
    std::string t = (compiler_options.target == "") ? LFortran::LLVMEvaluator::get_default_target_triple() : compiler_options.target;
#endif

    if (backend == Backend::llvm) {
        std::string CC = "cc";
        char *env_CC = std::getenv("LFORTRAN_CC");
        if (env_CC) CC = env_CC;
        std::string base_path = "\"" + runtime_library_dir + "\"";
        std::string options;
        std::string runtime_lib = "compy_runtime";
        std::string cmd = CC + options + " -o " + outfile + " ";
        runtime_lib = "compy_runtime_static";
        for (auto &s : infiles) {
            cmd += s + " ";
        }
        cmd += + " -L"
            + base_path + " -Wl,-rpath," + base_path + " -l" + runtime_lib + " -lm";
        int err = system(cmd.c_str());
        if (err) {
            std::cout << "The command '" + cmd + "' failed." << std::endl;
            return 10;
        }
        return 0;
    } else {
        LFORTRAN_ASSERT(false);
        return 1;
    }
}


} // anonymous namespace

int main(int argc, char *argv[])
{
    LFortran::initialize();

    try {
        std::string runtime_library_dir = LFortran::get_runtime_library_dir();

        std::string arg_o;
        std::vector<std::string> arg_files;
        bool arg_version = false;
        bool show_tokens = false;
        bool show_ast = false;
        bool show_asr = false;
        bool show_llvm = false;

        CompilerOptions compiler_options;

        CLI::App app{"ComPy: modern interactive LLVM-based ComPy-Lang compiler"};

        app.add_option("files", arg_files, "Source files");
        app.add_option("-o", arg_o, "Specify the file to place the output into");

        // ComPy Version
        app.add_flag("--version", arg_version, "Display compiler version information");

        // ComPy specific options
        app.add_flag("--show-tokens", show_tokens, "Show tokens for the given file and exit");
        app.add_flag("--show-ast", show_ast, "Show AST for the given file and exit");
        app.add_flag("--show-asr", show_asr, "Show ASR for the given file and exit");
        app.add_flag("--show-llvm", show_llvm, "Show LLVM IR for the given file and exit");
        app.add_flag("--indent", compiler_options.indent, "Indented print ASR/AST");

        app.get_formatter()->column_width(25);
        CLI11_PARSE(app, argc, argv);

        if (arg_version) {
            std::string version = LFORTRAN_VERSION;
            std::cout << "ComPy version: " << version << std::endl;
            std::cout << "Platform: ";
            switch (compiler_options.platform) {
                case (LFortran::Platform::Linux) : std::cout << "Linux"; break;
                case (LFortran::Platform::macOS_Intel) : std::cout << "macOS Intel"; break;
                case (LFortran::Platform::macOS_ARM) : std::cout << "macOS ARM"; break;
                case (LFortran::Platform::Windows) : std::cout << "Windows"; break;
                case (LFortran::Platform::FreeBSD) : std::cout << "FreeBSD"; break;
            }
            std::cout << std::endl;
            return 0;
        }

        // TODO: for now we ignore the other filenames, only handle the first:
        std::string arg_file = arg_files[0];

        std::string outfile;
        std::string basename;
        basename = remove_extension(arg_file);
        basename = remove_path(basename);
        if (arg_o.size() > 0) {
            outfile = arg_o;
        } else if (show_tokens) {
            outfile = basename + ".tok";
        } else if (show_ast) {
            outfile = basename + ".ast";
        } else if (show_asr) {
            outfile = basename + ".asr";
        } else if (show_llvm) {
            outfile = basename + ".ll";
        } else {
            outfile = "a.out";
        }

        if (show_tokens) {
            return emit_tokens(arg_file, true, compiler_options);
        }
        if (show_ast) {
            return emit_ast(arg_file, compiler_options);
        }
        if (show_asr) {
            return emit_asr(arg_file, compiler_options);
        }

        if (show_llvm) {
#ifdef HAVE_LFORTRAN_LLVM
            return emit_llvm(arg_file, compiler_options);
#else
            std::cerr << "The --show-llvm option requires the LLVM backend to be enabled. Recompile with `WITH_LLVM=yes`." << std::endl;
            return 1;
#endif
        }

        if (endswith(arg_file, ".cp"))
        {
            std::string tmp_o = outfile + ".tmp.o";
            int err;
#ifdef HAVE_LFORTRAN_LLVM
                err = compile_to_object_file(arg_file, tmp_o, compiler_options);
#else
                std::cerr << "Compiling Python files to object files requires the LLVM backend to be enabled. Recompile with `WITH_LLVM=yes`." << std::endl;
                return 1;
#endif
            if (err) return err;
            return link_executable({tmp_o}, outfile, runtime_library_dir,
                    Backend::llvm, compiler_options);
        } else {
            return link_executable(arg_files, outfile, runtime_library_dir,
                    Backend::llvm, compiler_options);
        }

    } catch(const LFortran::LFortranException &e) {
        std::cerr << "Internal Compiler Error: Unhandled exception" << std::endl;
        std::vector<LFortran::StacktraceItem> d = e.stacktrace_addresses();
        get_local_addresses(d);
        get_local_info(d);
        std::cerr << stacktrace2str(d, LFortran::stacktrace_depth);
        std::cerr << e.name() + ": " << e.msg() << std::endl;
        return 1;
    } catch(const std::runtime_error &e) {
        std::cerr << "runtime_error: " << e.what() << std::endl;
        return 1;
    } catch(const std::exception &e) {
        std::cerr << "std::exception: " << e.what() << std::endl;
        return 1;
    } catch(...) {
        std::cerr << "Unknown Exception" << std::endl;
        return 1;
    }
    return 0;
}
