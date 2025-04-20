prelude
import Corpus.ExtendedPrelude

-- see type_checker.cpp:is_def_eq_succ
theorem  nat_succ_eq_succ (n : Nat) : Eq (Nat.succ n) (Nat.succ n) := rfl
