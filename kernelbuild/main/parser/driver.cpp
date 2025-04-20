/*
Copyright (c) 2025 Markus Himmel. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Markus Himmel
*/
#include "parser.h"
#include "kernel/environment.h"

#include <iostream>
#include <fstream>
#include <sstream>

extern void lean_initialize_runtime_module();
extern void lean_initialize();
extern void lean_io_mark_end_initialization();
extern lean_object * initialize_Lean_Environment(uint8_t builtin, lean_object *);

extern lean_object* lean_mk_empty_environment(uint32_t trust_level, lean_object* /* world */);
extern lean_object* lean_elab_environment_to_kernel_env(lean_object* x_1);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Missing file name" << std::endl;
        return 0;
    }

    lean_initialize_runtime_module();
    lean_object * res;
    // use same default as for Lean executables
    uint8_t builtin = 1;
    res = initialize_Lean_Environment(builtin, lean_io_mk_world());
    if (lean_io_result_is_ok(res)) {
        lean_dec_ref(res);
    } else {
        lean_io_result_show_error(res);
        lean_dec(res);
        return 1;  // do not access Lean declarations if initialization failed
    }
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
    
    lean_object *env = lean_elab_environment_to_kernel_env(lean_mk_empty_environment(0, lean_io_mk_world()));
    lean::environment environment(env);
    
    for (const lean::declaration & d : p.get_decls()) {
        environment = environment.add(d);
    }

    return 0;
}