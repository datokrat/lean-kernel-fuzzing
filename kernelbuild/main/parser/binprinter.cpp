/*
Copyright (c) 2025 Markus Himmel. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Markus Himmel
*/
#include "binprinter.h"
#include <sstream>
#include <iostream>
#include <filesystem>
#include <fstream>


// #define MARKUS_DEBUG

#ifdef MARKUS_DEBUG
    #define dbgf printf
#else
    #define dbgf(...)
#endif

std::uint8_t BinPrinter::parse_u8() {
    if (remaining_len > 0) {
        std::uint8_t result = *cur;
        ++cur;
        --remaining_len;
        return result;
    } else {
        dbgf("Read over the end\n");
        return 0;
    }
}

std::uint16_t BinPrinter::parse_u16() {
    std::uint16_t high = parse_u8();
    std::uint16_t low = parse_u8();
    return (high << 8) | low;
}

std::uint32_t BinPrinter::parse_u32() {
    std::uint32_t high = parse_u16();
    std::uint32_t low = parse_u16();
    return (high << 16) | low;
}

std::string BinPrinter::parse_string() {
    std::uint16_t idx = parse_u16();
    if (idx > strings.size()) {
        dbgf("bad string index\n");
    }
    std::string ans = strings[idx % strings.size()];
    return ans;
}

std::string BinPrinter::parse_level_idx() {
    std::uint16_t idx = parse_u16();
    if (idx >= numLevels) {
        dbgf("bad level index\n");
    }
    std::stringstream out;
    out << "level_" << (idx % numLevels);
    return out.str();
}

std::string BinPrinter::parse_name_idx(bool allowAnon) {
    std::uint16_t pidx = parse_u16();
    if (pidx >= names.size()) {
        dbgf("bad name index\n");
    }
    std::uint64_t idx = pidx % names.size();
    if (idx == 0 && !allowAnon) {
        dbgf("bad anonymous name");
        return "`foo42";
    }
    std::stringstream out;
    out << "name_" << idx;
    return out.str();
}

std::pair<std::string, lean::name> BinPrinter::parse_name_with_idx(bool allowAnon) {
    std::uint16_t pidx = parse_u16();
    std::uint64_t idx = pidx % names.size();
    if (idx == 0 && !allowAnon) {
        lean::name n(lean::name::anonymous(), lean::string_ref("foo42"));
        return { "`foo42", n };
    }
    return names[idx];
}

std::string BinPrinter::parse_levels() {
    std::uint8_t amount = parse_u8();
    std::stringstream out;
    out << "[";
    for (uint8_t i; i < amount; ++i) {
        out << parse_level_idx();
        if (i + 1 < amount) {
            out << ", ";
        }
    }
    out << "]";
    return out.str();
}

std::string BinPrinter::parse_names() {
    std::uint8_t amount = parse_u8();
    std::stringstream out;
    out << "[";
    for (uint8_t i; i < amount; ++i) {
        out << parse_name_idx(true);
        if (i + 1 < amount) {
            out << ", ";
        }
    }
    out << "]";
    return out.str();
}

std::vector<std::pair<std::string, lean::name>> BinPrinter::parse_name_vec() {
    std::vector<std::pair<std::string, lean::name>> result;
    std::uint8_t amount = parse_u8();
    for (uint8_t i = 0; i < amount; ++i) {
        std::uint16_t idx = parse_u16() % names.size();
        result.emplace_back(names[idx]);
    }
    return result;
}

std::string BinPrinter::parse_expr_idx() {
    std::uint16_t idx = parse_u16();
    if (idx >= numExprs) {
        dbgf("bad expr index\n");
    } 
    if (numExprs > 0) {
        std::stringstream out;
        out << "expr_" << (idx % numExprs);
        return out.str();
    } else {
        // If no expression has been declared yet, return `Prop`.
        return "(.sort .zero)";
    }
}

std::string BinPrinter::parse_natlit() {
    std::stringstream result;
    result << "(decodeNatLit [";
    std::uint8_t len = parse_u8();
    for (std::uint8_t i = 0; i < len; ++i) {
        std::uint8_t byte = parse_u8();
        result << byte;
        if (i + 1 < len) {
            result << ", ";
        }
    }
    result << "])";
    return result.str();
}

