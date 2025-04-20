import Lake
open Lake DSL

package lean4export

lean_lib Export
lean_lib Test
lean_lib Corpus

@[default_target]
lean_exe lean4export where
  root := `Main
  supportInterpreter := true
