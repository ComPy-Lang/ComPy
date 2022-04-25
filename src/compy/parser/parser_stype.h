#ifndef COMPY1_PARSER_STYPE_H
#define COMPY1_PARSER_STYPE_H

#include <cstring>
#include <compy/ast.h>
#include <libasr/location.h>
#include <libasr/containers.h>
#include <libasr/bigint.h>

namespace LFortran
{

struct IntSuffix {
    BigInt::BigInt int_n;
};

union YYSTYPE {
    int64_t n;
    Str string;
    IntSuffix int_suffix;

    ComPy::AST::ast_t* ast;
    Vec<ComPy::AST::ast_t*> vec_ast;
};

static_assert(std::is_standard_layout<YYSTYPE>::value);
static_assert(std::is_trivial<YYSTYPE>::value);

} // namespace LFortran


typedef struct LFortran::Location YYLTYPE;
#define YYLTYPE_IS_DECLARED 1
#define YYLTYPE_IS_TRIVIAL 0


#endif // COMPY1_PARSER_STYPE_H
