prelude
import Corpus.ExtendedPrelude

mutual

inductive X (α : Type u) where
  | pure : α → X α
  | y : Y α → X α

inductive Y (α : Type u) where
  | x : X α → Y α

end

mutual

def decodeX (α : Type u) : X α → α
  | .pure a => a
  | .y y => decodeY α y

def decodeY (α : Type u) : Y α → α
  | .x x => decodeX α x

end
