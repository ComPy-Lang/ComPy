#ifndef COMPY_SEMANTICS_COMPTIME_EVAL_H
#define COMPY_SEMANTICS_COMPTIME_EVAL_H

#include <complex>
#include <string>
#include <cstring>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <iterator>

#include <libasr/asr.h>
#include <libasr/bigint.h>
#include <libasr/string_utils.h>
#include <compy/utils.h>
#include <compy/semantics/semantic_exception.h>

namespace LFortran {

struct PythonIntrinsicProcedures {

    const std::string m_builtin = "builtin_functions";

    typedef ASR::expr_t* (*comptime_eval_callback)(Allocator &, const Location &, Vec<ASR::expr_t*> &);
    // Table of intrinsics
    // The callback is only called if all arguments have compile time `value`
    // which is always one of the `Constant*` expression ASR nodes, so inside
    // the callback one can assume that.
    std::map<std::string, std::tuple<std::string, comptime_eval_callback>> comptime_eval_map;

    PythonIntrinsicProcedures() {
        comptime_eval_map = {
            {"abs", {m_builtin, &eval_abs}},
            {"str", {m_builtin, &eval_str}},
            {"bool", {m_builtin, &eval_bool}},
            {"len", {m_builtin, &eval_len}},
            {"pow", {m_builtin, &eval_pow}},
            {"round", {m_builtin, &eval_round}},
            {"_mod", {m_builtin, &eval__mod}},
            {"max" , {m_builtin , &eval_max}},
            {"min" , {m_builtin , &eval_min}}
        };
    }

    // Return `true` if `name` is in the table of intrinsics
    bool is_intrinsic(std::string name) const {
        auto search = comptime_eval_map.find(name);
        if (search != comptime_eval_map.end()) {
            return true;
        } else {
            return false;
        }
    }

    // Looks up `name` in the table of intrinsics and returns the corresponding
    // module name; Otherwise rises an exception
    std::string get_module(std::string name, const Location &loc) const {
        auto search = comptime_eval_map.find(name);
        if (search != comptime_eval_map.end()) {
            std::string module_name = std::get<0>(search->second);
            return module_name;
        } else {
            throw SemanticError("Function '" + name
                + "' not found among intrinsic procedures",
                loc);
        }
    }

    // Evaluates the intrinsic function `name` at compile time
    ASR::expr_t *comptime_eval(std::string name, Allocator &al, const Location &loc, Vec<ASR::call_arg_t> &args) const {
        auto search = comptime_eval_map.find(name);
        if (search != comptime_eval_map.end()) {
            comptime_eval_callback cb = std::get<1>(search->second);
            Vec<ASR::call_arg_t> arg_values = ASRUtils::get_arg_values(al, args);
            if (arg_values.size() != args.size()) {
                // Not all arguments have compile time values; we do not call the callback
                return nullptr;
            }
            Vec<ASR::expr_t*> expr_args;
            expr_args.reserve(al, arg_values.size());
            for( auto& a: arg_values ) {
                expr_args.push_back(al, a.m_value);
            }
            return cb(al, loc, expr_args);
        } else {
            throw SemanticError("Intrinsic function '" + name
                + "' compile time evaluation is not implemented yet",
                loc);
        }
    }

