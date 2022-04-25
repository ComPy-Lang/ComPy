#include <iostream>

#define CLI11_HAS_FILESYSTEM 0
#include <bin/CLI11.hpp>

#include <libasr/stacktrace.h>
#include <compy/utils.h>
#include <compy/semantics/ast_to_asr.h>
#include <compy/parser/tokenizer.h>
#include <compy/parser/parser.h>
#include <libasr/config.h>
#include <libasr/string_utils.h>

namespace {

using LFortran::CompilerOptions;
using LFortran::parse_file;

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

} // anonymous namespace

int main(int argc, char *argv[])
{
    LFortran::initialize();

    try {
        std::vector<std::string> arg_files;
        bool arg_version = false;
        bool show_tokens = false;
        bool show_ast = false;

        CompilerOptions compiler_options;

        CLI::App app{"ComPy: modern interactive LLVM-based ComPy-Lang compiler"};

        app.add_option("files", arg_files, "Source files");

        // ComPy Version
        app.add_flag("--version", arg_version, "Display compiler version information");

        // ComPy specific options
        app.add_flag("--show-tokens", show_tokens, "Show tokens for the given file and exit");
        app.add_flag("--show-ast", show_ast, "Show AST for the given file and exit");
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

        if (show_tokens) {
            return emit_tokens(arg_file, true, compiler_options);
        }
        if (show_ast) {
            return emit_ast(arg_file, compiler_options);
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
