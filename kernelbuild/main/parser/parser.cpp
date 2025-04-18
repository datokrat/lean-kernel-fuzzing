/*
Copyright (c) 2025 Markus Himmel. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Markus Himmel
*/
#include "parser.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <charconv>

#define MARKUS_DEBUG

#ifdef MARKUS_DEBUG
    #define dbgf printf
#else
    #define dbgf(...)
#endif

namespace sz = ashvardanian::stringzilla;
using sz::literals::operator""_sz;

sz::string_view Parser::try_parse_string() {
    line = line.lstrip(sz::whitespaces_set());
    if (line.empty()) {
        return line;
    } else {
        auto [val, _, rest] = line.partition(" ");
        line = rest;
        return val;
    }
}

sz::string_view Parser::parse_string() {
    line = line.lstrip(sz::whitespaces_set());
    if (line.empty()) {
        dbgf("Nothing to read\n");
        error = true;
        return line;
    } else {
        auto [val, _, rest] = line.partition(" ");
        line = rest;
        return val;
    }
}

char Parser::convert_hexchar(sz::string_view val) {
    char result;
    auto [ptr, err] = std::from_chars(val.data(), val.data() + val.length(), result, 16);
    if (err == std::errc() && ptr == val.data() + val.length()) {
        return result;
    } else {
        dbgf("Not a valid hex char\n");
        error = true;
        return -1;
    }
}

sz::string Parser::parse_hexstring() {
    sz::string result;
    auto val = try_parse_string();
    while (!val.empty()) {
        auto chr = convert_hexchar(val);
        if (error) {
            return result;
        }
        result.push_back(chr);
        val = try_parse_string();
    }
    return result;
}

template<typename T>
T Parser::convert_numeric(sz::string_view val) {
    T result;
    auto [ptr, err] = std::from_chars(val.data(), val.data() + val.length(), result);
    if (err == std::errc() && ptr == val.data() + val.length()) {
        return result;
    } else {
        dbgf("Not a valid number\n");
        error = true;
        return -1;
    }
}

template<typename T>
T Parser::parse_numeric() {
    auto val = parse_string();
    if (error) return 0;
    auto result = convert_numeric<T>(val);
    return result;
}

template <typename T>
std::vector<T> Parser::parse_numeric_star() {
    std::vector<T> result;
    auto val = try_parse_string();
    while (!val.empty()) {
        auto num = convert_numeric<T>(val);
        if (error) {
            return result;
        }
        result.push_back(num);
        val = try_parse_string();
    }
    return result;
}

template <typename T>
std::vector<T> Parser::parse_numeric_amount(std::uint64_t n) {
    std::vector<T> result(n);
    for (std::uint64_t i = 0; i < n; ++i) {
        auto num = parse_numeric<T>();
        if (error) {
            return result;
        }
        result.push_back(num);
    }
    return result;
}

std::uint64_t Parser::parse_u64() {
    return parse_numeric<std::uint64_t>();
}

unsigned int Parser::parse_uint() {
    return parse_numeric<unsigned int>();
}

std::vector<std::uint64_t> Parser::parse_u64_star() {
    return parse_numeric_star<std::uint64_t>();
}

std::vector<std::uint64_t> Parser::parse_u64_amount(std::uint64_t n) {
    return parse_numeric_amount<std::uint64_t>(n);
}

lean::name Parser::parse_name_idx() {
    auto idx = parse_u64();
    if (error) return lean::name();
    if (idx >= names.size()) {
        dbgf("Referenced a name that doesn't exist");
        error = true;
        return lean::name();
    }
    return names[idx];
}

lean::level Parser::parse_level_idx() {
    auto idx = parse_u64();
    if (error) return lean::mk_level_zero();
    if (idx >= levels.size()) {
        dbgf("Referenced a level that doesn't exist");
        error = true;
        return lean::mk_level_zero();
    }
    return levels[idx];
}

std::vector<lean::level> Parser::parse_level_star() {
    std::vector<lean::level> result;
    auto val = try_parse_string();
    while (!val.empty()) {
        auto num = convert_numeric<std::uint64_t>(val);
        if (error) {
            return result;
        }
        if (num >= levels.size()) {
            dbgf("Referenced a level that doesn't exist");
            error = true;
            return result;
        }
        result.push_back(levels[num]);
        val = try_parse_string();
    }
    return result;
}

std::vector<lean::level> Parser::parse_level_amount(std::uint64_t n) {
    std::vector<lean::level> result(n);
    for (std::uint64_t i = 0; i < n; ++i) {
        auto num = parse_numeric<std::uint64_t>();
        if (error) {
            return result;
        }
        if (num >= levels.size()) {
            dbgf("Referenced a level that doesn't exist");
            error = true;
            return result;
        }
        result.push_back(levels[num]);
    }
    return result;
}