std::string BinPrinter::parse_strlit() {
    std::string result = "\"";
    std::uint8_t len = parse_u8();
    for (std::uint8_t i = 0; i < len; ++i) {
        std::uint8_t byte = parse_u8();
        char c = (char)byte;
        result.push_back(c);
    }
    return result + "\"";
}

std::string BinPrinter::parse_hint() {
    auto hintType = parse_u8();
    switch (hintType % 3) {
        case 0: {
            return ".opaque";
        }
        case 1: {
            return ".abbrev";
        }
        case 2: {
            std::uint32_t v = parse_u32();
            std::stringstream out;
            out << "(.regular " << v << ")";
            return out.str();
        }
    }
}

std::string BinPrinter::any_name() {
    if (names.size() > 1) {
        std::stringstream out;
        out << "name_" << (names.size() - 1);
        return out.str();
    } else {
        return "`foo42";
    }
}

std::string BinPrinter::any_expr() {
    if (numExprs > 0) {
        std::stringstream out;
        out << "expr_" << (numExprs - 1);
        return out.str();
    } else {
        return "(.sort .zero)";
    }
}

std::string BinPrinter::any_constructors() {
    return "[" + any_constructor() + "]";
}

std::string BinPrinter::any_inductive() {
    return "{ name := " + any_name() + ", type := " + any_expr() + " ctors := " + any_constructors() + " }";
}

std::string BinPrinter::any_constructor() {
    return "{ name := " + any_name() + ", type := " + any_expr() + " }";
}

void BinPrinter::parse_name() {
    std::uint8_t nameType = parse_u8();
    size_t myIndex = names.size();
    switch (nameType % 2) {
        case 0: {
            std::string parent = parse_name_idx(true);
            std::string comp = parse_string();
            std::cout << "def name_" << myIndex << " : Lean.Name := "
                << ".str " << parent << " \"" << comp << "\"" << std::endl;
            lean::name n(parent, lean::string_ref(comp));
            std::stringstream out;
            out << "name_" << myIndex;
            names.push_back({ out.str(), n });
            break;
        }
        case 1: {
            std::string parent = parse_name_idx(true);
            std::uint16_t comp = parse_u16();
            std::cout << "def name_" << myIndex << " : Lean.Name := "
                << ".num " << parent << " " << comp << std::endl;
            lean::name n(parent, comp);
            std::stringstream out;
            out << "name_" << myIndex;
            names.push_back({ out.str(), n });
            break;
        }
    }
}

void BinPrinter::parse_level() {
    std::uint8_t levelType = parse_u8();
    size_t myIndex = numLevels;
    switch (levelType % 4) {
        case 0: {
            std::string parent = parse_level_idx();
            std::cout << "def level_" << myIndex << " : Lean.Level := "
                << ".succ " << parent << std::endl; 
            ++numLevels;
            break;
        }
        case 1: {
            std::string lhs = parse_level_idx();
            std::string rhs = parse_level_idx();
            std::cout << "def level_" << myIndex << " : Lean.Level := "
                << ".max " << lhs << " " << rhs << std::endl;
            ++numLevels;
            break;
        }
        case 2: {
            std::string lhs = parse_level_idx();
            std::string rhs = parse_level_idx();
            std::cout << "def level_" << myIndex << " : Lean.Level := "
                << ".imax " << lhs << " " << rhs << std::endl;
            ++numLevels;
            break;
        }
        case 3: {
            std::string n = parse_name_idx(false);
            std::cout << "def level_" << myIndex << " : Lean.Level := "
                << ".param " << n << std::endl;
            ++numLevels;
        }
    }
}

