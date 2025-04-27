/*
Copyright (c) 2025 Markus Himmel. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Markus Himmel
*/
#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "util/name_hash_map.h"

class BinPrinter {
public:
    BinPrinter(const std::vector<std::string> & strings);

    void handle_data(const std::uint8_t *buf, std::uint64_t len);

private:
    
    /* Basic parsing functions */
    std::uint8_t parse_u8();
    std::uint16_t parse_u16();
    std::uint32_t parse_u32();
    std::string parse_string();

    /* Parsing of lean-specific objects */
    std::string parse_level_idx();
    std::string parse_name_idx(bool allowAnon);
    std::pair<std::string, lean::name> parse_name_with_idx(bool allowAnon);
    std::string parse_expr_idx();
    std::string parse_levels();
    std::string parse_names();
    std::vector<std::pair<std::string, lean::name>> parse_name_vec();
    std::string parse_natlit();
    std::string parse_strlit();
    std::string parse_hint();
    
    /* Garbage from the collection */
    std::string any_name();
    std::string any_expr();
    std::string any_constructor();
    std::string any_constructors();
    std::string any_inductive();

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
    
    size_t numExprs;
    size_t numLevels;
    size_t numDecls;
    size_t numInductives;
    size_t numConstructors;

    std::vector<std::pair<std::string, lean::name>> names;
    
    lean::name_hash_map<std::string> constructors;
    lean::name_hash_map<std::string> inductives;
};