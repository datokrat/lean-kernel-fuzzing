prelude
import Corpus.ExtendedPrelude

inductive UnitLike where
  | mk : UnitLike

-- type_checker.cpp:is_def_eq_unit_like
theorem t (a b : UnitLike) : Eq a b := rfl