    static ASR::expr_t *eval_abs(Allocator &al, const Location &loc,
            Vec<ASR::expr_t*> &args
            ) {
        LFORTRAN_ASSERT(ASRUtils::all_args_evaluated(args));
        if (args.size() != 1) {
            throw SemanticError("abs() takes exactly one argument (" +
                std::to_string(args.size()) + " given)", loc);
        }
        ASR::expr_t* arg = args[0];
        ASR::ttype_t* t = ASRUtils::expr_type(args[0]);
        ASR::ttype_t *int_type = ASRUtils::TYPE(ASR::make_Integer_t(al, loc, 4, nullptr, 0));
        ASR::ttype_t *real_type = ASRUtils::TYPE(ASR::make_Real_t(al, loc, 8, nullptr, 0));
        if (ASRUtils::is_real(*t)) {
            double rv = ASR::down_cast<ASR::RealConstant_t>(arg)->m_r;
            double val = std::abs(rv);
            return ASR::down_cast<ASR::expr_t>(ASR::make_RealConstant_t(al, loc, val, real_type));
        } else if (ASRUtils::is_integer(*t)) {
            int64_t rv = ASR::down_cast<ASR::IntegerConstant_t>(arg)->m_n;
            int64_t val = std::abs(rv);
            return ASR::down_cast<ASR::expr_t>(ASR::make_IntegerConstant_t(al, loc, val, int_type));
        } else if (ASRUtils::is_logical(*t)) {
            int8_t val = ASR::down_cast<ASR::LogicalConstant_t>(arg)->m_value;
            return ASR::down_cast<ASR::expr_t>(ASR::make_IntegerConstant_t(al, loc, val, int_type));
        } else if (ASRUtils::is_complex(*t)) {
            double re = ASR::down_cast<ASR::ComplexConstant_t>(arg)->m_re;
            double im = ASR::down_cast<ASR::ComplexConstant_t>(arg)->m_im;
            std::complex<double> x(re, im);
            double result = std::abs(x);
            return ASR::down_cast<ASR::expr_t>(ASR::make_RealConstant_t(al, loc, result, real_type));
        } else {
            throw SemanticError("Argument of the abs function must be Integer, Real, Logical or Complex", loc);
        }
    }

    static ASR::expr_t *eval_str(Allocator &al, const Location &loc, Vec<ASR::expr_t*> &args) {
        LFORTRAN_ASSERT(ASRUtils::all_args_evaluated(args));
        ASR::ttype_t* str_type = ASRUtils::TYPE(ASR::make_Character_t(al,
            loc, 1, 1, nullptr, nullptr, 0));
        if (args.size() == 0) { // create an empty string
            return ASR::down_cast<ASR::expr_t>(ASR::make_StringConstant_t(al, loc, s2c(al, ""), str_type));
        }
        ASR::expr_t* arg = args[0];
        ASR::ttype_t* arg_type = ASRUtils::expr_type(arg);
        if (ASRUtils::is_integer(*arg_type)) {
            int64_t ival = ASR::down_cast<ASR::IntegerConstant_t>(arg)->m_n;
            std::string s = std::to_string(ival);
            return ASR::down_cast<ASR::expr_t>(ASR::make_StringConstant_t(al, loc, s2c(al, s), str_type));
        } else if (ASRUtils::is_real(*arg_type)) {
            double rval = ASR::down_cast<ASR::RealConstant_t>(arg)->m_r;
            std::string s = std::to_string(rval);
            return ASR::down_cast<ASR::expr_t>(ASR::make_StringConstant_t(al, loc, s2c(al, s), str_type));
        } else if (ASRUtils::is_logical(*arg_type)) {
            bool rv = ASR::down_cast<ASR::LogicalConstant_t>(arg)->m_value;
            std::string s = rv ? "True" : "False";
            return ASR::down_cast<ASR::expr_t>(ASR::make_StringConstant_t(al, loc, s2c(al, s), str_type));
        } else if (ASRUtils::is_character(*arg_type)) {
            char* c = ASR::down_cast<ASR::StringConstant_t>(arg)->m_s;
            std::string s = std::string(c);
            return ASR::down_cast<ASR::expr_t>(ASR::make_StringConstant_t(al, loc, s2c(al, s), str_type));
        } else {
            throw SemanticError("str() argument must be real, integer, logical, or a string, not '" +
                ASRUtils::type_to_str_python(arg_type) + "'", loc);
        }
    }

