/*
Copyright (c) 2025 Markus Himmel. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Markus Himmel
*/
#include "binparser.h"

#define MARKUS_DEBUG

#ifdef MARKUS_DEBUG
    #define dbgf printf
#else
    #define dbgf(...)
#endif

std::uint8_t BinParser::parse_u8() {
    if (remaining_len > 0) {
        std::uint8_t result = *cur;
        ++cur;
        --remaining_len;
        return result;
    } else {
        std::cout << "Read over the end" << std::endl;
        return 0;
    }
}

std::uint16_t BinParser::parse_u16() {
    std::uint16_t high = parse_u8();
    std::uint16_t low = parse_u8();
    return (high << 8) | low;
}

std::uint32_t BinParser::parse_u32() {
    std::uint32_t high = parse_u16();
    std::uint32_t low = parse_u16();
    return (high << 16) | low;
}

std::string BinParser::parse_string() {
    std::uint16_t idx = parse_u16();
    std::string ans = strings[idx % strings.size()];
    // std::cout << idx << " -> " << ans << std::endl;
    return ans;
}

lean::level BinParser::parse_level_idx() {
    std::uint16_t idx = parse_u16();
    if (idx >= levels.size()) {
        std::cout << "bad level index" << std::endl;
    }
    return levels[idx % levels.size()];
}

lean::name BinParser::parse_name_idx(bool allowAnon) {
    std::uint16_t pidx = parse_u16();
    if (pidx >= names.size()) {
        std::cout << "bad name index" << std::endl;
    }
    std::uint64_t idx = pidx % names.size();
    if (idx == 0 && !allowAnon) {
        lean::name n(lean::name::anonymous(), lean::string_ref("foo42"));
        return n;
    }
    lean::name n = names[idx];
    return n;
}

lean::mpz BinParser::parse_natlit() {
    lean::mpz result(0);
    std::uint8_t len = parse_u8();
    for (std::uint8_t i = 0; i < len; ++i) {
        result *= 256;
        std::uint8_t byte = parse_u8();
        result += byte;
    }
    return result;
}

std::string BinParser::parse_strlit() {
    std::string result;
    std::uint8_t len = parse_u8();
    for (std::uint8_t i = 0; i < len; ++i) {
        std::uint8_t byte = parse_u8();
        char c = (char)byte;
        result.push_back(c);
    }
    return result;
}

lean::reducibility_hints BinParser::parse_hint() {
    auto hintType = parse_u8();
    switch (hintType % 3) {
        case 0: {
            return lean::reducibility_hints::mk_opaque();
        }
        case 1: {
            return lean::reducibility_hints::mk_abbreviation();
        }
        case 2: {
            std::uint32_t v = parse_u32();
            return lean::reducibility_hints::mk_regular(v);
        }
    }
}

lean::name BinParser::any_name() {
    if (names.size() > 1) {
        return names.back();
    } else {
        lean::name n(lean::name::anonymous(), lean::string_ref("foo42"));
        return n;
    }
}

lean::expr BinParser::any_expr() {
    if (exprs.size() > 0) {
        return exprs.back();
    } else {
        return lean::mk_sort(lean::mk_level_zero());
    }
}

lean::constructors BinParser::any_constructors() {
    return lean::list_ref<lean::constructor>({ any_constructor() });
}

lean::inductive_type BinParser::any_inductive() {
    return lean::inductive_type(any_name(), any_expr(), any_constructors());
}

lean::constructor BinParser::any_constructor() {
    return lean::pair_ref(any_name(), any_expr());
}

lean::expr BinParser::parse_expr_idx() {
    std::uint8_t idx = parse_u16();
    if (idx >= exprs.size()) {
        std::cout << "bad expr index" << std::endl;
    } 
    if (exprs.size() > 0) {
        return exprs[idx % exprs.size()];
    } else {
        // If no expression has been declared yet, return `Prop`.
        return lean::mk_sort(lean::mk_level_zero());
    }
}

template<typename T>
std::vector<T> BinParser::parse_objs(const std::vector<T> & objs) {
    std::vector<T> result;
    std::uint16_t amt = parse_u8();
    // std::cout << "Amount is " << amt << std::endl;
    for (std::uint16_t i = 0; i < amt; ++i) {
        std::uint16_t num = parse_u16();
        result.push_back(objs[num % objs.size()]);
    }
    return result;
}

lean::levels BinParser::parse_levels() {
    std::vector<lean::level> vec = parse_objs<lean::level>(levels);
    return lean::list_ref<lean::level>(vec.begin(), vec.end());
}

