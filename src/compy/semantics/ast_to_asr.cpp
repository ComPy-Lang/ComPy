#include <compy/ast.h>
#include <compy/semantics/ast_to_asr.h>
#include <compy/utils.h>
#include <compy/parser/parser.h>


namespace LFortran::ComPy {

class PickleVisitor : public AST::PickleBaseVisitor<PickleVisitor>
{
public:
    std::string get_str() {
        return s;
    }
};

std::string pickle_ast(AST::ast_t &ast, bool colors) {
    PickleVisitor v;
    v.use_colors = colors;
    v.visit_ast(ast);
    return v.get_str();
}


} // namespace LFortran::Python