    static ASR::expr_t *eval_bool(Allocator &al, const Location &loc, Vec<ASR::expr_t*> &args) {
        LFORTRAN_ASSERT(ASRUtils::all_args_evaluated(args));
        if (args.size() != 1) {
            throw SemanticError("bool() takes exactly one argument (" +
                std::to_string(args.size()) + " given)", loc);
        }
        ASR::ttype_t *type = ASRUtils::TYPE(ASR::make_Logical_t(al, loc,
            1, nullptr, 0));
        ASR::expr_t* arg = args[0];
        ASR::ttype_t* t = ASRUtils::expr_type(arg);
        bool result;
        if (ASRUtils::is_real(*t)) {
            result = ASR::down_cast<ASR::RealConstant_t>(arg)->m_r;
        } else if (ASRUtils::is_integer(*t)) {
            result = ASR::down_cast<ASR::IntegerConstant_t>(arg)->m_n;
        } else if (ASRUtils::is_complex(*t)) {
            double re = ASR::down_cast<ASR::ComplexConstant_t>(arg)->m_re;
            double im = ASR::down_cast<ASR::ComplexConstant_t>(arg)->m_im;
            std::complex<double> c(re, im);
            result = (re || im);
        } else if (ASRUtils::is_logical(*t)) {
            result = ASR::down_cast<ASR::LogicalConstant_t>(arg)->m_value;
        } else if (ASRUtils::is_character(*t)) {
            char* c = ASR::down_cast<ASR::StringConstant_t>(ASRUtils::expr_value(arg))->m_s;
            result = strlen(s2c(al, std::string(c)));
        } else {
            throw SemanticError("bool() must have one real, integer, character,"
                " complex, or logical argument, not '" + ASRUtils::type_to_str_python(t) + "'", loc);
        }
        return ASR::down_cast<ASR::expr_t>(make_LogicalConstant_t(al, loc, result, type));
    }

    static ASR::expr_t *eval__mod(Allocator &al, const Location &loc, Vec<ASR::expr_t*> &args) {
        LFORTRAN_ASSERT(ASRUtils::all_args_evaluated(args));
        if (args.size() != 2) {
            throw SemanticError("_mod() must have two integer/real arguments.", loc);
        }
        ASR::expr_t* arg1 = args[0], *arg2 = args[1];
        LFORTRAN_ASSERT(ASRUtils::check_equal_type(ASRUtils::expr_type(arg1),
                                    ASRUtils::expr_type(arg2)));
        ASR::ttype_t* type = ASRUtils::expr_type(arg1);
        if (ASRUtils::is_integer(*type)) {
            int64_t a = ASR::down_cast<ASR::IntegerConstant_t>(arg1)->m_n;
            int64_t b = ASR::down_cast<ASR::IntegerConstant_t>(arg2)->m_n;
            return ASR::down_cast<ASR::expr_t>(
                ASR::make_IntegerConstant_t(al, loc, a%b, type));
        } else if (ASRUtils::is_real(*type)) {
            double a = ASR::down_cast<ASR::RealConstant_t>(arg1)->m_r;
            double b = ASR::down_cast<ASR::RealConstant_t>(arg2)->m_r;
            return ASR::down_cast<ASR::expr_t>(
                ASR::make_RealConstant_t(al, loc, std::fmod(a, b), type));
        } else {
            throw SemanticError("_mod() must have both integer or both real arguments.", loc);
        }
    }

    static ASR::expr_t *eval_len(Allocator &al, const Location &loc, Vec<ASR::expr_t*> &args) {
        LFORTRAN_ASSERT(ASRUtils::all_args_evaluated(args));
        if (args.size() != 1) {
            throw SemanticError("len() takes exactly one argument (" +
                std::to_string(args.size()) + " given)", loc);
        }
        ASR::expr_t *arg = args[0];
        ASR::ttype_t *type = ASRUtils::TYPE(ASR::make_Integer_t(al, loc, 4, nullptr, 0));
        if (arg->type == ASR::exprType::StringConstant) {
            char* str_value = ASR::down_cast<ASR::StringConstant_t>(arg)->m_s;
            return ASR::down_cast<ASR::expr_t>(make_IntegerConstant_t(al, loc,
                (int64_t)strlen(s2c(al, std::string(str_value))), type));
        } else if (arg->type == ASR::exprType::ArrayConstant) {
            return ASR::down_cast<ASR::expr_t>(ASR::make_IntegerConstant_t(al, loc,
                (int64_t)ASR::down_cast<ASR::ArrayConstant_t>(arg)->n_args, type));
        } else if (arg->type == ASR::exprType::TupleConstant) {
            return ASR::down_cast<ASR::expr_t>(make_IntegerConstant_t(al, loc,
                (int64_t)ASR::down_cast<ASR::TupleConstant_t>(arg)->n_elements, type));
        } else if (arg->type == ASR::exprType::DictConstant) {
            return ASR::down_cast<ASR::expr_t>(make_IntegerConstant_t(al, loc,
                (int64_t)ASR::down_cast<ASR::DictConstant_t>(arg)->n_keys, type));
        } else if (arg->type == ASR::exprType::SetConstant) {
            return ASR::down_cast<ASR::expr_t>(make_IntegerConstant_t(al, loc,
                (int64_t)ASR::down_cast<ASR::SetConstant_t>(arg)->n_elements, type));
        } else {
            throw SemanticError("len() only works on strings, lists, tuples, dictionaries and sets",
                loc);
        }
    }

