import Lean

open Lean
open Std (HashMap)

instance : Hashable RecursorRule where
  hash r := hash (r.ctor, r.nfields, r.rhs)

structure Context where
  env : Environment
  goodIndices : Array ModuleIdx

structure State where
  visitedNames : HashMap Name Nat := .insert {} .anonymous 0
  visitedLevels : HashMap Level Nat := .insert {} .zero 0
  visitedExprs : HashMap Expr Nat := {}
  visitedRecRules : HashMap RecursorRule Nat := {}
  visitedConstants : NameHashSet := {}

abbrev M := ReaderT Context <| StateT State IO

def M.run (env : Environment) (goodIndices : Array ModuleIdx) (act : M α) : IO α :=
  StateT.run' (s := {}) do
    ReaderT.run (r := { env, goodIndices }) do
      act

def shouldDump (n : Name) : M Bool := do
  let env := (← read).env
  let good := (← read).goodIndices
  return good.contains (env.getModuleIdxFor? n).get!

@[inline]
def getIdx [Hashable α] [BEq α] (x : α) (getM : State → HashMap α Nat) (setM : State → HashMap α Nat → State) (pref : String) (rec : M String) : M Nat := do
  let m ← getM <$> get
  if let some idx := m[x]? then
    return idx
  let s ← rec
  let m ← getM <$> get
  let idx := m.size
  IO.println s!"{pref} {s}"
  modify fun st => setM st ((getM st).insert x idx)
  return idx

def dumpName (n : Name) : M Nat := getIdx n (·.visitedNames) ({ · with visitedNames := · }) "#NAME" do
  match n with
  | .anonymous => unreachable!
  | .str n s => return s!"#NS {← dumpName n} {s}"
  | .num n i => return s!"#NI {← dumpName n} {i}"

def dumpLevel (l : Level) : M Nat := getIdx l (·.visitedLevels) ({ · with visitedLevels := · }) "#LVL" do
  match l with
  | .zero | .mvar _ => unreachable!
  | .succ l => return s!"#US {← dumpLevel l}"
  | .max l1 l2 => return s!"#UM {← dumpLevel l1} {← dumpLevel l2}"
  | .imax l1 l2 => return s!"#UIM {← dumpLevel l1} {← dumpLevel l2}"
  | .param n => return s!"#UP {← dumpName n}"

def seq [ToString α] (xs : List α) : String :=
  xs.map toString |> String.intercalate " "

def dumpInfo : BinderInfo → String
  | .default => "#BD"
  | .implicit => "#BI"
  | .strictImplicit => "#BS"
  | .instImplicit => "#BC"

def uint8ToHex (c : UInt8) : String :=
  let d2 := c / 16
  let d1 := c % 16
  (hexDigitRepr d2.toNat ++ hexDigitRepr d1.toNat).toUpper

/-
Eliminate `mdata` nodes, while dumping an expression.

When an `mdata` node is encountered, skip it. For everything else, update `e`
IFF a sub-expression contained `mdata`.