void BinPrinter::parse_expression() {
    std::uint8_t expressionType = parse_u8();
    size_t myIndex = numExprs;
    std::cout << "def expr_" << myIndex << " : Lean.Expr := ";
    switch (expressionType % 10) {
        case 0: { // Bound variable
            std::uint64_t deBruijnIndex = parse_u16();
            dbgf("Bound variable %d\n", deBruijnIndex);
            std::cout << ".bvar " << deBruijnIndex << std::endl;
            break;
        }
        case 1: { // Sort
            std::string universe = parse_level_idx();
            std::cout << ".sort " << universe << std::endl;
            break;
        }
        case 2: { // Constant
            std::string name = parse_name_idx(false);
            std::string levels = parse_levels();
            std::cout << ".const " << name << " " << levels << std::endl;
            break;
        }
        case 3: { // App
            std::string lhs = parse_expr_idx();
            std::string rhs = parse_expr_idx();
            std::cout << ".app " << lhs << " " << rhs << std::endl;
            break;
        }
        case 4: { // Lambda
            std::string binderName = parse_name_idx(true);
            std::string binderType = parse_expr_idx();
            std::string body = parse_expr_idx();
            std::cout << ".lam " << binderName << " " << binderType << " " << body << " .default" << std::endl;
            break;
        }
        case 5: { // Pi
            std::string binderName = parse_name_idx(true);
            std::string binderType = parse_expr_idx();
            std::string body = parse_expr_idx();
            std::cout << ".forallE " << binderName << " " << binderType << " " << body << " .default" << std::endl;
            break;
        }
        case 6: { // Let
            std::string binderName = parse_name_idx(false);
            std::string binderType = parse_expr_idx();
            std::string boundValue = parse_expr_idx();
            std::string body = parse_expr_idx();
            std::cout << ".letE " << binderName << " " << binderType << " " << boundValue << " " << body
                << " false" << std::endl;
            break;
        }
        case 7: { // Projection
            std::string typeName = parse_name_idx(false);
            std::uint16_t fieldIndex = parse_u16();
            std::string value = parse_expr_idx();
            std::cout << ".proj " << typeName << " " << fieldIndex << " " << value << std::endl;
            break;
        }
        case 8: { // Nat literal
            std::string lit = parse_natlit();
            std::cout << ".lit (.natVal " << lit << ")" << std::endl;
            break;
        }
        case 9: { // String literal
            // dbgf("Strlit\n");
            std::string lit = parse_strlit();
            std::cout << ".lit (.strVal " << lit << ")" << std::endl;
            break;
        }
    }
    ++numExprs;
}

void BinPrinter::parse_definition() {
    std::string name = parse_name_idx(false);
    std::string type = parse_expr_idx();
    std::string value = parse_expr_idx();
    std::string hint = parse_hint();
    std::string universeParameters = parse_names();
    
    size_t myIndex = numDecls;
    std::cout << "def decl_" << myIndex << " : Lean.Declaration :=\n  .defnDecl {\n"
        << "    name := " << name << "\n"
        << "    levelParams := " << universeParameters << "\n"
        << "    type := " << type << "\n"
        << "    value := " << value << "\n"
        << "    hints := " << hint << "\n"
        << "    safety := .safe" << "\n"
        << "    all := [" << name << "]\n"
        << "  }" << std::endl;
    ++numDecls;
}


void BinPrinter::parse_theorem() {
    std::string name = parse_name_idx(false);
    std::string type = parse_expr_idx();
    std::string value = parse_expr_idx();
    std::string universeParameters = parse_names();
    
    size_t myIndex = numDecls;
    std::cout << "def decl_" << myIndex << " : Lean.Declaration :=\n  .thmDecl {\n"
        << "    name := " << name << "\n"
        << "    levelParams := " << universeParameters << "\n"
        << "    type := " << type << "\n"
        << "    value := " << value << "\n"
        << "  }" << std::endl;
    ++numDecls;
}

void BinPrinter::parse_inductive() {
    auto [strName, leanName] = parse_name_with_idx(false);
    std::string type = parse_expr_idx();
    std::vector<std::pair<std::string, lean::name>> constructorNames = parse_name_vec();

    std::stringstream constructorsList;
    constructorsList << "[";
    for (uint64_t i = 0; i < constructorNames.size(); ++i) {
        auto it = constructors.find(constructorNames[i].second);
        if (it == constructors.end()) {
            constructorsList << any_constructor();
        } else {
            constructorsList << it->second;
        }
        if (i + 1 < constructorNames.size()) {
            constructorsList << ", ";
        }
    }
    constructorsList << "]";

    size_t myIndex = numInductives;
    std::cout << "def inductive_" << myIndex << " : Lean.InductiveType where\n"
        << "  name := " << strName << "\n"
        << "  type := " << type << "\n"
        << "  ctors := " << constructorsList.str() << std::endl;
    std::stringstream out;
    out << "inductive_" << myIndex;
    inductives.insert({ leanName, out.str() });
    ++numInductives;
}