lean::expr Parser::parse_expr_idx() {
    auto idx = parse_u64();
    if (error) return lean::expr();
    if (idx >= exprs.size()) {
        dbgf("Referenced an expression that doens't exist");
        error = true;
        return lean::expr();
    }
    return exprs[idx];
}

bool Parser::parse_bool() {
    auto val = parse_u64();
    if (error) return false;
    if (val == 0) {
        return false;
    } else if (val == 1) {
        return true;
    } else {
        dbgf("Not a valid bool\n");
        error = true;
        return false;
    }
}

sz::string_view hint_o = "O"_sz;
sz::string_view hint_a = "A"_sz;
sz::string_view hint_r = "R"_sz;

Hint hintO () {
    return { O, 0 };
}

Hint hintA () {
    return { A, 0 };
}

Hint hintR(uint64_t val) {
    return { R, val };
}

Hint Parser::parse_hint() {
    auto type = parse_string();
    if (error) return hintO();
    if (type == hint_o) {
        return hintO();
    } else if (type == hint_a) {
        return hintA();
    } else if (type == hint_r) {
        auto val = parse_u64();
        if (error) return hintO();
        return hintR(val);
    } else {
        dbgf("Unknown hint type\n");
        error = true;
        return hintO();
    }
}

sz::string_view name_string = "#NS"_sz;
sz::string_view name_int = "#NI"_sz;

void Parser::parse_name() {
    auto index = parse_u64();
    if (error) return;
    if (index != names.size()) {
        dbgf("Expected a different index");
        error = true;
        return;
    }
    auto type = parse_string();
    if (error) return;
    auto parent = parse_name_idx();
    if (error) return;
    if (type == name_string) {
        auto comp = parse_string();
        if (error) return;
        
        std::string compstr = comp;
        lean::name n(parent, lean::string_ref(compstr));
        names.push_back(n);

        std::cout << "Have a string name: " << n.to_string() << std::endl;
    } else if (type == name_int) {
        auto comp = parse_u64();
        if (error) return;
        
        lean::name n(parent, lean::nat(comp));
        names.push_back(n);

        std::cout << "Have an int name: " << n.to_string() << std::endl;
    } else {
        dbgf("Unknown name type\n");
        error = true;
        return;
    }
}

sz::string_view universe_succ = "#US"_sz;
sz::string_view universe_max = "#UM"_sz;
sz::string_view universe_imax = "#UIM"_sz;
sz::string_view universe_parameter = "#UP"_sz;

void Parser::parse_level() {
    auto index = parse_u64();
    if (error) return;
    if (index != levels.size()) {
        dbgf("Expected a different index");
        error = true;
        return;
    }
    auto type = parse_string();
    if (type == universe_succ) {
        auto parent = parse_level_idx();
        if (error) return;
        lean::level l = lean::mk_succ(parent);
        levels.push_back(l);
        std::cout << "Have a universe successor" << std::endl;
    } else if (type == universe_max) {
        auto lhs = parse_level_idx();
        if (error) return;
        auto rhs = parse_level_idx();
        if (error) return;
        lean::level l = lean::mk_max(lhs, rhs);
        levels.push_back(l);
        std::cout << "Have a universe max" << std::endl;
    } else if (type == universe_imax) {
        auto lhs = parse_level_idx();
        if (error) return;
        auto rhs = parse_level_idx();
        if (error) return;
        lean::level l = lean::mk_imax(lhs, rhs);
        levels.push_back(l);
        std::cout << "Have a universe imax" << std::endl;
    } else if (type == universe_parameter) {
        auto parameter = parse_name_idx();
        if (error) return;
        lean::level l = lean::mk_univ_param(parameter);
        levels.push_back(l);
        std::cout << "Have a universe parameter" << std::endl;
    } else {
        dbgf("Unknown universe type\n");
        error = true;
        return;
    }
}

sz::string_view expression_variable = "#EV"_sz;
sz::string_view expression_sort = "#ES"_sz;
sz::string_view expression_constant = "#EC"_sz;
sz::string_view expression_application = "#EA"_sz;
sz::string_view expression_lambda = "#EL"_sz;
sz::string_view expression_pi = "#EP"_sz;
sz::string_view expression_let = "#EZ"_sz;
sz::string_view expression_projection = "#EJ"_sz;
sz::string_view expression_natlit = "#ELN"_sz;
sz::string_view expression_strlit = "#ELS"_sz;

