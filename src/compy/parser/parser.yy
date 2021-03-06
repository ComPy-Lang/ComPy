%require "3.0"
%define api.pure
%define api.value.type {LFortran::YYSTYPE}
%param {LFortran::Parser &p}
%locations
%expect    0   // shift/reduce conflicts

// Uncomment this to get verbose error messages
//%define parse.error verbose

/*
// Uncomment this to enable parser tracing. Then in the main code, set
// extern int yydebug;
// yydebug=1;
%define parse.trace
%printer { fprintf(yyo, "%s", $$.str().c_str()); } <string>
%printer { fprintf(yyo, "%d", $$); } <n>
%printer { std::cerr << "AST TYPE: " << $$->type; } <ast>
*/


%code requires // *.h
{
#include <compy/parser/parser.h>
}

%code // *.cpp
{

#include <compy/parser/parser.h>
#include <compy/parser/tokenizer.h>
#include <compy/parser/semantics.h>


int yylex(LFortran::YYSTYPE *yylval, YYLTYPE *yyloc, LFortran::Parser &p)
{
    return p.m_tokenizer.lex(p.m_a, *yylval, *yyloc, p.diag);
} // ylex
void yyerror(YYLTYPE *yyloc, LFortran::Parser &p, const std::string &msg)
{
    p.handle_yyerror(*yyloc, msg);
}


#define YYLLOC_DEFAULT(Current, Rhs, N)                                 \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first   = YYRHSLOC (Rhs, 1).first;                  \
          (Current).last    = YYRHSLOC (Rhs, N).last;                   \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first   = (Current).last   =                        \
            YYRHSLOC (Rhs, 0).last;                                     \
        }                                                               \
    while (0)

} // code


// -----------------------------------------------------------------------------
// List of tokens
// All tokens that we use (including "+" and such) are declared here first
// using the %token line. Each token will end up a member of the "enum
// yytokentype" in parser.tab.hh. Tokens can have a string equivalent (such as
// "+" for TK_PLUS) that is used later in the file to simplify reading it, but
// it is equivalent to TK_PLUS. Bison also allows so called "character token
// type" which are specified using single quotes (and that bypass the %token
// definitions), and those are not used here, and we test that the whole file
// does not contain any single quotes to ensure that.
//
// If this list is updated, update also token2text() in parser.cpp.

// Terminal tokens
%token END_OF_FILE 0
%token TK_NEWLINE
%token TK_INDENT
%token TK_DEDENT
%token <string> TK_NAME
%token <int_suffix> TK_INTEGER
%token <string> TK_FLOAT
%token <int_suffix> TK_IMAG_NUM
%token TK_PLUS "+"
%token TK_MINUS "-"
%token TK_STAR "*"
%token TK_SLASH "/"
%token TK_COLON ":"
%token TK_SEMICOLON ";"
%token TK_COMMA ","
%token TK_EQUAL "="
%token TK_LPAREN "("
%token TK_RPAREN ")"
%token TK_LBRACKET "["
%token TK_RBRACKET "]"
%token TK_LBRACE "{"
%token TK_RBRACE "}"
%token TK_PERCENT "%"
%token TK_VBAR "|"
%token TK_AMPERSAND "&"
%token TK_DOT "."
%token TK_TILDE "~"
%token TK_CARET "^"
%token TK_AT "@"
%token <string> TK_STRING
%token <string> TK_COMMENT
%token <string> TK_EOLCOMMENT
%token TK_POW "**"
%token TK_FLOOR_DIV "//"
%token TK_RIGHTSHIFT ">>"
%token TK_LEFTSHIFT "<<"
%token TK_PLUS_EQUAL "+="
%token TK_MIN_EQUAL "-="
%token TK_STAR_EQUAL "*="
%token TK_SLASH_EQUAL "/="
%token TK_PERCENT_EQUAL "%="
%token TK_AMPER_EQUAL "&="
%token TK_VBAR_EQUAL "|="
%token TK_CARET_EQUAL "^="
%token TK_ATEQUAL "@="
%token TK_RARROW "->"
%token TK_COLONEQUAL ":="
%token TK_ELLIPSIS "..."
%token TK_LEFTSHIFT_EQUAL "<<="
%token TK_RIGHTSHIFT_EQUAL ">>="
%token TK_POW_EQUAL "**="
%token TK_DOUBLESLASH_EQUAL "//="
%token TK_EQ "=="
%token TK_NE "!="
%token TK_LT "<"
%token TK_LE "<="
%token TK_GT ">"
%token TK_GE ">="
%token TK_NOT "not"
%token TK_AND "and"
%token TK_OR "or"
%token TK_TRUE "True"
%token TK_FALSE "False"


