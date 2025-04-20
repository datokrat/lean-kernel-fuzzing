prelude
import Corpus.ExtendedPrelude

inductive UX : Type u → Type (u + 1) where
  | pure : α → UX α
  | bind {α : Type u} {β : Type u} : (α → UX β) → UX β