    static ASR::expr_t *eval_pow(Allocator &al, const Location &loc, Vec<ASR::expr_t*> &args) {
        LFORTRAN_ASSERT(ASRUtils::all_args_evaluated(args));
        ASR::expr_t* arg1 = args[0];
        ASR::expr_t* arg2 = args[1];
        ASR::ttype_t* arg1_type = ASRUtils::expr_type(arg1);
        ASR::ttype_t* arg2_type = ASRUtils::expr_type(arg2);
        ASR::ttype_t *int_type = ASRUtils::TYPE(ASR::make_Integer_t(al, loc, 4, nullptr, 0));
        ASR::ttype_t *real_type = ASRUtils::TYPE(ASR::make_Real_t(al, loc, 8, nullptr, 0));
        ASR::ttype_t *complex_type = ASRUtils::TYPE(ASR::make_Complex_t(al, loc, 8, nullptr, 0));
        if (ASRUtils::is_integer(*arg1_type) && ASRUtils::is_integer(*arg2_type)) {
            int64_t a = ASR::down_cast<ASR::IntegerConstant_t>(arg1)->m_n;
            int64_t b = ASR::down_cast<ASR::IntegerConstant_t>(arg2)->m_n;
            if (a == 0 && b < 0) { // Zero Division
                throw SemanticError("0.0 cannot be raised to a negative power.", loc);
            }
            if (b < 0) // Negative power
                return ASR::down_cast<ASR::expr_t>(make_RealConstant_t(al, loc,
                    pow(a, b), real_type));
            else // Positive power
                return ASR::down_cast<ASR::expr_t>(make_IntegerConstant_t(al, loc,
                    (int64_t)pow(a, b), int_type));

        } else if (ASRUtils::is_real(*arg1_type) && ASRUtils::is_real(*arg2_type)) {
            double a = ASR::down_cast<ASR::RealConstant_t>(arg1)->m_r;
            double b = ASR::down_cast<ASR::RealConstant_t>(arg2)->m_r;
            if (a == 0.0 && b < 0.0) { // Zero Division
                throw SemanticError("0.0 cannot be raised to a negative power.", loc);
            }
            return ASR::down_cast<ASR::expr_t>(make_RealConstant_t(al, loc,
                pow(a, b), real_type));

        } else if (ASRUtils::is_integer(*arg1_type) && ASRUtils::is_real(*arg2_type)) {
            int64_t a = ASR::down_cast<ASR::IntegerConstant_t>(arg1)->m_n;
            double b = ASR::down_cast<ASR::RealConstant_t>(arg2)->m_r;
            if (a == 0 && b < 0.0) { // Zero Division
                throw SemanticError("0.0 cannot be raised to a negative power.", loc);
            }
            return ASR::down_cast<ASR::expr_t>(make_RealConstant_t(al, loc,
                pow(a, b), real_type));

        } else if (ASRUtils::is_real(*arg1_type) && ASRUtils::is_integer(*arg2_type)) {
            double a = ASR::down_cast<ASR::RealConstant_t>(arg1)->m_r;
            int64_t b = ASR::down_cast<ASR::IntegerConstant_t>(arg2)->m_n;
            if (a == 0.0 && b < 0) { // Zero Division
                throw SemanticError("0.0 cannot be raised to a negative power.", loc);
            }
            return ASR::down_cast<ASR::expr_t>(make_RealConstant_t(al, loc,
                pow(a, b), real_type));

        } else if (ASRUtils::is_logical(*arg1_type) && ASRUtils::is_logical(*arg2_type)) {
            bool a = ASR::down_cast<ASR::LogicalConstant_t>(arg1)->m_value;
            bool b = ASR::down_cast<ASR::LogicalConstant_t>(arg2)->m_value;
            return ASR::down_cast<ASR::expr_t>(make_IntegerConstant_t(al, loc,
                pow(a, b), int_type));

        } else if (ASRUtils::is_complex(*arg1_type) && ASRUtils::is_integer(*arg2_type)) {
            double re = ASR::down_cast<ASR::ComplexConstant_t>(arg1)->m_re;
            double im = ASR::down_cast<ASR::ComplexConstant_t>(arg1)->m_im;
            std::complex<double> x(re, im);
            int64_t b = ASR::down_cast<ASR::IntegerConstant_t>(arg2)->m_n;
            std::complex<double> y = pow(x, b);
            return ASR::down_cast<ASR::expr_t>(make_ComplexConstant_t(al, loc,
                y.real(), y.imag(), complex_type));

        } else {
            throw SemanticError("pow() only works on integer, real, logical, and complex types", loc);
        }
    }

