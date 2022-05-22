#ifndef COMPY_PARSER_SEMANTICS_H
#define COMPY_PARSER_SEMANTICS_H

/*
   This header file contains parser semantics: how the AST classes get
   constructed from the parser. This file only gets included in the generated
   parser cpp file, nowhere else.
   Note that this is part of constructing the AST from the source code, not the
   ComPy semantic phase (AST -> ASR).
*/

#include <cstring>

#include <compy/ast.h>
#include <libasr/string_utils.h>

// This is only used in parser.tab.cc, nowhere else, so we simply include
// everything from LFortran::AST to save typing:
using namespace LFortran::ComPy::AST;
using LFortran::Location;
using LFortran::Vec;

static inline char* name2char(const ast_t *n) {
    return down_cast2<Name_t>(n)->m_id;
}

Vec<ast_t*> A2LIST(Allocator &al, ast_t *x) {
    Vec<ast_t*> v;
    v.reserve(al, 1);
    v.push_back(al, x);
    return v;
}

static inline char** REDUCE_ARGS(Allocator &al, const Vec<ast_t*> args) {
    char **a = al.allocate<char*>(args.size());
    for (size_t i=0; i < args.size(); i++) {
        a[i] = name2char(args.p[i]);
    }
    return a;
}

template <typename T, astType type>
static inline T** vec_cast(const Vec<ast_t*> &x) {
    T **s = (T**)x.p;
    for (size_t i=0; i < x.size(); i++) {
        LFORTRAN_ASSERT((s[i]->base.type == type))
    }
    return s;
}

#define VEC_CAST(x, type) vec_cast<type##_t, astType::type>(x)
#define STMTS(x) VEC_CAST(x, stmt)
#define EXPRS(x) VEC_CAST(x, expr)

#define EXPR(x) (down_cast<expr_t>(x))
#define STMT(x) (down_cast<stmt_t>(x))
#define RESULT(x) p.result.push_back(p.m_a, STMT(x))

#define LIST_NEW(l) l.reserve(p.m_a, 4)
#define LIST_ADD(l, x) l.push_back(p.m_a, x)
#define PLIST_ADD(l, x) l.push_back(p.m_a, *x)

#define EXPR_01(e, l) make_Expr_t(p.m_a, l, EXPR(e))

#define ASSIGNMENT(targets, val, l) make_Assign_t(p.m_a, l, \
        EXPRS(targets), targets.size(), EXPR(val))
#define TARGET_ID(name, l) make_Name_t(p.m_a, l, \
        name2char(name), expr_contextType::Store)

#define ANNASSIGN_01(x, y, l) make_AnnAssign_t(p.m_a, l, \
        EXPR(x), EXPR(y), nullptr)
#define ANNASSIGN_02(x, y, val, l) make_AnnAssign_t(p.m_a, l, \
        EXPR(x), EXPR(y), EXPR(val))

#define OPERATOR(op, l) operatorType::op
#define AUGASSIGN_01(x, op, y, l) make_AugAssign_t(p.m_a, l, EXPR(x), op, EXPR(y))

#define RETURN_01(l) make_Return_t(p.m_a, l, nullptr)
#define RETURN_02(e, l) make_Return_t(p.m_a, l, EXPR(e))

#define IF_STMT_01(e, stmt, l) make_If_t(p.m_a, l, \
        EXPR(e), STMTS(stmt), stmt.size(), nullptr, 0)
#define IF_STMT_02(e, stmt, orelse, l) make_If_t(p.m_a, l, \
        EXPR(e), STMTS(stmt), stmt.size(), STMTS(orelse), orelse.size())
#define IF_STMT_03(e, stmt, orelse, l) make_If_t(p.m_a, l, \
        EXPR(e), STMTS(stmt), stmt.size(), STMTS(A2LIST(p.m_a, orelse)), 1)

#define FOR_01(target, iter, stmts, l) make_For_t(p.m_a, l, \
        EXPR(target), EXPR(iter), STMTS(stmts), stmts.size(), nullptr, 0)
#define FOR_02(target, iter, stmts, orelse, l) make_For_t(p.m_a, l, \
        EXPR(target), EXPR(iter), STMTS(stmts), stmts.size(), \
        STMTS(orelse), orelse.size())

static inline arg_t *FUNC_ARG(Allocator &al, Location &l, char *arg, expr_t* e) {
    arg_t *r = al.allocate<arg_t>();
    r->loc = l;
    r->m_arg = arg;
    r->m_annotation = e;
    return r;
}

static inline arguments_t FUNC_ARGS(Location &l, arg_t* m_args, size_t n_args) {
    arguments_t r;
    r.loc = l;
    r.m_args = m_args;
    r.n_args = n_args;
    return r;
}

#define ARGS_01(arg, l) FUNC_ARG(p.m_a, l, name2char((ast_t *)arg), nullptr)
#define ARGS_02(arg, annotation, l) FUNC_ARG(p.m_a, l, \
        name2char((ast_t *)arg), EXPR(annotation))
#define FUNCTION_01(id, args, stmts, l) \
        make_FunctionDef_t(p.m_a, l, name2char(id), \
        FUNC_ARGS(l, args.p, args.n), \
        STMTS(stmts), stmts.size(), \
        nullptr, 0, nullptr)
#define FUNCTION_02(id, args, return, stmts, l) \
        make_FunctionDef_t(p.m_a, l, name2char(id), \
        FUNC_ARGS(l, args.p, args.n), \
        STMTS(stmts), stmts.size(), \
        nullptr, 0, EXPR(return))

Vec<ast_t*> MERGE_EXPR(Allocator &al, ast_t *x, ast_t *y) {
    Vec<ast_t*> v;
    v.reserve(al, 2);
    v.push_back(al, x);
    v.push_back(al, y);
    return v;
}

#define BOOLOP(x, op, y, l) make_BoolOp_t(p.m_a, l, \
        boolopType::op, EXPRS(MERGE_EXPR(p.m_a, x, y)), 2)
#define BINOP(x, op, y, l) make_BinOp_t(p.m_a, l, \
        EXPR(x), operatorType::op, EXPR(y))
#define UNARY(x, op, l) make_UnaryOp_t(p.m_a, l, unaryopType::op, EXPR(x))
#define COMPARE(x, op, y, l) make_Compare_t(p.m_a, l, \
EXPR(x), cmpopType::op, EXPRS(A2LIST(p.m_a, y)), 1)
#define SYMBOL(x, l) make_Name_t(p.m_a, l, x.c_str(p.m_a), expr_contextType::Load)
// `x.int_n` is of type BigInt but we store the int64_t directly in AST
#define INTEGER(x, l) make_ConstantInt_t(p.m_a, l, x.int_n.n, nullptr)
#define STRING(x, l) make_ConstantStr_t(p.m_a, l, x.c_str(p.m_a), nullptr)
#define FLOAT(x, l) make_ConstantFloat_t(p.m_a, l, \
        std::stof(x.c_str(p.m_a)), nullptr)
#define BOOL(x, l) make_ConstantBool_t(p.m_a, l, x, nullptr)
#define CALL_01(func, args, l) make_Call_t(p.m_a, l, \
        EXPR(func), EXPRS(args), args.size())



#endif
