/*
Copyright (c) 2025 Markus Himmel. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Markus Himmel
*/
#pragma once
#include <stringzilla/stringzilla.hpp>
#include "kernel/expr.h"
#include <vector>

namespace sz = ashvardanian::stringzilla;

enum HintType {
    O, A, R
};

struct Hint {
    HintType type;
    std::uint64_t value;
};

class Parser {
public:
    Parser() : error(false), line(), full_line() { }
    
    void handle_file(sz::string_view file);

    bool is_error();

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
    std::vector<std::uint64_t> parse_u64_star();
    std::vector<std::uint64_t> parse_u64_amount(std::uint64_t n);
    bool parse_bool();
    Hint parse_hint();

    /* Parsing of specific lines */
    void parse_name();
    void parse_level();
    void parse_expression();
    void parse_recrule();
    void parse_axiom();
    void parse_quotient();
    void parse_definition();
    void parse_theorem();
    void parse_opaque();
    void parse_inductive();
    void parse_constructor();
    void parse_recursor();
    
    /* Parsing the current line */
    void parse_line();

    /* Data members */
    
    // Current line
    sz::string_view line;
    sz::string_view full_line;
    
    // Was there ever an error
    bool error;
    
    std::vector<lean::expr> exprs;
    // TODO: State: names, levels, expressions
    // TODO: environment??
};