    static ASR::expr_t *eval_int(Allocator &al, const Location &loc, Vec<ASR::expr_t*> &args) {
        LFORTRAN_ASSERT(ASRUtils::all_args_evaluated(args));
        ASR::ttype_t *type = ASRUtils::TYPE(ASR::make_Integer_t(al, loc, 4, nullptr, 0));
        if (args.size() == 0) {
            return ASR::down_cast<ASR::expr_t>(make_IntegerConstant_t(al, loc, 0, type));
        }
        ASR::expr_t* int_expr = args[0];
        ASR::ttype_t* int_type = ASRUtils::expr_type(int_expr);
        if (ASRUtils::is_integer(*int_type)) {
            int64_t ival = ASR::down_cast<ASR::IntegerConstant_t>(int_expr)->m_n;
            return ASR::down_cast<ASR::expr_t>(make_IntegerConstant_t(al, loc, ival, type));

        } else if (ASRUtils::is_character(*int_type)) {
            // convert a string to an int
            char* c = ASR::down_cast<ASR::StringConstant_t>(int_expr)->m_s;
            std::string str = std::string(c);
            int64_t ival = std::stoll(str);
            return ASR::down_cast<ASR::expr_t>(make_IntegerConstant_t(al, loc, ival, type));

        } else if (ASRUtils::is_real(*int_type)) {
            int64_t ival = ASR::down_cast<ASR::RealConstant_t>(int_expr)->m_r;
            return ASR::down_cast<ASR::expr_t>(make_IntegerConstant_t(al, loc, ival, type));

        } else if (ASRUtils::is_logical(*int_type)) {
            bool rv = ASR::down_cast<ASR::LogicalConstant_t>(int_expr)->m_value;
            int8_t val = rv ? 1 : 0;
            return ASR::down_cast<ASR::expr_t>(make_IntegerConstant_t(al, loc, val, type));

        } else {
            throw SemanticError("int() argument must be real, integer, logical, or a string, not '" +
                ASRUtils::type_to_str_python(int_type) + "'", loc);
        }
    }

