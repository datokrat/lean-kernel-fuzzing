prelude
import Corpus.ExtendedPrelude

inductive FX (α : Type u) where
  | const : α → FX α
  | apply : (α → FX α) → FX α

def eval {α : Type u} (a : α) : FX α → α
  | .const b => b
  | .apply f => eval a (f a)
