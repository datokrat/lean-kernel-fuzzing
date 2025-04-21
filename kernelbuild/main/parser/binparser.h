/*
Copyright (c) 2025 Markus Himmel. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Markus Himmel
*/
#pragma once
#include "kernel/expr.h"
#include "kernel/level.h"
#include "kernel/declaration.h"
#include "util/name_hash_map.h"
#include <vector>

class BinParser {
public:
    BinParser(const std::vector<std::string> & strings);

    void handle_data(const uint8_t *buf, uint64_t len);

    const std::vector<lean::declaration> & get_decls() const;

    // Returns false if it was not added
    bool add_false();

private:
    /* Basic parsing functions */
    std::uint8_t parse_u8();
    std::uint16_t parse_u16();
    std::uint32_t parse_u32();
    std::string parse_string();

    /* Parsing of lean-specific objects */
    lean::level parse_level_idx();
    lean::name parse_name_idx(bool allowAnon);
    lean::expr parse_expr_idx();
    template<typename T> std::vector<T> parse_objs(const std::vector<T> & objs);
    lean::levels parse_levels();
    lean::names parse_names();
    lean::mpz parse_natlit();
    std::string parse_strlit();
    lean::reducibility_hints parse_hint();
    
    /* Garbage from the collection */
    lean::name any_name();
    lean::expr any_expr();
    lean::constructor any_constructor();
    lean::constructors any_constructors();
    lean::inductive_type any_inductive();

    /* Parsing of specific constructions */
    void parse_name();
    void parse_level();
    void parse_expression();
    void parse_definition();
    void parse_theorem();
    void parse_inductive();
    void parse_inductive_family();
    void parse_constructor();

    void parse_line();

    /* Data members */

    const std::uint8_t * cur;
    std::uint64_t remaining_len;
    
    std::vector<std::string> strings;

    std::vector<lean::expr> exprs;
    std::vector<lean::name> names;
    std::vector<lean::level> levels;

    std::vector<lean::declaration> decls;

    lean::name_hash_map<lean::constructor> constructors;
    lean::name_hash_map<lean::inductive_type> inductives;
    
    
};