    static ASR::expr_t *eval_float(Allocator &al, const Location &loc, Vec<ASR::expr_t*> &args) {
        LFORTRAN_ASSERT(ASRUtils::all_args_evaluated(args));
        ASR::ttype_t* type = ASRUtils::TYPE(ASR::make_Real_t(al, loc, 8, nullptr, 0));
        if (args.size() == 0) {
            return ASR::down_cast<ASR::expr_t>(make_RealConstant_t(al, loc, 0.0, type));
        }
        ASR::expr_t* expr = args[0];
        ASR::ttype_t* float_type = ASRUtils::expr_type(expr);
        if (ASRUtils::is_real(*float_type)) {
            float rv = ASR::down_cast<ASR::RealConstant_t>(expr)->m_r;
            return ASR::down_cast<ASR::expr_t>(make_RealConstant_t(al, loc, rv, type));
        } else if (ASRUtils::is_integer(*float_type)) {
            double rv = ASR::down_cast<ASR::IntegerConstant_t>(expr)->m_n;
            return ASR::down_cast<ASR::expr_t>(make_RealConstant_t(al, loc, rv, type));
        } else if (ASRUtils::is_logical(*float_type)) {
            bool rv = ASR::down_cast<ASR::LogicalConstant_t>(expr)->m_value;
            float val = rv ? 1.0 : 0.0;
            return ASR::down_cast<ASR::expr_t>(make_RealConstant_t(al, loc, val, type));
        } else if (ASRUtils::is_character(*float_type)) {
            // convert a string to a float
            char* c = ASR::down_cast<ASR::StringConstant_t>(expr)->m_s;
            std::string str = std::string(c);
            float rv = std::stof(str);
            return ASR::down_cast<ASR::expr_t>(make_RealConstant_t(al, loc, rv, type));
        } else {
            throw SemanticError("float() argument must be real, integer, logical, or a string, not '" +
                ASRUtils::type_to_str_python(float_type) + "'", loc);
        }
    }

    static ASR::expr_t *eval_round(Allocator &al, const Location &loc, Vec<ASR::expr_t*> &args) {
        LFORTRAN_ASSERT(ASRUtils::all_args_evaluated(args));
        ASR::ttype_t *type = ASRUtils::TYPE(ASR::make_Integer_t(al, loc, 4, nullptr, 0));
        if (args.size() != 1) {
            throw SemanticError("round() missing required argument 'number' (pos 1)", loc);
        }
        ASR::expr_t* expr = args[0];
        ASR::ttype_t* t = ASRUtils::expr_type(expr);
        if (ASRUtils::is_real(*t)) {
            double rv = ASR::down_cast<ASR::RealConstant_t>(expr)->m_r;
            int64_t rounded = round(rv);
            if (fabs(rv-rounded) == 0.5)
                rounded = 2.0*round(rv/2.0);
            return ASR::down_cast<ASR::expr_t>(make_IntegerConstant_t(al, loc, rounded, type));
        } else if (ASRUtils::is_integer(*t)) {
            int64_t rv = ASR::down_cast<ASR::IntegerConstant_t>(expr)->m_n;
            return ASR::down_cast<ASR::expr_t>(make_IntegerConstant_t(al, loc, rv, type));
        } else if (ASRUtils::is_logical(*t)) {
            int64_t rv = ASR::down_cast<ASR::LogicalConstant_t>(expr)->m_value;
            return ASR::down_cast<ASR::expr_t>(make_IntegerConstant_t(al, loc, rv, type));
        } else {
            throw SemanticError("round() argument must be float, integer, or logical for now, not '" +
                ASRUtils::type_to_str_python(t) + "'", loc);
        }
    }

    static ASR::expr_t *eval_max(Allocator &/*al*/, const Location &loc, Vec<ASR::expr_t *> &args) {
        LFORTRAN_ASSERT(ASRUtils::all_args_evaluated(args));
        bool semantic_error_flag = args.size() != 0;
        std::string msg = "max() takes many arguments to comparing";
        ASR::expr_t *first_element = args[0];
        ASR::ttype_t *first_element_type = ASRUtils::expr_type(first_element);
        semantic_error_flag &= ASRUtils::is_integer(*first_element_type)
                               || ASRUtils::is_real(*first_element_type)
                               || ASRUtils::is_character(*first_element_type);
        int32_t biggest_ind = 0;
        if (semantic_error_flag) {
            if (ASRUtils::is_integer(*first_element_type)) {
                int32_t biggest = 0;
                for (size_t i = 0; i < args.size() && semantic_error_flag; i++) {
                    ASR::expr_t *current_arg = args[i];
                    ASR::ttype_t *current_arg_type = ASRUtils::expr_type(current_arg);
                    semantic_error_flag &= current_arg_type->type == first_element_type->type;
                    if (!semantic_error_flag) {
                        msg = "type of arg in index [" + std::to_string(i) = "] is not comparable";
                        break;
                    }
                    int32_t current_val = ASR::down_cast<ASR::IntegerConstant_t>(current_arg)->m_n;
                    if (i == 0) {
                        biggest = current_val;
                        biggest_ind = 0;
                    } else {
                        if (current_val > biggest) {
                            biggest = current_val;
                            biggest_ind = i;
                        }
                    }
                }
                if (semantic_error_flag) {
                    return args[biggest_ind];
                }
            } else if (ASRUtils::is_real(*first_element_type)) {
                double_t biggest = 0;
                for (size_t i = 0; i < args.size() && semantic_error_flag; i++) {
                    ASR::expr_t *current_arg = args[i];
                    ASR::ttype_t *current_arg_type = ASRUtils::expr_type(current_arg);
                    semantic_error_flag &= current_arg_type->type == first_element_type->type;
                    if (!semantic_error_flag) {
                        msg = "type of arg in index [" + std::to_string(i) = "] is not comparable";
                        break;
                    }
                    double_t current_val = ASR::down_cast<ASR::RealConstant_t>(current_arg)->m_r;
                    if (i == 0) {
                        biggest = current_val;
                        biggest_ind = 0;
                    } else {
                        if (current_val - biggest > 1e-6) {
                            biggest = current_val;
                            biggest_ind = i;
                        }
                    }
                }
                if (semantic_error_flag) {
                    return args[biggest_ind];
                }
            }
        }
        throw SemanticError(msg, loc);
    }

