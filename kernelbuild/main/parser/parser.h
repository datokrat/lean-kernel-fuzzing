/*
Copyright (c) 2025 Markus Himmel. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Markus Himmel
*/
#pragma once
#include <stringzilla/stringzilla.hpp>
#include "kernel/expr.h"
#include "kernel/level.h"
#include "kernel/declaration.h"
#include "util/name_hash_map.h"
#include <vector>

namespace sz = ashvardanian::stringzilla;

class Parser {
public:
    Parser(bool preludeMode);
    
    void handle_file(sz::string_view file);

    bool is_error() const;

    const std::vector<lean::declaration> & get_decls() const;

    // Returns false if it was not added.
    bool add_false();

private:
    /* Basic parsing functions */
    sz::string_view try_parse_string();
    sz::string_view parse_string();
    char convert_hexchar(sz::string_view val);
    sz::string parse_hexstring();
    template<typename T> T convert_numeric(sz::string_view val);
    template<typename T> T parse_numeric();
    template<typename T> std::vector<T> parse_numeric_star();
    template<typename T> std::vector<T> parse_numeric_amount(std::uint64_t n);
    std::uint64_t parse_u64();
    unsigned int parse_uint();
    std::vector<std::uint64_t> parse_u64_star();
    std::vector<std::uint64_t> parse_u64_amount(std::uint64_t n);
    bool parse_bool();
    lean::reducibility_hints parse_hint();

    /* Parsing of lean-specific objects */
    lean::name parse_name_idx(bool allowAnon);
    lean::level parse_level_idx();
    lean::expr parse_expr_idx();
    template<typename T> std::vector<T> parse_obj_star(const std::vector<T> & objs);
    template<typename T> std::vector<T> parse_obj_amount(std::uint64_t n, const std::vector<T> & objs);
    lean::levels parse_level_star();
    lean::levels parse_level_amount(std::uint64_t n);
    lean::names parse_name_star();
    lean::names parse_name_amount(std::uint64_t n);
    std::vector<lean::name> parse_name_vec_amount(std::uint64_t n);
    
    /* Parsing of specific lines */
    void parse_name();
    void parse_level();
    void parse_expression();
    void parse_axiom();
    void parse_definition();
    void parse_theorem();
    void parse_opaque();
    void parse_inductive();
    void parse_inductive_family();
    void parse_constructor();
    void parse_quotient();
    
    /* Parsing the current line */
    void parse_line();

    /* Data members */
    
    // Just the suffix of the line that is still to be parsed
    sz::string_view line;
    // The entire line that is currently being parsed, for printing out during debugging
    sz::string_view full_line;
    
    // Was there ever an error
    bool error;
    // Are we in "prelude mode", where axioms are accepted?
    bool prelude;
    
    std::vector<lean::expr> exprs;
    std::vector<lean::name> names;
    std::vector<lean::level> levels;

    std::vector<lean::declaration> decls;
    lean::name_hash_map<lean::constructor> constructors;
    lean::name_hash_map<lean::inductive_type> inductives;
};