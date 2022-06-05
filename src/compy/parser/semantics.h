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

static inline expr_t* EXPR_OPT(const ast_t *f)
{
    if (f) {
        return EXPR(f);
    } else {
        return nullptr;
    }
}

#define LIST_NEW(l) l.reserve(p.m_a, 4)
#define LIST_ADD(l, x) l.push_back(p.m_a, x)
#define PLIST_ADD(l, x) l.push_back(p.m_a, *x)

#define EXPR_01(e, l) make_Expr_t(p.m_a, l, EXPR(e))

#define PASS(l) make_Pass_t(p.m_a, l)
#define BREAK(l) make_Break_t(p.m_a, l)
#define CONTINUE(l) make_Continue_t(p.m_a, l)

#define RAISE_01(l) make_Raise_t(p.m_a, l, nullptr, nullptr)
#define RAISE_02(exec, l) make_Raise_t(p.m_a, l, EXPR(exec), nullptr)

#define ASSIGNMENT(targets, val, l) make_Assign_t(p.m_a, l, \
        EXPRS(targets), targets.size(), EXPR(val))
#define TARGET_ID(name, l) make_Name_t(p.m_a, l, \
        name2char(name), expr_contextType::Store)
#define TARGET_SUBSCRIPT(value, slice, l) make_Subscript_t(p.m_a, l, \
        EXPR(value), CHECK_TUPLE(EXPR(slice)), expr_contextType::Store)


#define ANNASSIGN_01(x, y, l) make_AnnAssign_t(p.m_a, l, \
        EXPR(x), EXPR(y), nullptr)
#define ANNASSIGN_02(x, y, val, l) make_AnnAssign_t(p.m_a, l, \
        EXPR(x), EXPR(y), EXPR(val))

#define OPERATOR(op, l) operatorType::op
#define AUGASSIGN_01(x, op, y, l) make_AugAssign_t(p.m_a, l, EXPR(x), op, EXPR(y))

static inline alias_t *IMPORT_ALIAS_01(Allocator &al, Location &l,
        char *name, char *asname){
    alias_t *r = al.allocate<alias_t>();
    r->loc = l;
    r->m_name = name;
    r->m_asname = asname;
    return r;
}
static inline char *mod2char(Allocator &al, Vec<ast_t*> module) {
    std::string s = "";
    for (size_t i=0; i<module.size(); i++) {
        s.append(name2char(module[i]));
        if (i < module.size()-1)s.append(".");
    }
    LFortran::Str str;
    str.from_str_view(s);
    return str.c_str(al);
}
#define MOD_ID_01(module, l) IMPORT_ALIAS_01(p.m_a, l, \
        mod2char(p.m_a, module), nullptr)
#define MOD_ID_02(module, as_id, l) IMPORT_ALIAS_01(p.m_a, l, \
        mod2char(p.m_a, module), name2char(as_id))
#define MOD_ID_03(star, l) IMPORT_ALIAS_01(p.m_a, l, \
        (char *)"*", nullptr)

#define IMPORT_01(names, l) make_Import_t(p.m_a, l, names.p, names.size())
#define IMPORT_02(module, names, l) make_ImportFrom_t(p.m_a, l, \
        mod2char(p.m_a, module), names.p, names.size())

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
#define FUNCTION_03(decorator, id, args, stmts, l) \
        make_FunctionDef_t(p.m_a, l, name2char(id), \
        FUNC_ARGS(l, args.p, args.n), \
        STMTS(stmts), stmts.size(), \
        EXPRS(decorator), decorator.size(), nullptr)
#define FUNCTION_04(decorator, id, args, return, stmts, l) \
        make_FunctionDef_t(p.m_a, l, name2char(id), \
        FUNC_ARGS(l, args.p, args.n), \
        STMTS(stmts), stmts.size(), \
        EXPRS(decorator), decorator.size(), EXPR(return))

#define WHILE_STMT_01(e, stmts, l) make_While_t(p.m_a, l, \
        EXPR(e), STMTS(stmts), stmts.size(), nullptr, 0)
#define WHILE_STMT_02(e, stmts, orelse, l) make_While_t(p.m_a, l, \
        EXPR(e), STMTS(stmts), stmts.size(), STMTS(orelse), orelse.size())

static inline ast_t* SLICE(Allocator &al, Location &l,
        ast_t *lower, ast_t *upper, ast_t *_step) {
    expr_t* start = EXPR_OPT(lower);
    expr_t* end = EXPR_OPT(upper);
    expr_t* step = EXPR_OPT(_step);
    return make_Slice_t(al, l, start, end, step);
}

#define SLICE_01(lower, upper, step, l) SLICE(p.m_a, l, lower, upper, step)

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

static inline expr_t* CHECK_TUPLE(expr_t *x) {
    if(is_a<Tuple_t>(*x) && down_cast<Tuple_t>(x)->n_elts == 1) {
        return down_cast<Tuple_t>(x)->m_elts[0];
    } else {
        return x;
    }
}
#define TUPLE(elts, l) make_Tuple_t(p.m_a, l, \
        EXPRS(elts), elts.size(), expr_contextType::Load)
#define SUBSCRIPT_01(value, slice, l) make_Subscript_t(p.m_a, l, \
        EXPR(value), CHECK_TUPLE(EXPR(slice)), expr_contextType::Load)
#define ATTRIBUTE_REF(val, attr, l) make_Attribute_t(p.m_a, l, \
        EXPR(val), name2char(attr), expr_contextType::Load)
#define LIST(e, l) make_List_t(p.m_a, l, \
        EXPRS(e), e.size(), expr_contextType::Load)

#endif