// Terminal tokens: Keywords
%token KW_AS
%token KW_ASSERT
%token KW_ASYNC
%token KW_AWAIT
%token KW_BREAK
%token KW_CLASS
%token KW_CONTINUE
%token KW_DEF
%token KW_DEL
%token KW_ELIF
%token KW_ELSE
%token KW_EXCEPT
%token KW_FINALLY
%token KW_FOR
%token KW_FROM
%token KW_GLOBAL
%token KW_IF
%token KW_IMPORT
%token KW_IN
%token KW_IS
%token KW_LAMBDA
%token KW_NONE
%token KW_NONLOCAL
%token KW_PASS
%token KW_RAISE
%token KW_RETURN
%token KW_TRY
%token KW_WHILE
%token KW_WITH
%token KW_YIELD

// Nonterminal tokens

%type <ast> id
%type <ast> expr
%type <vec_ast> expr_list
%type <vec_ast> expr_list_opt
%type <ast> script_unit
%type <ast> statement
%type <vec_ast> statements
%type <vec_ast> statements1
%type <ast> single_line_statement
%type <ast> multi_line_statement
%type <ast> assignment_statement
%type <ast> ann_assignment_statement
%type <vec_ast> target_list
%type <ast> target
%type <ast> augassign_statement
%type <operator_type> augassign_op
%type <ast> break_statement
%type <ast> continue_statement
%type <ast> expression_statement
%type <ast> import_statement
%type <vec_ast> module
%type <alias> module_as_id
%type <vec_alias> module_item_list
%type <ast> pass_statement
%type <ast> raise_statement
%type <ast> return_statement
%type <ast> if_statement
%type <ast> elif_statement
%type <ast> for_statement
%type <ast> function_def
%type <ast> tuple_list
%type <ast> while_statement
%type <ast> slice_item
%type <vec_ast> slice_item_list
%type <vec_arg> parameter_list_opt
%type <arg> parameter
%type <vec_ast> decorators
%type <vec_ast> sep
%type <ast> sep1

// Precedence

%left "or"
%left "and"
%precedence "not"
%left "==" "!=" ">=" ">" "<=" "<"
%left "-" "+"
%left "%" "/" "*"
%precedence UNARY
%right "**"
%precedence "."

%start units

%%

// The order of rules does not matter in Bison (unlike in ANTLR). The
// precedence is specified not by the order but by %left and %right directives
// as well as with %dprec.

// ----------------------------------------------------------------------------
// Top level rules to be used for parsing.

// Higher %dprec means higher precedence

units
    : units script_unit   { RESULT($2); }
    | script_unit         { RESULT($1); }
    | sep
    ;

script_unit
    : statement
    ;

statements
    : TK_INDENT statements1 TK_DEDENT { $$ = $2; }
    ;

statements1
    : statements1 statement { $$ = $1; LIST_ADD($$, $2); }
    | statement { LIST_NEW($$); LIST_ADD($$, $1); }
    ;

statement
    : single_line_statement sep
    | multi_line_statement
    ;

single_line_statement
    : assignment_statement
    | ann_assignment_statement
    | augassign_statement
    | break_statement
    | continue_statement
    | expression_statement
    | import_statement
    | pass_statement
    | raise_statement
    | return_statement
    ;

multi_line_statement
    : if_statement
    | for_statement
    | function_def
    | while_statement
    ;

expression_statement
    : expr { $$ = EXPR_01($1, @$); }
    ;

pass_statement
    : KW_PASS { $$ = PASS(@$); }
    ;

break_statement
    : KW_BREAK { $$ = BREAK(@$); }
    ;

continue_statement
    : KW_CONTINUE { $$ = CONTINUE(@$); }

raise_statement
    : KW_RAISE { $$ = RAISE_01(@$); }
    | KW_RAISE expr { $$ = RAISE_02($2, @$); }
    ;

target
    : id { $$ = TARGET_ID($1, @$); }
    | id "[" tuple_list "]" { $$ = TARGET_SUBSCRIPT($1, $3, @$); }
    ;

target_list
    : target_list target "=" { $$ = $1; LIST_ADD($$, $2); }
    | target "=" { LIST_NEW($$); LIST_ADD($$, $1); }
    ;

assignment_statement
    : target_list expr { $$ = ASSIGNMENT($1, $2, @$); }
    ;

