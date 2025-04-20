prelude
import Corpus.ExtendedPrelude

inductive X : Type u → Type (u + 1) where
  | pure : α → X α
  | bind {α : Type u} {β : Type u} : (α → X β) → X β
