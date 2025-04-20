prelude
import Corpus.ExtendedPrelude

theorem lit_eq_lit : Eq "Hello" "Hello" := rfl

-- attempt to trigger type_checker.cpp:try_string_lit_expansion
theorem lit_eq_mk : Eq "Hi" (String.mk (List.cons 'H' (List.cons 'i' List.nil))) := rfl
