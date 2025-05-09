/*
Copyright (c) 2025 Markus Himmel. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Markus Himmel
*/
#include "parser.h"

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

uint8_t Parser::convert_hexchar(sz::string_view val) {
    uint8_t result;
    auto [ptr, err] = std::from_chars(val.data(), val.data() + val.length(), result, 16);
    if (err == std::errc() && ptr == val.data() + val.length()) {
        return result;
    } else {
        dbgf("Not a valid hex char\n");
        std::cout << val << std::endl;
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
        result.push_back((char)chr);
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
    std::vector<T> result;
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

lean::name Parser::parse_name_idx(bool allowAnon) {
    auto idx = parse_u64();
    if (error) return lean::name();
    if (!allowAnon && idx == 0) {
        dbgf("Must not use anon name here\n");
        
        error = true;
        return lean::name();
    }
    if (idx >= names.size()) {
        dbgf("Referenced a name that doesn't exist\n");
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

template<typename T>
std::vector<T> Parser::parse_obj_star(const std::vector<T> & objs) {
    std::vector<T> result;
    auto val = try_parse_string();
    while (!val.empty()) {
        auto num = convert_numeric<std::uint64_t>(val);
        if (error) {
            return std::vector<T>();
        }
        if (num >= objs.size()) {
            dbgf("Referenced an object that doesn't exist");
            error = true;
            return std::vector<T>();
        }
        result.push_back(objs[num]);
        val = try_parse_string();
    }
    return result;
}

template<typename T>
std::vector<T> Parser::parse_obj_amount(std::uint64_t n, const std::vector<T> & objs) {
    std::vector<T> result;
    for (std::uint64_t i = 0; i < n; ++i) {
        auto num = parse_numeric<std::uint64_t>();
        if (error) {
            return std::vector<T>();
        }
        if (num >= objs.size()) {
            dbgf("Referenced an object that doesn't exist");
            error = true;
            return result;
        }
        result.push_back(objs[num]);
    }
    return result;
}

lean::levels Parser::parse_level_star() {
    auto vec = parse_obj_star<lean::level>(levels);
    return lean::list_ref<lean::level>(vec.begin(), vec.end());
}

lean::levels Parser::parse_level_amount(std::uint64_t n) {
    auto vec = parse_obj_amount<lean::level>(n, levels);
    return lean::list_ref<lean::level>(vec.begin(), vec.end());
}

lean::names Parser::parse_name_star() {
    auto vec = parse_obj_star<lean::name>(names);
    return lean::list_ref<lean::name>(vec.begin(), vec.end());
}

lean::names Parser::parse_name_amount(std::uint64_t n) {
    auto vec = parse_obj_amount<lean::name>(n, names);
    return lean::list_ref<lean::name>(vec.begin(), vec.end());
}

std::vector<lean::name> Parser::parse_name_vec_amount(std::uint64_t n) {
    return parse_obj_amount<lean::name>(n, names);
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

lean::reducibility_hints Parser::parse_hint() {
    auto type = parse_string();
    if (error) return lean::reducibility_hints::mk_opaque();
    if (type == hint_o) {
        return lean::reducibility_hints::mk_opaque();
    } else if (type == hint_a) {
        return lean::reducibility_hints::mk_abbreviation();
    } else if (type == hint_r) {
        auto val = parse_uint();
        if (error) return lean::reducibility_hints::mk_opaque();
        return lean::reducibility_hints::mk_regular(val);
    } else {
        dbgf("Unknown hint type\n");
        error = true;
        return lean::reducibility_hints::mk_opaque();
    }
}

sz::string_view name_string = "#NS"_sz;
sz::string_view name_int = "#NI"_sz;

void Parser::parse_name() {
    auto type = parse_string();
    if (error) return;
    auto parent = parse_name_idx(true);
    if (error) return;
    if (type == name_string) {
        auto comp = parse_string();
        if (error) return;
        
        std::string compstr = comp;
        lean::name n(parent, lean::string_ref(compstr));
        names.push_back(n);

    } else if (type == name_int) {
        auto comp = parse_u64();
        if (error) return;
        
        lean::name n(parent, lean::nat(comp));
        names.push_back(n);

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
    auto type = parse_string();
    if (type == universe_succ) {
        auto parent = parse_level_idx();
        if (error) return;
        lean::level l = lean::mk_succ(parent);
        levels.push_back(l);
    } else if (type == universe_max) {
        auto lhs = parse_level_idx();
        if (error) return;
        auto rhs = parse_level_idx();
        if (error) return;
        lean::level l = lean::mk_max(lhs, rhs);
        levels.push_back(l);
    } else if (type == universe_imax) {
        auto lhs = parse_level_idx();
        if (error) return;
        auto rhs = parse_level_idx();
        if (error) return;
        lean::level l = lean::mk_imax(lhs, rhs);
        levels.push_back(l);
    } else if (type == universe_parameter) {
        auto parameter = parse_name_idx(false);
        if (error) return;
        lean::level l = lean::mk_univ_param(parameter);
        levels.push_back(l);
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
    auto type = parse_string();
    if (error) return;
    if (type == expression_variable) {
        auto deBruijnIndex = parse_u64();
        if (error) return;
        lean::expr e = lean::mk_bvar(lean::nat(deBruijnIndex));
        exprs.push_back(e);
    } else if (type == expression_sort) {
        auto universe = parse_level_idx();
        if (error) return;
        lean::expr e = lean::mk_sort(universe);
        exprs.push_back(e);
    } else if (type == expression_constant) {
        auto name = parse_name_idx(false);
        if (error) return;
        auto universes = parse_level_star();
        if (error) return;
        lean::expr e = lean::mk_const(name, universes);
        exprs.push_back(e);
    } else if (type == expression_application) {
        auto lhs = parse_expr_idx();
        if (error) return;
        auto rhs = parse_expr_idx();
        if (error) return;
        lean::expr e = lean::mk_app(lhs, rhs);
        exprs.push_back(e);
    } else if (type == expression_lambda) {
        parse_string(); // ignored, we don't care
        if (error) return;
        auto binderName = parse_name_idx(true);
        if (error) return;
        auto binderType = parse_expr_idx();
        if (error) return;
        auto body = parse_expr_idx();
        if (error) return;
        lean::expr e = lean::mk_lambda(binderName, binderType, body);
        exprs.push_back(e);
    } else if (type == expression_pi) {
        parse_string(); // ignored, we don't care
        if (error) return;
        auto binderName = parse_name_idx(true);
        if (error) return;
        auto binderType = parse_expr_idx();
        if (error) return;
        auto body = parse_expr_idx();
        if (error) return;
        lean::expr e = lean::mk_pi(binderName, binderType, body);
        exprs.push_back(e);
    } else if (type == expression_let) {
        auto binderName = parse_name_idx(false);
        if (error) return;
        auto binderType = parse_expr_idx();
        if (error) return;
        auto boundValue = parse_expr_idx();
        if (error) return;
        auto body = parse_expr_idx();
        if (error) return;
        lean::expr e = lean::mk_let(binderName, binderType, boundValue, body);
        exprs.push_back(e);
    } else if (type == expression_projection) {
        auto typeName = parse_name_idx(false);
        if (error) return;
        auto fieldIndex = parse_uint();
        if (error) return;
        auto value = parse_expr_idx();
        if (error) return;
        lean::expr e = lean::mk_proj(typeName, fieldIndex, value);
        exprs.push_back(e);
    } else if (type == expression_natlit) {
        auto value = parse_string();
        if (error) return;
        std::string s(value.begin(), value.end());
        lean::mpz num(s.c_str());
        lean::expr e = lean::mk_lit(lean::literal(num));
        exprs.push_back(e);
    } else if (type == expression_strlit) {
        auto value = parse_hexstring();
        if (error) return;
        lean::expr e = lean::mk_lit(lean::literal(value.c_str()));
        exprs.push_back(e);
    } else {
        dbgf("Unknown expression type\n");
        error = true;
        return;
    }
}

void Parser::parse_axiom() {
    if (!prelude) {
        dbgf("Not accepting axioms");
        error = true;
        return;
    }
    auto name = parse_name_idx(false);
    if (error) return;
    auto type = parse_expr_idx();
    if (error) return;
    auto universeParameters = parse_name_star();
    if (error) return;
    
    lean::declaration d = lean::mk_axiom(name, universeParameters, type);
    decls.push_back(d);
}

void Parser::parse_definition() {
    auto name = parse_name_idx(false);
    if (error) return;
    auto type = parse_expr_idx();
    if (error) return;
    auto value = parse_expr_idx();
    if (error) return;
    auto hint = parse_hint();
    if (error) return;
    auto universeParameters = parse_name_star();
    if (error) return;
    
    lean::declaration d = lean::mk_definition(name, universeParameters, type, value, hint);
    decls.push_back(d);
}

void Parser::parse_theorem() {
    auto name = parse_name_idx(false);
    if (error) return;
    auto type = parse_expr_idx();
    if (error) return;
    auto value = parse_expr_idx();
    if (error) return;
    auto universeParameters = parse_name_star();
    if (error) return;
    
    lean::declaration d = lean::mk_theorem(name, universeParameters, type, value);
    decls.push_back(d);
}

void Parser::parse_opaque() {
    auto name = parse_name_idx(false);
    if (error) return;
    auto type = parse_expr_idx();
    if (error) return;
    auto value = parse_expr_idx();
    if (error) return;
    auto universeParameters = parse_name_star();
    if (error) return;
    
    lean::declaration d = lean::mk_opaque(name, universeParameters, type, value, false);
    decls.push_back(d);
}

void Parser::parse_inductive() {
    auto name = parse_name_idx(false);
    if (error) return;
    auto type = parse_expr_idx();
    if (error) return;
    auto numConstructors = parse_u64();
    if (error) return;
    auto constructorNames = parse_name_vec_amount(numConstructors);
    if (error) return;
    
    std::vector<lean::constructor> constructorsVec;
    
    for (uint64_t i = 0; i < numConstructors; ++i) {
        auto it = constructors.find(constructorNames[i]);
        if (it == constructors.end()) {
            dbgf("Referenced constructor that does not exist\n");
            error = true;
            return;
        } else {
            constructorsVec.push_back(it->second);
        }
    }

    lean::constructors constructorsList = lean::list_ref<lean::constructor>(constructorsVec.begin(), constructorsVec.end());
    
    lean::inductive_type ind = lean::inductive_type(name, type, constructorsList);
    inductives.insert({ name, ind });
}

void Parser::parse_inductive_family() {
    auto numParams = parse_u64();
    if (error) return;
    auto numInductives = parse_u64();
    if (error) return;
    auto inductiveNames = parse_name_vec_amount(numInductives);
    if (error) return;
    auto universeParameters = parse_name_star();
    if (error) return;
    
    std::vector<lean::inductive_type> inductivesVec;

    for (uint64_t i = 0; i < numInductives; ++i) {
        auto it = inductives.find(inductiveNames[i]);
        if (it == inductives.end()) {
            dbgf("Referenced inductive that does not exist");
            error = true;
            return;
        } else {
            inductivesVec.push_back(it->second);
        }
    }

    lean::inductive_types inductivesList = lean::list_ref<lean::inductive_type>(inductivesVec.begin(), inductivesVec.end());
    lean::declaration d = lean::mk_inductive_decl(universeParameters, lean::nat(numParams), inductivesList, false);
    decls.push_back(d);
}

void Parser::parse_constructor() {
    auto name = parse_name_idx(false);
    if (error) return;
    auto type = parse_expr_idx();
    if (error) return;
    auto parentInductive = parse_name_idx(false); // Unused
    if (error) return;
    auto constructorIndex = parse_u64(); // Unused
    if (error) return;
    auto numParams = parse_u64(); // Unused
    if (error) return;
    auto numFields = parse_u64(); // Unused
    if (error) return;
    auto universeParameters = parse_name_star(); // Unused
    if (error) return;

    lean::constructor c = lean::pair_ref(name, type);
    constructors.insert({ name, c });
}

bool Parser::add_false() {
    if (exprs.empty()) {
        return false;
    }
    lean::expr possibleProof = exprs.back();

    lean::name falseName(names[0], lean::string_ref("False"));
    
    lean::expr falseType = lean::mk_const(falseName);
    
    lean::name fooName(names[0], lean::string_ref("foo"));

    lean::declaration d = lean::mk_theorem(fooName, lean::names(), falseType, possibleProof);
    decls.push_back(d);
    return true;
}

lean::declaration mk_quot() {
    return lean::declaration(lean_box(4));
}

void Parser::parse_quotient() {
    lean::declaration d = mk_quot();
    decls.push_back(d);
}

sz::string_view name_prefix = "#NAME"_sz;
sz::string_view level_prefix = "#LVL"_sz;
sz::string_view expression_prefix = "#EXPR"_sz;
sz::string_view axiom_prefix = "#AX"_sz;
sz::string_view definition_prefix = "#DEF"_sz;
sz::string_view theorem_prefix = "#THM"_sz;
sz::string_view opaque_prefix = "#OPAQ"_sz;
sz::string_view inductive_prefix = "#IND"_sz;
sz::string_view inductive_family_prefix = "#INDF"_sz;
sz::string_view constructor_prefix = "#CTOR"_sz;
sz::string_view quotient_prefix = "#QUOT"_sz;

void Parser::parse_line() {
    auto command = parse_string();
    if (error) return;
    if (command == name_prefix) {
        parse_name();        
    } else if (command == level_prefix) {
        parse_level();
    } else if (command == expression_prefix) {
        parse_expression();
    } else if (command == axiom_prefix) {
        parse_axiom();
    } else if (command == definition_prefix) {
        parse_definition();
    } else if (command == theorem_prefix) {
        parse_theorem();
    } else if (command == opaque_prefix) {
        parse_opaque();
    } else if (command == inductive_prefix) {
        parse_inductive();
    } else if (command == inductive_family_prefix) {
        parse_inductive_family();
    } else if (command == constructor_prefix) {
        parse_constructor();
    } else if (command == quotient_prefix) {
        parse_quotient();
    } else {
        dbgf("Not a known command type\n");
        error = true;
    }
    
    if (error) {
        return;
    }
    
    if (!line.empty()) {
        dbgf("Not fully consumed\n");
        error = true;
    }
}

sz::string_view expected_version = "markus-0.0.5\n"_sz;

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

bool Parser::is_error() const {
    return error;
}

Parser::Parser(bool preludeMode) :
        line(),
        full_line(),
        error(false),
        prelude(preludeMode),
        exprs(),
        names(),
        levels(),
        decls(),
        constructors(),
        inductives() {
    levels.push_back(lean::mk_level_zero());
    names.push_back(lean::name::anonymous());
}

const std::vector<lean::declaration> & Parser::get_decls() const {
    return decls;
}