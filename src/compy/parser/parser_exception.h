#ifndef COMPY_PARSER_PARSER_EXCEPTION_H
#define COMPY_PARSER_PARSER_EXCEPTION_H

#include <libasr/exception.h>
#include <libasr/diagnostics.h>

namespace LFortran {

namespace parser_local {

    // Local exceptions that are used to terminate the tokenizer.
    class TokenizerError
    {
    public:
        diag::Diagnostic d;
    public:
        TokenizerError(const std::string &msg, const Location &loc)
            : d{diag::Diagnostic(msg, diag::Level::Error, diag::Stage::Tokenizer, {
                diag::Label("", {loc})
            })}
        { }

        TokenizerError(const diag::Diagnostic &d) : d{d} { }
    };

    class ParserError
    {
    public:
        LFortran::diag::Diagnostic d;
    public:
        ParserError(const std::string &msg, const LFortran::Location &loc)
            : d{diag::Diagnostic(msg, diag::Level::Error, diag::Stage::Parser, {
                    diag::Label("", {loc})
                })}
        { }

        ParserError(const std::string &msg)
            : d{diag::Diagnostic(msg, diag::Level::Error, diag::Stage::Parser)}
        { }
    };

}

}


#endif // COMPY_PARSER_PARSER_EXCEPTION_H
