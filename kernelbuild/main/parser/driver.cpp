/*
Copyright (c) 2025 Markus Himmel. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Markus Himmel
*/
#include "parser.h"
#include "kernel/environment.h"
#include "kernel/init_module.h"
#include "library/elab_environment.h"

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
    
    lean_initialize_runtime_module();
    lean_object * res;
    // use same default as for Lean executables
    uint8_t builtin = 1;
    lean_initialize();
    lean_init_task_manager();
    lean_io_mark_end_initialization();
    
    std::ifstream stream(argv[1]);
    std::stringstream buffer;
    buffer << stream.rdbuf();
    
    Parser p(true);
    p.handle_file(buffer.str());

    if (p.is_error()) {
        std::cout << "Prelude parsing error, not running environment." << std::endl;
        return 1;
    }
    
    std::cout << "Finished parsing the file, creating an empty environment" << std::endl;
    
    lean_object *io_ress = lean_mk_empty_environment(0, lean_io_mk_world());
    lean_inc(io_ress);
    lean_object *eenv = lean_io_result_get_value(io_ress);

    std::cout << "a" << std::endl;
    lean::elab_environment elab_env(eenv, true);
    std::cout << "b" << std::endl;
    // lean::environment environment = elab_env.to_kernel_env();
    // std::cout << "c" << std::endl;
    
    for (const lean::declaration & d : p.get_decls()) {
        std::printf("Trying to add a declaration, it is an an inductive: %lld\n", (uint64_t)d.is_inductive());
        elab_env = elab_env.add(d);
    }
    
    std::cout << "Successfully checked prelude" << std::endl;

    return 0;
}