lean::names BinParser::parse_names() {
    std::vector<lean::name> vec = parse_objs<lean::name>(names);
    return lean::list_ref<lean::name>(vec.begin(), vec.end());
}

void BinParser::parse_name() {
    // dbgf("Name\n");
    std::uint8_t nameType = parse_u8();
    switch (nameType % 2) {
        case 0: {
            lean::name parent = parse_name_idx(true);
            std::string comp = parse_string();
            lean::name n(parent, lean::string_ref(comp));
            // std::cout << n << std::endl;
            names.push_back(n);
            break;
        }
        case 1: {
            lean::name parent = parse_name_idx(true);
            std::uint16_t comp = parse_u16();
            lean::name n(parent, comp);
            names.push_back(n);
            break;
        }
    }
}

void BinParser::parse_level() {
    dbgf("Level\n");
    std::uint8_t levelType = parse_u8();
    switch (levelType % 4) {
        case 0: {
            lean::level parent = parse_level_idx();
            lean::level l = lean::mk_succ(parent);
            levels.push_back(l);
            break;
        }
        case 1: {
            lean::level lhs = parse_level_idx();
            lean::level rhs = parse_level_idx();
            lean::level l = lean::mk_max(lhs, rhs);
            levels.push_back(l);
            break;
        }
        case 2: {
            lean::level lhs = parse_level_idx();
            lean::level rhs = parse_level_idx();
            lean::level l = lean::mk_imax(lhs, rhs);
            levels.push_back(l);
            break;
        }
        case 3: {
            lean::name n = parse_name_idx(false);
            lean::level l = lean::mk_univ_param(n);
            levels.push_back(l);
            break;
        }
    }
}

void BinParser::parse_expression() {
    std::uint8_t expressionType = parse_u8();
    switch (expressionType % 10) {
        case 0: { // Bound variable
            std::uint16_t deBruijnIndex = parse_u16();
            dbgf("Bound variable %d\n", deBruijnIndex);
            lean::expr e = lean::mk_bvar(lean::nat(deBruijnIndex));
            exprs.push_back(e);
            break;
        }
        case 1: { // Sort
            // dbgf("Sort\n");
            lean::level universe = parse_level_idx();
            lean::expr e = lean::mk_sort(universe);
            exprs.push_back(e);
            break;
        }
        case 2: { // Constant
            // dbgf("Constant\n");
            lean::name name = parse_name_idx(false);
            // std::cout << name << std::endl;
            lean::levels levels = parse_levels();
            lean::expr e = lean::mk_const(name, levels);
            exprs.push_back(e);
            break;
        }
        case 3: { // App
            dbgf("App\n");
            lean::expr lhs = parse_expr_idx();
            lean::expr rhs = parse_expr_idx();
            lean::expr e = lean::mk_app(lhs, rhs);
            exprs.push_back(e);
            break;
        }
        case 4: { // Lambda
            dbgf("Lambda\n");
            lean::name binderName = parse_name_idx(true);
            lean::expr binderType = parse_expr_idx();
            lean::expr body = parse_expr_idx();
            lean::expr e = lean::mk_lambda(binderName, binderType, body);
            exprs.push_back(e);
            break;
        }
        case 5: { // Pi
            dbgf("Pi\n");
            lean::name binderName = parse_name_idx(true);
            lean::expr binderType = parse_expr_idx();
            lean::expr body = parse_expr_idx();
            lean::expr e = lean::mk_pi(binderName, binderType, body);
            exprs.push_back(e);
            break;
        }
        case 6: { // Let
            // dbgf("Let\n");
            lean::name binderName = parse_name_idx(false);
            lean::expr binderType = parse_expr_idx();
            lean::expr boundValue = parse_expr_idx();
            lean::expr body = parse_expr_idx();
            lean::expr e = lean::mk_let(binderName, binderType, boundValue, body);
            exprs.push_back(e);
            break;
        }
        case 7: { // Projection
            // dbgf("Proj\n");
            lean::name typeName = parse_name_idx(false);
            std::uint16_t fieldIndex = parse_u16();
            lean::expr value = parse_expr_idx();
            lean::expr e = lean::mk_proj(typeName, fieldIndex, value);
            exprs.push_back(e);
            break;
        }
        case 8: { // Nat literal
            // dbgf("Natlit\n");
            lean::mpz lit = parse_natlit();
            lean::expr e = lean::mk_lit(lean::literal(lit));
            exprs.push_back(e);
            break;
        }
        case 9: { // String literal
            // dbgf("Strlit\n");
            std::string lit = parse_strlit();
            lean::expr e = lean::mk_lit(lean::literal(lit.c_str()));
            exprs.push_back(e);
            break;
        }
    }
}