ann_assignment_statement
    : target ":" expr { $$ = ANNASSIGN_01($1, $3, @$); }
    | target ":" expr "=" expr { $$ = ANNASSIGN_02($1, $3, $5, @$); }
    ;

augassign_statement
    : target augassign_op expr { $$ = AUGASSIGN_01($1, $2, $3, @$); }
    ;

augassign_op
    : "+=" { $$ = OPERATOR(Add, @$); }
    | "-=" { $$ = OPERATOR(Sub, @$); }
    | "*=" { $$ = OPERATOR(Mult, @$); }
    | "/=" { $$ = OPERATOR(Div, @$); }
    | "%=" { $$ = OPERATOR(Mod, @$); }
    | "**=" { $$ = OPERATOR(Pow, @$); }
    ;

module
    : module "." id { $$ = $1; LIST_ADD($$, $3); }
    | id { LIST_NEW($$); LIST_ADD($$, $1); }
    ;

module_as_id
    : module { $$ = MOD_ID_01($1, @$); }
    | module KW_AS id { $$ = MOD_ID_02($1, $3, @$); }
    | "*" { $$ = MOD_ID_03("*", @$); }
    ;

module_item_list
    : module_item_list "," module_as_id { $$ = $1; PLIST_ADD($$, $3); }
    | module_as_id { LIST_NEW($$); PLIST_ADD($$, $1); }
    ;

import_statement
    : KW_IMPORT module_item_list { $$ = IMPORT_01($2, @$); }
    | KW_FROM module KW_IMPORT module_item_list {
        $$ = IMPORT_02($2, $4, @$); }
    | KW_FROM module KW_IMPORT "(" module_item_list ")" {
        $$ = IMPORT_02($2, $5, @$); }
    ;

return_statement
    : KW_RETURN { $$ = RETURN_01(@$); }
    | KW_RETURN expr { $$ = RETURN_02($2, @$); }
    ;


elif_statement
    : KW_ELIF expr ":" sep statements { $$ = IF_STMT_01($2, $5, @$); }
    | KW_ELIF expr ":" sep statements KW_ELSE ":" sep statements {
        $$ = IF_STMT_02($2, $5, $9, @$); }
    | KW_ELIF expr ":" sep statements elif_statement {
        $$ = IF_STMT_03($2, $5, $6, @$); }
    ;

if_statement
    : KW_IF expr ":" sep statements { $$ = IF_STMT_01($2, $5, @$); }
    | KW_IF expr ":" sep statements KW_ELSE ":" sep statements {
        $$ = IF_STMT_02($2, $5, $9, @$); }
    | KW_IF expr ":" sep statements elif_statement {
        $$ = IF_STMT_03($2, $5, $6, @$); }
    ;

for_statement
    : KW_FOR target KW_IN expr ":" sep statements { $$ = FOR_01($2, $4, $7, @$); }
    | KW_FOR target KW_IN expr ":" sep statements KW_ELSE ":" sep statements {
        $$ = FOR_02($2, $4, $7, $11, @$); }
    ;

decorators
    : decorators "@" expr sep { $$ = $1; LIST_ADD($$, $3); }
    | "@" expr sep { LIST_NEW($$); LIST_ADD($$, $2); }
    ;

parameter
    : id { $$ = ARGS_01($1, @$); }
    | id ":" expr { $$ = ARGS_02($1, $3, @$); }
    ;

parameter_list_opt
    : parameter_list_opt "," parameter { $$ = $1; PLIST_ADD($$, $3); }
    | parameter { LIST_NEW($$); PLIST_ADD($$, $1); }
    | %empty { LIST_NEW($$); }
    ;

function_def
    : KW_DEF id "(" parameter_list_opt ")" ":" sep statements {
        $$ = FUNCTION_01($2, $4, $8, @$); }
    | KW_DEF id "(" parameter_list_opt ")" "->" expr ":"
        sep statements { $$ = FUNCTION_02($2, $4, $7, $10, @$); }
    | decorators KW_DEF id "(" parameter_list_opt ")" ":" sep statements {
        $$ = FUNCTION_03($1, $3, $5, $9, @$); }
    | decorators KW_DEF id "(" parameter_list_opt ")" "->" expr ":"
        sep statements { $$ = FUNCTION_04($1, $3, $5, $8, $11, @$); }
    ;

while_statement
    : KW_WHILE expr ":" sep statements { $$ = WHILE_STMT_01($2, $5, @$); }
    | KW_WHILE expr ":" sep statements KW_ELSE ":" sep statements {
        $$ = WHILE_STMT_02($2, $5, $9, @$); }
    ;