void Parser::parse_expression() {
    auto index = parse_u64();
    if (error) return;
    if (index != exprs.size()) {
        dbgf("Expected a different index");
        error = true;
        return;
    }
    auto type = parse_string();
    if (error) return;
    if (type == expression_variable) {
        auto deBruijnIndex = parse_u64();
        if (error) return;
        lean::expr e = lean::mk_bvar(lean::nat(deBruijnIndex));
        exprs.push_back(e);
        std::cout << "Have a variable expression" << std::endl;
    } else if (type == expression_sort) {
        auto universe = parse_level_idx();
        if (error) return;
        lean::expr e = lean::mk_sort(universe);
        exprs.push_back(e);
        std::cout << "Have a sort expression" << std::endl;
    } else if (type == expression_constant) {
        auto name = parse_name_idx();
        if (error) return;
        auto universes = parse_level_star();
        if (error) return;
        lean::expr e = lean::mk_const(name, lean::levels(universes.begin(), universes.end()));
        exprs.push_back(e);
        std::cout << "Have a constant expression" << std::endl;
    } else if (type == expression_application) {
        auto lhs = parse_expr_idx();
        if (error) return;
        auto rhs = parse_expr_idx();
        if (error) return;
        lean::expr e = lean::mk_app(lhs, rhs);
        exprs.push_back(e);
        std::cout << "Have an application expression" << std::endl;
    } else if (type == expression_lambda) {
        parse_string(); // ignored, we don't care
        if (error) return;
        auto binderName = parse_name_idx();
        if (error) return;
        auto binderType = parse_expr_idx();
        if (error) return;
        auto body = parse_expr_idx();
        if (error) return;
        lean::expr e = lean::mk_lambda(binderName, binderType, body);
        exprs.push_back(e);
        std::cout << "Have a lambda expression" << std::endl;
    } else if (type == expression_pi) {
        parse_string(); // ignored, we don't care
        if (error) return;
        auto binderName = parse_name_idx();
        if (error) return;
        auto binderType = parse_expr_idx();
        if (error) return;
        auto body = parse_expr_idx();
        if (error) return;
        lean::expr e = lean::mk_pi(binderName, binderType, body);
        exprs.push_back(e);
        std::cout << "Have a pi expression" << std::endl;
    } else if (type == expression_let) {
        auto binderName = parse_name_idx();
        if (error) return;
        auto binderType = parse_expr_idx();
        if (error) return;
        auto boundValue = parse_expr_idx();
        if (error) return;
        auto body = parse_expr_idx();
        if (error) return;
        lean::expr e = lean::mk_let(binderName, binderType, boundValue, body);
        exprs.push_back(e);
        std::cout << "Have a let expression" << std::endl;
    } else if (type == expression_projection) {
        auto typeName = parse_name_idx();
        if (error) return;
        auto fieldIndex = parse_uint();
        if (error) return;
        auto value = parse_expr_idx();
        if (error) return;
        lean::expr e = lean::mk_proj(typeName, fieldIndex, value);
        exprs.push_back(e);
        std::cout << "Have a projection expression" << std::endl;
    } else if (type == expression_natlit) {
        auto value = parse_string();
        if (error) return;
        std::string s(value.begin(), value.end());
        lean::mpz num(s.c_str());
        lean::expr e = lean::mk_lit(lean::literal(num));
        exprs.push_back(e);
        std::cout << "Have a nat literal" << std::endl;
    } else if (type == expression_strlit) {
        auto value = parse_hexstring();
        if (error) return;
        lean::expr e = lean::mk_lit(lean::literal(value.c_str()));
        exprs.push_back(e);
        std::cout << "Have a string literal: " << value << std::endl;
    } else {
        dbgf("Unknown expression type\n");
        error = true;
        return;
    }
}

void Parser::parse_recrule() {
    std::cout << "Have a recrule: " << line << std::endl;
    line.remove_prefix(line.length());
}

void Parser::parse_axiom() {
    auto name = parse_u64();
    if (error) return;
    auto type = parse_u64();
    if (error) return;
    auto universeParameters = parse_u64_star();
    if (error) return;
    
    std::cout << "Have an axiom" << std::endl;
}

void Parser::parse_quotient() {
    auto name = parse_u64();
    if (error) return;
    auto type = parse_u64();
    if (error) return;
    auto universeParameters = parse_u64_star();
    if (error) return;
    std::cout << "Have a quotient" << std::endl;
}

void Parser::parse_definition() {
    auto name = parse_u64();
    if (error) return;
    auto type = parse_u64();
    if (error) return;
    auto value = parse_u64();
    if (error) return;
    auto hint = parse_hint();
    if (error) return;
    auto universeParameters = parse_u64_star();
    if (error) return;
    std::cout << "Have a definition" << std::endl;
}

