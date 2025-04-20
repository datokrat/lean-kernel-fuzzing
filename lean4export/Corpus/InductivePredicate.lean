prelude
import Corpus.ExtendedPrelude

inductive PowerOfTwo : Nat → Prop where
  | one : PowerOfTwo 1
  | double : PowerOfTwo n → PowerOfTwo (Nat.mul 2 n)

theorem t : PowerOfTwo n → PowerOfTwo n
  | .one => .one
  | .double _ _ => .double
