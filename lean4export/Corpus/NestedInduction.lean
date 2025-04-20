prelude
import Corpus.ExtendedPrelude

inductive X (α : Type u) where
  | nil : X α
  | cons : α → X α → X α

inductive Y (α : Type u) where
  | leaf : α → Y α
  | branch : X (Y α) → Y α

mutual

def sizeX {α : Type u} : X (Y α) → Nat
  | .nil => 0
  | .cons head tail => Nat.add (sizeY head) (sizeX tail)

def sizeY {α : Type u} : Y α → Nat
  | .leaf _ => 1
  | .branch x => sizeX x

end
