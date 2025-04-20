import Export
open Lean

def semver := "markus-0.0.5"

/-
Example usage: lake exe lean4export Corpus.ExtendedPrelude
-/
def main (args : List String) : IO Unit := do
  initSearchPath (← findSysroot)
  let (imports, constants) := args.span (· != "--")
  let names := imports.toArray.map fun mod => Syntax.decodeNameLit ("`" ++ mod) |>.get!
  let imports := names.map ({ module := · })
  let env ← importModules imports {}
  let goodIndices := names.map (fun n => env.getModuleIdx? n |>.get!)
  let constants := match constants.tail? with
    | some cs => cs.map fun c => Syntax.decodeNameLit ("`" ++ c) |>.get!
    | none    => env.constants.toList.map Prod.fst |>.filter (fun d => !d.isInternal && goodIndices.contains (env.getModuleIdxFor? d).get!)
  M.run env goodIndices do
    IO.println semver
    for c in constants do
      let _ ← dumpConstant c
