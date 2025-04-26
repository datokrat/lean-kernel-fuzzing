/*
Copyright (c) 2025 Markus Himmel. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Markus Himmel
*/
#include "parser.h"
#include "binparser.h"
#include "kernel/environment.h"
#include "kernel/kernel_exception.h"
#include "kernel/init_module.h"
#include "library/elab_environment.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <filesystem>

extern "C" void lean_initialize_runtime_module();
extern "C" void lean_initialize();
extern "C" void lean_io_mark_end_initialization();
// extern "C" lean_object * initialize_Lean_Environment(uint8_t builtin, lean_object *);

extern "C" lean_object* lean_mk_empty_environment(uint32_t trust_level, lean_object* /* world */);
extern "C" lean_object* lean_elab_environment_to_kernel_env(lean_object* x_1);

#ifdef __AFL_FUZZ_TESTCASE_LEN

__AFL_FUZZ_INIT();

#endif

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

int main(int argc, char* argv[]) {
    lean_initialize_runtime_module();
    lean_initialize();
    lean_io_mark_end_initialization();

    std::vector<std::string> strings = read_strings();
    
    std::ifstream stream("prelude.elean");
    std::stringstream buffer;
    buffer << stream.rdbuf();
    
    Parser p(true);
    p.handle_file(buffer.str());

    if (p.is_error()) {
        return 1;
    }
    
    lean_object *io_ress = lean_mk_empty_environment(0, lean_io_mk_world());
    lean_inc(io_ress);
    lean_object *eenv = lean_io_result_get_value(io_ress);

    lean::elab_environment elab_env(eenv, true);
    
    try {
        for (const lean::declaration & d : p.get_decls()) {
          elab_env = elab_env.add(d);
        }
    } catch (const lean::unknown_constant_exception &ex) {
        std::cout << "Unkown constant: " << ex.get_name() << std::endl;
    }
    
#ifdef __AFL_FUZZ_TESTCASE_LEN

    #ifdef __AFL_HAVE_MANUAL_CONTROL
	__AFL_INIT();
    #endif

    unsigned char *buf = __AFL_FUZZ_TESTCASE_BUF;

    while (__AFL_LOOP(10000)) {
        unsigned long len = __AFL_FUZZ_TESTCASE_LEN;

        sz::string_view data = { (const char *)buf, len };
        
        Parser p2(strings);
        p2.handle_data((const uint8_t *)buf, len);
        
        lean::elab_environment loop_env(elab_env);

        bool kernel_error = false;
        bool added_false = p2.add_false();
        try {
            for (const lean::declaration & d : p2.get_decls()) {
                loop_env = loop_env.add(d);
            }
        } catch (...) {
            // Did not succeed
            kernel_error = true;
        }
        
        if (added_false && !kernel_error) {
            std::cout << "Have a proof of false?!" << std::endl;
            abort();
        }

    }
#else
 
    bool binary = true;

    if (binary) {
        std::vector<std::byte> data = readFileData(argv[1]);
    
        BinParser p2(strings);
        p2.handle_data((const uint8_t *)data.data(), data.size());
    
        std::cout << "Finished parsing." << std::endl;
        
        lean::elab_environment loop_env(elab_env);
        
        // p2.add_false();
        for (const lean::declaration & d : p2.get_decls()) {
            loop_env = loop_env.add(d);
        }
    } else {
        std::ifstream stream2(argv[1]);
        std::stringstream buffer2;
        buffer2 << stream2.rdbuf();
        
        Parser p2(false);
        p2.handle_file(buffer2.str());
    
        if (p2.is_error()) {
            std::cout << "Parse error" << std::endl;
            return 1;
        }
        
        std::cout << "Finished parsing." << std::endl;
        
        lean::elab_environment loop_env(elab_env);
    
        // p2.add_false();
        for (const lean::declaration & d : p2.get_decls()) {
            loop_env = loop_env.add(d);
        }
        
        std::cout << "Finished adding to env" << std::endl;
    }

#endif

    return 0;
}