    static ASR::expr_t *eval_min(Allocator &/*al*/, const Location &loc, Vec<ASR::expr_t *> &args) {
        LFORTRAN_ASSERT(ASRUtils::all_args_evaluated(args));
        bool semantic_error_flag = args.size() != 0;
        std::string msg = "min() takes many arguments to comparing";
        ASR::expr_t *first_element = args[0];
        ASR::ttype_t *first_element_type = ASRUtils::expr_type(first_element);
        semantic_error_flag &= ASRUtils::is_integer(*first_element_type)
                               || ASRUtils::is_real(*first_element_type)
                               || ASRUtils::is_character(*first_element_type);
        int32_t smallest_ind = 0;
        if (semantic_error_flag) {
            if (ASRUtils::is_integer(*first_element_type)) {
                int32_t smallest = 0;
                for (size_t i = 0; i < args.size() && semantic_error_flag; i++) {
                    ASR::expr_t *current_arg = args[i];
                    ASR::ttype_t *current_arg_type = ASRUtils::expr_type(current_arg);
                    semantic_error_flag &= current_arg_type->type == first_element_type->type;
                    if (!semantic_error_flag) {
                        msg = "type of arg in index [" + std::to_string(i) = "] is not comparable";
                        break;
                    }
                    int32_t current_val = ASR::down_cast<ASR::IntegerConstant_t>(current_arg)->m_n;
                    if (i == 0) {
                        smallest = current_val;
                        smallest_ind = 0;
                    } else {
                        if (current_val < smallest) {
                            smallest = current_val;
                            smallest_ind = i;
                        }
                    }
                }
                if (semantic_error_flag) {
                    return args[smallest_ind];
                }
            } else if (ASRUtils::is_real(*first_element_type)) {
                double_t smallest = 0;
                for (size_t i = 0; i < args.size() && semantic_error_flag; i++) {
                    ASR::expr_t *current_arg = args[i];
                    ASR::ttype_t *current_arg_type = ASRUtils::expr_type(current_arg);
                    semantic_error_flag &= current_arg_type->type == first_element_type->type;
                    if (!semantic_error_flag) {
                        msg = "type of arg in index [" + std::to_string(i) = "] is not comparable";
                        break;
                    }
                    double_t current_val = ASR::down_cast<ASR::RealConstant_t>(current_arg)->m_r;
                    if (i == 0) {
                        smallest = current_val;
                        smallest_ind = 0;
                    } else {
                        if (smallest - current_val > 1e-6) {
                            smallest = current_val;
                            smallest_ind = i;
                        }
                    }
                }
                if (semantic_error_flag) {
                    return args[smallest_ind];
                }
            }
        }
        throw SemanticError(msg, loc);

    }



}; // ComptimeEval

} // namespace LFortran

#endif /* COMPY_SEMANTICS_COMPTIME_EVAL_H */