void Parser::parse_theorem() {
    auto name = parse_u64();
    if (error) return;
    auto type = parse_u64();
    if (error) return;
    auto value = parse_u64();
    if (error) return;
    auto universeParameters = parse_u64_star();
    if (error) return;
    std::cout << "Have a theorem" << std::endl;
}

void Parser::parse_opaque() {
    auto name = parse_u64();
    if (error) return;
    auto type = parse_u64();
    if (error) return;
    auto value = parse_u64();
    if (error) return;
    auto universeParameters = parse_u64_star();
    if (error) return;
    std::cout << "Have a opaque" << std::endl;
}

void Parser::parse_inductive() {
    auto name = parse_u64();
    if (error) return;
    auto type = parse_u64();
    if (error) return;
    auto isRecursive = parse_bool();
    if (error) return;
    auto isNested = parse_bool();
    if (error) return;
    auto numParams = parse_u64();
    if (error) return;
    auto numIndices = parse_u64();
    if (error) return;
    auto numInductives = parse_u64();
    if (error) return;
    auto inductiveNames = parse_u64_amount(numInductives);
    if (error) return;
    auto numConstructors = parse_u64();
    if (error) return;
    auto constructorNames = parse_u64_amount(numConstructors);
    if (error) return;
    auto universeParameters = parse_u64_star();
    if (error) return;

    std::cout << "Have a inductive" << std::endl;
}

void Parser::parse_constructor() {
    auto name = parse_u64();
    if (error) return;
    auto type = parse_u64();
    if (error) return;
    auto parentInductive = parse_u64();
    if (error) return;
    auto constructorIndex = parse_u64();
    if (error) return;
    auto numParams = parse_u64();
    if (error) return;
    auto numFields = parse_u64();
    if (error) return;
    auto universeParameters = parse_u64_star();
    if (error) return;
    std::cout << "Have a constructor"  << std::endl;
}

void Parser::parse_recursor() {
    std::cout << "Have a recursor: " << line << std::endl;
    line.remove_prefix(line.length());
}

sz::string_view name_prefix = "#NAME"_sz;
sz::string_view level_prefix = "#LVL"_sz;
sz::string_view expression_prefix = "#EXPR"_sz;
sz::string_view recrule_prefix = "#RECR"_sz;
sz::string_view axiom_prefix = "#AX"_sz;
sz::string_view quotient_prefix = "#QUOT"_sz;
sz::string_view definition_prefix = "#DEF"_sz;
sz::string_view theorem_prefix = "#THM"_sz;
sz::string_view opaque_prefix = "#OPAQ"_sz;
sz::string_view inductive_prefix = "#IND"_sz;
sz::string_view constructor_prefix = "#CTOR"_sz;
sz::string_view recursor_prefix = "#REC"_sz;

void Parser::parse_line() {
    auto command = parse_string();
    if (error) return;
    if (command == name_prefix) {
        parse_name();        
    } else if (command == level_prefix) {
        parse_level();
    } else if (command == expression_prefix) {
        parse_expression();
    } else if (command == recrule_prefix) {
        parse_recrule();
    } else if (command == axiom_prefix) {
        parse_axiom();
    } else if (command == quotient_prefix) {
        parse_quotient();
    } else if (command == definition_prefix) {
        parse_definition();
    } else if (command == theorem_prefix) {
        parse_theorem();
    } else if (command == opaque_prefix) {
        parse_opaque();
    } else if (command == inductive_prefix) {
        parse_inductive();
    } else if (command == constructor_prefix) {
        parse_constructor();
    } else if (command == recursor_prefix) {
        parse_recursor();
    } else {
        dbgf("Not a known command type\n");
        error = true;
    }
    
    if (error) {
        return;
    }
    
    if (!line.empty()) {
        dbgf("Not fully consumed\n");
        std::cout << full_line << std::endl;
        error = true;
    }
}

sz::string_view expected_version = "markus-0.0.1\n"_sz;

void Parser::handle_file(sz::string_view file) {
    if (!file.starts_with(expected_version)) {
        dbgf("Version mismatch\n");
        error = true;
        return;
    }
    file.remove_prefix(expected_version.length());
    for (auto l : file.split("\n")) {
        if (!l.empty()) {
            line = l;
            full_line = l;
            parse_line();       
        }
        if (error) {
            break;
        }
    }
}

bool Parser::is_error() {
    return error;
}

Parser::Parser() : error(false) {
    levels.push_back(lean::mk_level_zero());
    names.push_back(lean::name::anonymous());
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Missing file name" << std::endl;
        return 0;
    }
    std::ifstream stream(argv[1]);
    std::stringstream buffer;
    buffer << stream.rdbuf();
    
    Parser p;
    p.handle_file(buffer.str());
    
    std::cout << "Error: " << p.is_error() << std::endl;

    return 0;
}