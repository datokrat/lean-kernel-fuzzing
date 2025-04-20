/*
Copyright (c) 2025 Markus Himmel. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Markus Himmel
*/
#include "parser.h"
#include "kernel/environment.h"
#include "kernel/init_module.h"

#include <iostream>
#include <fstream>
#include <sstream>

extern "C" void lean_initialize_runtime_module();
extern "C" void lean_initialize();
extern "C" void lean_io_mark_end_initialization();
// extern "C" lean_object * initialize_Lean_Environment(uint8_t builtin, lean_object *);

extern "C" lean_object* lean_mk_empty_environment(uint32_t trust_level, lean_object* /* world */);
extern "C" lean_object* lean_elab_environment_to_kernel_env(lean_object* x_1);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Missing file name" << std::endl;
        return 0;
    }
    
    std::cout << "a" << std::endl;

    lean_initialize_runtime_module();
    lean_object * res;
    // use same default as for Lean executables
    uint8_t builtin = 1;
    lean_initialize();
    lean_io_mark_end_initialization();
    std::cout << "b " << argv[1] << std::endl;
    
    std::ifstream stream(argv[1]);
    std::cout << "b1" << std::endl;
    std::stringstream buffer;
    std::cout << "b2" << std::endl;
    buffer << stream.rdbuf();
    std::cout << "b3" << std::endl;
    
    Parser p(true);
    std::cout << "c" << std::endl;
    p.handle_file(buffer.str());
    std::cout << "d" << std::endl;

    if (p.is_error()) {
        std::cout << "Prelude parsing error, not running environment." << std::endl;
        return 1;
    }
    
    lean_object *env = lean_elab_environment_to_kernel_env(lean_mk_empty_environment(0, lean_io_mk_world()));
    lean::environment environment(env);
    
    for (const lean::declaration & d : p.get_decls()) {
        environment = environment.add(d);
    }

    return 0;
}