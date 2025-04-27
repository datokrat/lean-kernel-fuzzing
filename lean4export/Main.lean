import Export
import Export.Binary
open Lean

def semver := "markus-0.0.5"

def readStringsFromFile (s : String) : IO (Array String) := do
  return (← IO.FS.readFile s).split (· == '\n') |>.toArray

def extraNames : List Name := [`foo, `foo1, `foo2, `foo3, `foo4, `foo5, `foo6, `foo7, `foo8, `foo9,
  `Foo, `Foo1, `Foo2, `Foo3, `Foo4, `Foo5, `Foo6, `Foo7, `Foo8, `Foo9,
  `nat_beq₁, `nat_beq₂, `nat_beq₃, `nat_ble₂, `nat_ble₁, `nat_ble₃, `FX, `const, `apply, `eval,
  `X, `pure, `y, `Y, `decodeX, `decodeY, `PowerOfTwo, `one, `double, `t, `nat_mul₁,
  `nat_mul₂, `nat_mul₃, `NX, `leaf, `branch, `sizeNX, `NY, `sizeNY, `nat_pow,
  `lit_eq_lit, `lit_eq_mk, `nat_sub₁, `nat_sub₂, `nat_succ_eq_succ, `UnitLike, `ut,
  `UX, `nat_zero_eq_zero, `inductivePredicate, `Corpus, `InductivePredicate,
  `NestedInduction, `motive_1, `rec_1, `motive_2, `ibelow_1, `below_1, `F_2, `binductionOn_1,
  `brecOn_1, `UniverseBumpingInduction, `α_eq, `β_eq, `Inductive, `FunctionalInduction,
  `v_def, `ExcludedMiddle, `u_def, `hvf, `hut, `V, `U, `Classical2, `px, `indefiniteDescription,
  `choose, `choose_spec, `not_uv_or_p, `hne, `exU, `exV, `p_implies_uv, `hpred, `hl, `hr,
  `Funext, `funext2]

def binaryMode : Bool := true

/-
Example usage: lake exe lean4export Corpus.ExtendedPrelude
-/
def main (args : List String) : IO Unit := do
  initSearchPath (← findSysroot)
  let imports := args
  let names := imports.toArray.map fun mod => Syntax.decodeNameLit ("`" ++ mod) |>.get!
  let preludeMode := names.contains `Init.Prelude
  let imports := names.map ({ module := · })
  let env ← importModules imports {}
  let goodIndices := names.map (fun n => env.getModuleIdx? n |>.get!)
  let constants := env.constants.toList.map Prod.fst |>.filter
    (fun d => !d.isInternal && goodIndices.contains (env.getModuleIdxFor? d).get!)
  if preludeMode then
    M.run env goodIndices do
      IO.println semver
      for c in constants do
        let _ ← dumpConstant c
      discard <| extraNames.mapM dumpName
      writeStringsToFile "strings"
  else
    if binaryMode then
      let names ← readStringsFromFile "strings"
      Binary.M.run env goodIndices names do
        for c in constants do
          let _ ← Binary.dumpConstant c
        let filename := s!"ExportedCorpus/{imports[0]!}.belean"
        IO.println s!"Writing {filename}"
        Binary.writeBinaryData filename
    else
      M.run env goodIndices do
        IO.println semver
        for c in constants do
          let _ ← dumpConstant c
      -- let filename := s!"ExportedCorpus/{imports[0]!}.belean"
      -- IO.println s!"Writing {filename}"
      -- Binary.writeBinaryData filename
