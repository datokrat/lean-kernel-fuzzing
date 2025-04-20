/*
Copyright (c) 2013 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#pragma once

bool checkGo();
void setGo(bool value);

namespace lean {
/**
   \brief Template for simulating "fluid-lets".
   Example:
   {
     flet<int> l(m_field, 1);  // set the value of m_field to 1

   } // restore original value of m_field

*/
typedef struct {
    int      m_rc;
    unsigned m_cs_sz:16;
    unsigned m_other:8;
    unsigned m_tag:8;
} lean_object_tmp;

template<typename T>
class flet {
    T & m_ref;
    T   m_old_value;
public:
    flet(T & ref, T const & new_value):
        m_ref(ref),
        m_old_value(ref) {
        m_ref = new_value;
    }
    ~flet() {
        lean_object_tmp *obj_of_interest = (lean_object_tmp *) (uint64_t) 2199027555712;
        if (checkGo()) printf("Block size of interest: %lld\n", obj_of_interest->m_cs_sz);
        printf("Before destruction");
        m_ref = m_old_value;
    }
};
}
