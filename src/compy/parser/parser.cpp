#include <iostream>
#include <string>
#include <sstream>

#include <compy/parser/parser.h>
#include <compy/parser/parser.tab.hh>
#include <libasr/diagnostics.h>
#include <libasr/string_utils.h>
#include <libasr/utils.h>
#include <compy/parser/parser_exception.h>

namespace LFortran {

Result<ComPy::AST::Module_t*> parse(Allocator &al, const std::string &s,
        diag::Diagnostics &diagnostics)
{
    Parser p(al, diagnostics);
    try {
        p.parse(s);
    } catch (const parser_local::TokenizerError &e) {
        Error error;
        diagnostics.diagnostics.push_back(e.d);
        return error;
    } catch (const parser_local::ParserError &e) {
        Error error;
        diagnostics.diagnostics.push_back(e.d);
        return error;
    }

    Location l;
    if (p.result.size() == 0) {
        l.first=0;
        l.last=0;
    } else {
        l.first=p.result[0]->base.loc.first;
        l.last=p.result[p.result.size()-1]->base.loc.last;
    }
    return (ComPy::AST::Module_t*)ComPy::AST::make_Module_t(al, l,
        p.result.p, p.result.size());
}

void Parser::parse(const std::string &input)
{
    inp = input;
    if (inp.size() > 0) {
        if (inp[inp.size()-1] != '\n') inp.append("\n");
    } else {
        inp.append("\n");
    }
    m_tokenizer.set_string(inp);
    if (yyparse(*this) == 0) {
        return;
    }
    throw parser_local::ParserError("Parsing unsuccessful (internal compiler error)");
}

void Parser::handle_yyerror(const Location &loc, const std::string &msg)
{
    std::string message;
    if (msg == "syntax is ambiguous") {
        message = "Internal Compiler Error: syntax is ambiguous in the parser";
    } else if (msg == "syntax error") {
        LFortran::YYSTYPE yylval_;
        YYLTYPE yyloc_;
        this->m_tokenizer.cur = this->m_tokenizer.tok;
        int token = this->m_tokenizer.lex(this->m_a, yylval_, yyloc_, diag);
        if (token == yytokentype::END_OF_FILE) {
            message =  "End of file is unexpected here";
        } else if (token == yytokentype::TK_NEWLINE) {
            message =  "Newline is unexpected here";
        } else {
            std::string token_str = this->m_tokenizer.token();
            std::string token_type = token2text(token);
            if (token_str == token_type) {
                message =  "Token '" + token_str + "' is unexpected here";
            } else {
                message =  "Token '" + token_str + "' (of type '" + token2text(token) + "') is unexpected here";
            }
        }
    } else {
        message = "Internal Compiler Error: parser returned unknown error";
    }
    throw parser_local::ParserError(message, loc);
}

Result<ComPy::AST::ast_t*> parse_file(Allocator &al,
        const std::string &infile,
        diag::Diagnostics &diagnostics) {
    ComPy::AST::ast_t* ast;
    std::string input = read_file(infile);
    Result<ComPy::AST::Module_t*> res = parse(al, input, diagnostics);
    if (res.ok) {
        ast = (ComPy::AST::ast_t*)res.result;
    } else {
        LFORTRAN_ASSERT(diagnostics.has_error())
        return Error();
    }
    return ast;
}

} // LFortran
