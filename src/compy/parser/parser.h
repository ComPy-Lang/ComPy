#ifndef COMPY_PARSER_PARSER_H
#define COMPY_PARSER_PARSER_H

#include <libasr/containers.h>
#include <libasr/diagnostics.h>
#include <compy/parser/tokenizer.h>

namespace LFortran
{

class Parser
{
public:
    std::string inp;

public:
    diag::Diagnostics &diag;
    Allocator &m_a;
    Tokenizer m_tokenizer;
    Vec<ComPy::AST::stmt_t*> result;

    Parser(Allocator &al, diag::Diagnostics &diagnostics)
            : diag{diagnostics}, m_a{al} {
        result.reserve(al, 32);
    }

    void parse(const std::string &input);
    void handle_yyerror(const Location &loc, const std::string &msg);
};


// Parses ComPy code to AST
Result<ComPy::AST::Module_t*> parse(Allocator &al,
    const std::string &s,
    diag::Diagnostics &diagnostics);

Result<ComPy::AST::ast_t*> parse_file(Allocator &al,
        const std::string &infile,
        diag::Diagnostics &diagnostics);

} // namespace LFortran

#endif
