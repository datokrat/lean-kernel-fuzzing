prelude
import Init.Prelude

inductive X (α : Type u) where
  | const : α → X α
  | apply : (α → X α) → X α

def eval {α : Type u} (a : α) : X α → α
  | .const b => b
  | .apply f => eval a (f a)