void BinParser::parse_definition() {
    dbgf("Def\n");
    lean::name name = parse_name_idx(false);
    lean::expr type = parse_expr_idx();
    lean::expr value = parse_expr_idx();
    lean::reducibility_hints hint = parse_hint();
    lean::names universeParameters = parse_names();

    lean::declaration d = lean::mk_definition(name, universeParameters, type, value, hint);
    decls.push_back(d);
}

void BinParser::parse_theorem() {
    dbgf("Theorem\n");
    lean::name name = parse_name_idx(false);
    lean::expr type = parse_expr_idx();
    lean::expr value = parse_expr_idx();
    lean::names universeParameters = parse_names();

    lean::declaration d = lean::mk_theorem(name, universeParameters, type, value);
    decls.push_back(d);
}

void BinParser::parse_inductive() {
    dbgf("Inductive\n");
    lean::name name = parse_name_idx(false);
    lean::expr type = parse_expr_idx();
    std::vector<lean::name> constructorNames = parse_objs<lean::name>(names);
    
    std::vector<lean::constructor> constructorsVec;

    for (uint64_t i = 0; i < constructorNames.size(); ++i) {
        auto it = constructors.find(constructorNames[i]);
        if (it == constructors.end()) {
            std::cout << "Bad inductive" << std::endl;
            constructorsVec.push_back(any_constructor());
        } else {
            constructorsVec.push_back(it->second);
        }
    }
    
    lean::constructors constructorsList = lean::list_ref<lean::constructor>(constructorsVec.begin(), constructorsVec.end());
    
    std::cout << "Inductive is called " << name << std::endl;
    
    lean::inductive_type ind = lean::inductive_type(name, type, constructorsList);
    inductives.insert({ name, ind });
}

void BinParser::parse_inductive_family() {
    dbgf("Inductive family\n");
    std::uint16_t numParams = parse_u8();
    std::vector<lean::name> inductiveNames = parse_objs<lean::name>(names);
    lean::names universeParameters = parse_names();
    
    std::vector<lean::inductive_type> inductivesVec;
    
    // std::cout << "Inductive family with " << inductiveNames.size() << " members" << std::endl;

    for (uint64_t i = 0; i < inductiveNames.size(); ++i) {
        auto it = inductives.find(inductiveNames[i]);
        if (it == inductives.end()) {
            std::cout << "Bad inductive family" << std::endl;
            inductivesVec.push_back(any_inductive());
        } else {
            inductivesVec.push_back(it->second);
        }
    }

    lean::inductive_types inductivesList = lean::list_ref<lean::inductive_type>(inductivesVec.begin(), inductivesVec.end());
    lean::declaration d = lean::mk_inductive_decl(universeParameters, lean::nat(numParams), inductivesList, false);
    decls.push_back(d);
}

void BinParser::parse_constructor() {
    dbgf("Constructor\n");
    lean::name name = parse_name_idx(false);
    lean::expr type = parse_expr_idx();

    lean::constructor c = lean::pair_ref(name, type);
    constructors.insert({ name, c });
}

void BinParser::parse_line() {
    std::uint8_t declType = parse_u8();
    switch (declType % 8) {
        case 0: // Level
            parse_level();
            break;
        case 1: // Expr
            parse_expression();
            break;
        case 2: // Def
            parse_definition();
            break;
        case 3: // Theorem
            parse_theorem();
            break;
        case 4: // Inductive
            parse_inductive();
            break;
        case 5: // Inductive Family
            parse_inductive_family();
            break;
        case 6: // Constructor
            parse_constructor();
            break;
        case 7: // Name
            parse_name();
            break;
    }
}

void BinParser::handle_data(const std::uint8_t *buf, std::uint64_t len) {
    cur = buf;
    remaining_len = len;

    while (remaining_len > 0) {
        parse_line();
    }
}

BinParser::BinParser(const std::vector<std::string> & _strings) :
        cur(nullptr),
        remaining_len(0),
        strings(_strings),
        exprs(),
        names(),
        levels(),
        decls(),
        constructors(),
        inductives() {
    levels.push_back(lean::mk_level_zero());
    names.push_back(lean::name::anonymous());
}

const std::vector<lean::declaration> & BinParser::get_decls() const {
    return decls;
}

bool BinParser::add_false() {
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