prelude
import Corpus.ExtendedPrelude

inductive NX (α : Type u) where
  | nil : NX α
  | cons : α → NX α → NX α

inductive NY (α : Type u) where
  | leaf : α → NY α
  | branch : NX (NY α) → NY α

mutual

def sizeNX {α : Type u} : NX (NY α) → Nat
  | .nil => 0
  | .cons head tail => Nat.add (sizeNY head) (sizeNX tail)

def sizeNY {α : Type u} : NY α → Nat
  | .leaf _ => 1
  | .branch x => sizeNX x

end