void BinPrinter::parse_inductive_family() {
    std::uint16_t numParams = parse_u8();
    std::vector<std::pair<std::string, lean::name>> inductiveNames = parse_name_vec();
    std::string universeParameters = parse_names();
    // std::cout << "Inductive family: " << numParams << " params, " << inductiveNames.size()
    //     << " inductives." << std::endl;

    std::stringstream inductivesList;
    inductivesList << "[";
    for (uint64_t i = 0; i < inductiveNames.size(); ++i) {
        auto it = inductives.find(inductiveNames[i].second);
        if (it == inductives.end()) {
            inductivesList << any_inductive();
        } else {
            inductivesList << it->second;
        }
        if (i + 1 < inductiveNames.size()) {
            inductivesList << ", ";
        }
    }
    inductivesList << "]";

    size_t myIndex = numDecls;
    std::cout << "def decl_" << myIndex << " : Lean.Declaration :=\n  .inductDecl "
        << universeParameters << " " << numParams << " " << inductivesList.str() << " false" << std::endl;
    ++numDecls;
}

void BinPrinter::parse_constructor() {
    auto [strName, leanName] = parse_name_with_idx(false);
    std::string type = parse_expr_idx();
    
    size_t myIndex = numConstructors;
    std::cout << "def constructor_" << myIndex << " : Lean.Constructor where\n"
        << "  name := " << strName << "\n"
        << "  type := " << type << std::endl;
    std::stringstream out;
    out << "constructor_" << myIndex;
    constructors.insert({ leanName, out.str() });
    ++numConstructors;
}

void BinPrinter::parse_line() {
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

void BinPrinter::handle_data(const std::uint8_t *buf, std::uint64_t len) {
    cur = buf;
    remaining_len = len;

    while (remaining_len > 0) {
        parse_line();
    }
}

std::vector<std::string> read_strings() {
    std::ifstream stream("strings");
    std::stringstream buffer;
    buffer << stream.rdbuf();
    std::vector<std::string> result;
    std::string s;
    while (std::getline(buffer, s, '\n')) {
        result.push_back(s);
    }
    return result;
}

// https://www.coniferproductions.com/posts/2022/10/25/reading-binary-files-cpp/
std::vector<std::byte> readFileData(const std::string& name) {
    std::filesystem::path inputFilePath{name};
    auto length = std::filesystem::file_size(inputFilePath);
    if (length == 0) {
        return {};  // empty vector
    }
    std::vector<std::byte> buffer(length);
    std::ifstream inputFile(name, std::ios_base::binary);
    inputFile.read(reinterpret_cast<char*>(buffer.data()), length);
    inputFile.close();
    return buffer;
}

BinPrinter::BinPrinter(const std::vector<std::string> & _strings) :
        cur(nullptr),
        remaining_len(0),
        strings(_strings),
        numExprs(0),
        numLevels(1),
        numDecls(0),
        numInductives(0),
        numConstructors(0),
        names(),
        constructors(),
        inductives() {
    names.push_back({ ".anonymous", lean::name::anonymous() });
    std::cout << "def name_0 : Lean.Name := .anonymous" << std::endl;
    std::cout << "def level_0 : Lean.Level := .zero" << std::endl;
}

extern "C" void lean_initialize_runtime_module();
extern "C" void lean_initialize();
extern "C" void lean_io_mark_end_initialization();

int main(int argc, char* argv[]) {
    lean_initialize_runtime_module();
    lean_initialize();
    lean_io_mark_end_initialization();

    std::vector<std::string> strings = read_strings();
    std::vector<std::byte> data = readFileData(argv[1]);
    
    BinPrinter p(strings);
    p.handle_data((const uint8_t *)data.data(), data.size());
}