returns `(had mdata, the expression with any mdata removed, e's export index)`
-/
@[inline]
partial def dumpExprAux (e : Expr) : M (Bool × Expr × Nat) := do
  /- If this is an mdata node, dump the inner item and return the expr without mdata -/
  if let .mdata _ e' := e then
    let (_, e'', idx) ← dumpExprAux e'
    return (true, e'', idx)
  /- If we've already seen this expression, use the cached data -/
  if let some idx := (← get).visitedExprs[e]? then
    return (false, e, idx)
  else
    let (ck, e, s) ←
      match e with
      | .mdata .. | .fvar .. | .mvar .. => unreachable!
      | .bvar i => pure (false, e, (return s!"#EV {i}"))
      | .sort l => pure (false, e, (return s!"#ES {← dumpLevel l}"))
      | .const n us => pure (false, e, (return s!"#EC {← dumpName n} {← seq <$> us.mapM dumpLevel}"))
      | .lit (.natVal i) => pure (false, e, (return s!"#ELN {i}"))
      | .lit (.strVal s) => pure (false, e, (return s!"#ELS {s.toUTF8.toList.map uint8ToHex |> seq}"))
      | .app f a =>
        let (f_rebuilt, f, f_idx) ← dumpExprAux f
        let (a_rebuilt, a, a_idx) ← dumpExprAux a
        let ck := f_rebuilt || a_rebuilt
        pure (ck, if ck then e.updateApp! f a else e, (return s!"#EA {f_idx} {a_idx}"))
      | .lam n d b bi =>
        let (d_rebuilt, d, d_idx) ← dumpExprAux d
        let (b_rebuilt, b, b_idx) ← dumpExprAux b
        let ck := (d_rebuilt || b_rebuilt)
        pure (ck, if ck then e.updateLambdaE! d b else e, (return s!"#EL {dumpInfo bi} {← dumpName n} {d_idx} {b_idx}"))
      | .letE n d v b _ =>
        let (d_rebuilt, d, d_idx) ← dumpExprAux d
        let (v_rebuilt, v, v_idx) ← dumpExprAux v
        let (b_rebuilt, b, b_idx) ← dumpExprAux b
        let ck := (d_rebuilt || v_rebuilt || b_rebuilt)
        pure (ck, if ck then e.updateLet! d v b else e, (return s!"#EZ {← dumpName n} {d_idx} {v_idx} {b_idx}"))
      | .forallE n d b bi =>
        let (d_rebuilt, d, d_idx) ← dumpExprAux d
        let (b_rebuilt, b, b_idx) ← dumpExprAux b
        let ck := (d_rebuilt || b_rebuilt)
        pure (ck, if ck then e.updateForallE! d b else e, (return s!"#EP {dumpInfo bi} {← dumpName n} {d_idx} {b_idx}"))
      | .proj s i e2 =>
        let (e_rebuilt, ex, e'_idx) ← dumpExprAux e2
        let ck := e_rebuilt
        pure (ck, if ck then (e.updateProj! ex) else e, (return s!"#EJ {← dumpName s} {i} {e'_idx}"))
    tryPrintE ck e s
where
  tryPrintE (hadMData: Bool) (e : Expr) (s : M String) : M (Bool × Expr × Nat) := do
    /- If the expression had `mdata` and was rebuilt, check the rebuilt expression
    against the cache once more -/
    if hadMData then
      if let some idx := (← get).visitedExprs[e]? then
      return (hadMData, e, idx)
    let idx := (← get).visitedExprs.size
    modify (fun st => { st with visitedExprs := st.visitedExprs.insert e idx })
    IO.println s!"#EXPR {← s}"
    return (hadMData, e, idx)

@[inline]
def dumpExpr (e : Expr) : M Nat := do
  let (_, _, n) ← dumpExprAux e
  return n

def dumpHints : ReducibilityHints → String
  | .opaque => "O"
  | .abbrev => "A"
  | .regular n => s!"R {n}"

partial def dumpConstant (c : Name) : M Unit := do
  if (← get).visitedConstants.contains c then
    return
  match (← read).env.find? c |>.get! with
  | .axiomInfo val => do
    modify fun st => { st with visitedConstants := st.visitedConstants.insert c }
    if val.isUnsafe then
      return
    dumpDeps val.type
    IO.println s!"#AX {← dumpName c} {← dumpExpr val.type} {← seq <$> val.levelParams.mapM dumpName}"
  | .defnInfo val => do
    modify fun st => { st with visitedConstants := st.visitedConstants.insert c }
    if val.safety != .safe then
      return
    dumpDeps val.type
    dumpDeps val.value
    IO.println s!"#DEF {← dumpName c} {← dumpExpr val.type} {← dumpExpr val.value} {dumpHints val.hints} {← seq <$> val.levelParams.mapM dumpName}"
  | .thmInfo val => do
    modify fun st => { st with visitedConstants := st.visitedConstants.insert c }
    dumpDeps val.type
    dumpDeps val.value
    IO.println s!"#THM {← dumpName c} {← dumpExpr val.type} {← dumpExpr val.value} {← seq <$> val.levelParams.mapM dumpName}"
  | .opaqueInfo val => do
    modify fun st => { st with visitedConstants := st.visitedConstants.insert c }
    if val.isUnsafe then
      return
    dumpDeps val.type
    dumpDeps val.value
    IO.println s!"#OPAQ {← dumpName c} {← dumpExpr val.type} {← dumpExpr val.value} {← seq <$> val.levelParams.mapM dumpName}"
  | .quotInfo _ =>
    modify fun st => { st with visitedConstants := st.visitedConstants.insert c }
    IO.println s!"#QUOT"
    return
  | .inductInfo val => do
    dumpInductive val
  | .ctorInfo val =>
    if val.isUnsafe then
      return
    if !((← get).visitedConstants.contains val.induct) then
      return
    modify fun st => { st with visitedConstants := st.visitedConstants.insert c }
    dumpDeps val.type
    IO.println s!"#CTOR {← dumpName c} {← dumpExpr val.type} {← dumpName val.induct} {val.cidx} {val.numParams} {val.numFields} {← seq <$> val.levelParams.mapM dumpName}"
  | .recInfo _ =>
    -- Don't care
    return
where
  dumpDeps e := do
    for c in e.getUsedConstants do
      dumpConstant c
  dumpInductiveInner (val : InductiveVal) : M Unit := do
    if val.isUnsafe then
      return
    if (← get).visitedConstants.contains c then
      return
    modify fun st => { st with visitedConstants := st.visitedConstants.insert c }
    dumpDeps val.type
    for ctor in val.ctors do
      dumpDeps ((← read).env.find? ctor |>.get!.type)
      dumpConstant ctor

    let ctorNameIdxs ← val.ctors.mapM (fun ctor => dumpName ctor)
    IO.println s!"#IND {← dumpName c} {← dumpExpr val.type} {val.numCtors} {seq ctorNameIdxs}"
  dumpInductive (val : InductiveVal) : M Unit := do
    if val.isUnsafe then
      return
    for iname in val.all do
      let some (.inductInfo ival) := (← read).env.find? iname | panic! "Unexpected inductive"
      dumpInductiveInner ival
    let numParams := val.numParams
    let indNameIdxs ← val.all.mapM dumpName
    IO.println s!"#INDF {numParams} {indNameIdxs.length} {seq indNameIdxs} {← seq <$> val.levelParams.mapM dumpName}"