slice_item_list
    : slice_item_list "," slice_item { $$ = $1; LIST_ADD($$, $3); }
    | slice_item { LIST_NEW($$); LIST_ADD($$, $1); }
    ;

slice_item
    : ":"                    { $$ = SLICE_01(nullptr, nullptr, nullptr, @$); }
    | expr ":"               { $$ = SLICE_01(     $1, nullptr, nullptr, @$); }
    | ":" expr               { $$ = SLICE_01(nullptr,      $2, nullptr, @$); }
    | expr ":" expr          { $$ = SLICE_01(     $1,      $3, nullptr, @$); }
    | ":" ":"                { $$ = SLICE_01(nullptr, nullptr, nullptr, @$); }
    | ":" ":" expr           { $$ = SLICE_01(nullptr, nullptr,      $3, @$); }
    | expr ":" ":"           { $$ = SLICE_01(     $1, nullptr, nullptr, @$); }
    | ":" expr ":"           { $$ = SLICE_01(nullptr,      $2, nullptr, @$); }
    | expr ":" ":" expr      { $$ = SLICE_01(     $1, nullptr,      $4, @$); }
    | ":" expr ":" expr      { $$ = SLICE_01(nullptr,      $2,      $4, @$); }
    | expr ":" expr ":"      { $$ = SLICE_01(     $1,      $3, nullptr, @$); }
    | expr ":" expr ":" expr { $$ = SLICE_01(     $1,      $3,      $5, @$); }
    | expr                   { $$ = $1; }
    ;

tuple_list
    : slice_item_list { $$ = TUPLE($1, @$); }
    ;

expr_list_opt
    : expr_list { $$ = $1; }
    | %empty { LIST_NEW($$); }

expr_list
    : expr_list "," expr { $$ = $1; LIST_ADD($$, $3); }
    | expr { LIST_NEW($$); LIST_ADD($$, $1); }
    ;

expr
    : id { $$ = $1; }
    | TK_INTEGER { $$ = INTEGER($1, @$); }
    | TK_STRING { $$ = STRING($1, @$); }
    | TK_FLOAT { $$ = FLOAT($1, @$); }
    | TK_TRUE { $$ = BOOL(true, @$); }
    | TK_FALSE { $$ = BOOL(false, @$); }
    | "(" expr ")" { $$ = $2; }
    | "[" expr_list_opt "]" { $$ = LIST($2, @$); }
    | id "(" expr_list_opt ")" { $$ = CALL_01($1, $3, @$); }
    | expr "." id "(" expr_list_opt ")" {
        $$ = CALL_01(ATTRIBUTE_REF($1, $3, @$), $5, @$); }
    | id "[" tuple_list "]" { $$ = SUBSCRIPT_01($1, $3, @$); }
    | expr "." id { $$ = ATTRIBUTE_REF($1, $3, @$); }

    | expr "+" expr { $$ = BINOP($1, Add, $3, @$); }
    | expr "-" expr { $$ = BINOP($1, Sub, $3, @$); }
    | expr "*" expr { $$ = BINOP($1, Mult, $3, @$); }
    | expr "/" expr { $$ = BINOP($1, Div, $3, @$); }
    | expr "%" expr { $$ = BINOP($1, Mod, $3, @$); }
    | expr "**" expr { $$ = BINOP($1, Pow, $3, @$); }

    | "-" expr %prec UNARY { $$ = UNARY($2, USub, @$); }
    | "+" expr %prec UNARY { $$ = UNARY($2, UAdd, @$); }

    | expr "==" expr { $$ = COMPARE($1, Eq, $3, @$); }
    | expr "!=" expr { $$ = COMPARE($1, NotEq, $3, @$); }
    | expr "<" expr { $$ = COMPARE($1, Lt, $3, @$); }
    | expr "<=" expr { $$ = COMPARE($1, LtE, $3, @$); }
    | expr ">" expr { $$ = COMPARE($1, Gt, $3, @$); }
    | expr ">=" expr { $$ = COMPARE($1, GtE, $3, @$); }

    | expr "and" expr { $$ = BOOLOP($1, And, $3, @$); }
    | expr "or" expr { $$ = BOOLOP($1, Or, $3, @$); }
    | "not" expr { $$ = UNARY($2, Not, @$); }
    ;

id
    : TK_NAME { $$ = SYMBOL($1, @$); }
    ;

sep
    : sep sep1 { $$ = $1; LIST_ADD($$, $2); }
    | sep1 { LIST_NEW($$); LIST_ADD($$, $1); }
    ;

sep1
    : TK_NEWLINE {}
    | ";" {}
    ;
