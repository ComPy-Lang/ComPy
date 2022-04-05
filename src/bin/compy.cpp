#include <iostream>

#define CLI11_HAS_FILESYSTEM 0
#include <bin/CLI11.hpp>

#include <compy/utils.h>


namespace {

using LFortran::CompilerOptions;

std::string read_file(const std::string &filename)
{
    std::ifstream ifs(filename.c_str(), std::ios::in | std::ios::binary
            | std::ios::ate);

    std::ifstream::pos_type filesize = ifs.tellg();
    if (filesize < 0) return std::string();

    ifs.seekg(0, std::ios::beg);

    std::vector<char> bytes(filesize);
    ifs.read(&bytes[0], filesize);

    return std::string(&bytes[0], filesize);
}

int emit_tokens(const std::string &infile, bool /*line_numbers*/, const CompilerOptions &/*compiler_options*/)
{
    std::string input = read_file(infile);
    // To be implemented
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

        CompilerOptions compiler_options;

        CLI::App app{"ComPy: modern interactive LLVM-based ComPy-Lang compiler"};

        app.add_option("files", arg_files, "Source files");

        // ComPy Version
        app.add_flag("--version", arg_version, "Display compiler version information");

        // ComPy specific options
        app.add_flag("--show-tokens", show_tokens, "Show tokens for the given python file and exit");

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
