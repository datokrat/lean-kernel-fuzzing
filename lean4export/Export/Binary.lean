import Lean

open Lean
open Std (HashMap)

namespace Binary

local instance : Hashable RecursorRule where
  hash r := hash (r.ctor, r.nfields, r.rhs)

structure Context where
  env : Environment
  goodIndices : Array ModuleIdx
  strings : Array String
  stringMap : HashMap String Nat

structure State where
  visitedNames : HashMap Name Nat := .insert {} .anonymous 0
  visitedLevels : HashMap Level Nat := .insert {} .zero 0
  visitedExprs : HashMap Expr Nat := {}
  visitedRecRules : HashMap RecursorRule Nat := {}
  visitedConstants : NameHashSet := {}
  data : ByteArray := {}

abbrev M := ReaderT Context <| StateT State IO

def computeStringMap (names : Array String) : HashMap String Nat := Id.run do
  let mut m : HashMap String Nat := ∅
  for h : i in [0:names.size] do
    m := m.insert names[i] i
  return m

def M.run (env : Environment) (goodIndices : Array ModuleIdx) (strings : Array String) (act : M α) : IO α :=
  StateT.run' (s := {}) <| Id.run do
    ReaderT.run (r := { env, goodIndices, strings, stringMap := computeStringMap strings }) do
      act

def writeBinaryData (s : String) : M Unit := do
  IO.FS.writeBinFile s (← get).data

def pushUInt8 (b : UInt8) : M Unit :=
  modify (fun s => { s with data := s.data.push b })

def pushUInt16 (b : UInt16) : M Unit := do
  pushUInt8 (b >>> 8).toUInt8
  pushUInt8 b.toUInt8

def pushUInt32 (b : UInt32) : M Unit := do
  pushUInt16 (b >>> 16).toUInt16
  pushUInt16 b.toUInt16

def pushNat8 (b : Nat) : M Unit :=
  if h : b < UInt8.size then
    pushUInt8 (UInt8.ofNatLT b h)
  else
    panic! "Nat overflow"

def pushNat16 (b : Nat) : M Unit :=
  if h : b < UInt16.size then
    pushUInt16 (UInt16.ofNatLT b h)
  else
    panic! "Nat overflow"

def pushU8 (a : UInt8) : M Unit :=
  pushUInt8 a

def pushU16 (a : UInt16) : M Unit :=
  pushUInt16 a

def pushN8 (a : Nat) : M Unit :=
  pushNat8 a

def pushN16 (a : Nat) : M Unit :=
  pushNat16 a

def pushU8N16 (a : UInt8) (b : Nat) : M Unit := do
  pushUInt8 a
  pushNat16 b

def pushU8U8N16 (x a : UInt8) (b : Nat) : M Unit := do
  pushUInt8 x
  pushUInt8 a
  pushNat16 b

def pushU8N16N16 (a : UInt8) (b c : Nat) : M Unit := do
  pushUInt8 a
  pushNat16 b
  pushNat16 c

def pushU8U8N16N16 (x a : UInt8) (b c : Nat) : M Unit := do
  pushUInt8 x
  pushUInt8 a
  pushNat16 b
  pushNat16 c

def pushU8N16N16N16 (a : UInt8) (b c d : Nat) : M Unit := do
  pushUInt8 a
  pushNat16 b
  pushNat16 c
  pushNat16 d

def pushU8U8N16N16N16 (x a : UInt8) (b c d : Nat) : M Unit := do
  pushUInt8 x
  pushUInt8 a
  pushNat16 b
  pushNat16 c
  pushNat16 d

def pushU8N16N16N16N16 (a : UInt8) (b c d e : Nat) : M Unit := do
  pushUInt8 a
  pushNat16 b
  pushNat16 c
  pushNat16 d
  pushNat16 e

def pushU8U8N16N16N16N16 (x a : UInt8) (b c d e : Nat) : M Unit := do
  pushUInt8 x
  pushUInt8 a
  pushNat16 b
  pushNat16 c
  pushNat16 d
  pushNat16 e

def pushNat16s (l : List Nat) : M Unit := do
  pushNat8 l.length
  l.forM pushNat16

def pushU8s (l : List UInt8) : M Unit := do
  pushNat8 l.length
  l.forM pushUInt8

def pushU8U8N16N16s (x a : UInt8) (b : Nat) (c : List Nat) : M Unit := do
  pushUInt8 x
  pushUInt8 a
  pushNat16 b
  pushNat16s c

def natToList (a : Nat) : List UInt8 := Id.run do
  let mut ans : List UInt8 := []
  let mut a' := a
  while 0 < a' do
    ans := (a' % UInt8.size).toUInt8 :: ans
    a' := a' / UInt8.size
  return ans

def pushNatAsU8s (a : Nat) : M Unit :=
  pushU8s (natToList a)

def pushU8U8NatList (x a : UInt8) (b : Nat) : M Unit := do
  pushUInt8 x
  pushUInt8 a
  pushNatAsU8s b

def pushString (s : String) : M Unit :=
  pushU8s s.toUTF8.toList

def pushU8U8String (x a : UInt8) (b : String) : M Unit := do
  pushUInt8 x
  pushUInt8 a
  pushString b

def stringMap : M (HashMap String Nat) := do
  return (← read).stringMap

def blockedNames : NameSet := .ofList [``sorryAx]

def shouldDump (n : Name) : M Bool := do
  if blockedNames.contains n then
    return false
  if n.toString.startsWith "Lean" then
    return false
  let env := (← read).env
  let good := (← read).goodIndices
  return good.contains (env.getModuleIdxFor? n).get!

@[inline]
def getIdx [Hashable α] [BEq α] [ToString α] (x : α) (getM : State → HashMap α Nat) (setM : State → HashMap α Nat → State) (rec : M Unit) : M Nat := do
  let m ← getM <$> get
  if let some idx := m[x]? then
    return idx
  rec
  let m ← getM <$> get
  let idx := m.size
  -- IO.println s!"{idx} -> {x}"
  modify fun st => setM st ((getM st).insert x idx)
  return idx

def getStringIndex (n : String) : M Nat := do
  match (← stringMap)[n]? with
  | none => panic! s!"Illegal string: {n}"
  | some i =>
    -- IO.println s!"{n} -> {i}"
    return i

def dumpName (n : Name) : M Nat := getIdx n (·.visitedNames) ({ · with visitedNames := · }) do
  match n with
  | .anonymous => unreachable!
  | .str p n => pushU8U8N16N16 7 0  (← dumpName p) (← getStringIndex n)
  | .num p i => pushU8U8N16N16 7 1 (← dumpName p) i

def dumpLevel (l : Level) : M Nat := getIdx l (·.visitedLevels) ({ · with visitedLevels := · }) do
  match l with
  | .zero | .mvar _ => unreachable!
  | .succ l => pushU8U8N16 0 0 (← dumpLevel l)
  | .max l1 l2 => pushU8U8N16N16 0 1 (← dumpLevel l1) (← dumpLevel l2)
  | .imax l1 l2 => pushU8U8N16N16 0 2 (← dumpLevel l1) (← dumpLevel l2)
  | .param n => pushU8U8N16 0 3 (← dumpName n)

def dumpLevels (l : List Level) : M (List Nat) :=
  l.mapM dumpLevel

def dumpNames (l : List Name) : M (List Nat) :=
  l.mapM dumpName

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
      | .bvar i => pure (false, e, pushU8U8N16 1 0 i)
      | .sort l => pure (false, e, do pushU8U8N16 1 1 (← dumpLevel l))
      | .const n us => pure (false, e, do pushU8U8N16N16s 1 2 (← dumpName n) (← dumpLevels us))
      | .lit (.natVal i) => pure (false, e, pushU8U8NatList 1 8 i)
      | .lit (.strVal s) => pure (false, e, pushU8U8String 1 9 s)
      | .app f a =>
        let (f_rebuilt, f, f_idx) ← dumpExprAux f
        let (a_rebuilt, a, a_idx) ← dumpExprAux a
        let ck := f_rebuilt || a_rebuilt
        pure (ck, if ck then e.updateApp! f a else e, do pushU8U8N16N16 1 3 f_idx a_idx)
      | .lam n d b _bi =>
        let (d_rebuilt, d, d_idx) ← dumpExprAux d
        let (b_rebuilt, b, b_idx) ← dumpExprAux b
        let ck := (d_rebuilt || b_rebuilt)
        pure (ck, if ck then e.updateLambdaE! d b else e, do pushU8U8N16N16N16 1 4 (← dumpName n) d_idx b_idx)
      | .letE n d v b _ =>
        let (d_rebuilt, d, d_idx) ← dumpExprAux d
        let (v_rebuilt, v, v_idx) ← dumpExprAux v
        let (b_rebuilt, b, b_idx) ← dumpExprAux b
        let ck := (d_rebuilt || v_rebuilt || b_rebuilt)
        pure (ck, if ck then e.updateLet! d v b else e, pushU8U8N16N16N16N16 1 6 (← dumpName n) d_idx v_idx b_idx)
      | .forallE n d b _bi =>
        let (d_rebuilt, d, d_idx) ← dumpExprAux d
        let (b_rebuilt, b, b_idx) ← dumpExprAux b
        let ck := (d_rebuilt || b_rebuilt)
        pure (ck, if ck then e.updateForallE! d b else e, do pushU8U8N16N16N16 1 5 (← dumpName n) d_idx b_idx)
      | .proj s i e2 =>
        let (e_rebuilt, ex, e'_idx) ← dumpExprAux e2
        let ck := e_rebuilt
        pure (ck, if ck then (e.updateProj! ex) else e, do pushU8U8N16N16N16 1 7 (← dumpName s) i e'_idx)
    tryPrintE ck e s
where
  tryPrintE (hadMData: Bool) (e : Expr) (s : M Unit) : M (Bool × Expr × Nat) := do
    /- If the expression had `mdata` and was rebuilt, check the rebuilt expression
    against the cache once more -/
    if hadMData then
      if let some idx := (← get).visitedExprs[e]? then
      return (hadMData, e, idx)
    let idx := (← get).visitedExprs.size
    modify (fun st => { st with visitedExprs := st.visitedExprs.insert e idx })
    s
    return (hadMData, e, idx)

@[inline]
def dumpExpr (e : Expr) : M Nat := do
  let (_, _, n) ← dumpExprAux e
  return n

def dumpHints : ReducibilityHints → M Unit
  | .opaque => pushU8 0
  | .abbrev => pushU8 1
  | .regular n => do
    pushU8 2
    pushUInt32 n

partial def dumpConstant (c : Name) : M Unit := do
  if (← get).visitedConstants.contains c then
    return
  if !(← shouldDump c) then
    return
  match (← read).env.find? c |>.get! with
  | .axiomInfo _ => do
    panic! "No axioms in binary mode"
  | .defnInfo val => do
    modify fun st => { st with visitedConstants := st.visitedConstants.insert c }
    if val.safety != .safe then
      return
    dumpDeps val.type
    dumpDeps val.value

    let eidx₁ ← dumpExpr val.type
    let eidx₂ ← dumpExpr val.value
    let n₁ ← dumpName c
    let names ← dumpNames val.levelParams

    pushU8 2
    pushN16 n₁
    pushN16 eidx₁
    pushN16 eidx₂
    dumpHints val.hints
    pushNat16s names
  | .thmInfo val => do
    modify fun st => { st with visitedConstants := st.visitedConstants.insert c }
    dumpDeps val.type
    dumpDeps val.value

    let eidx₁ ← dumpExpr val.type
    let eidx₂ ← dumpExpr val.value
    let n₁ ← dumpName c
    let names ← dumpNames val.levelParams

    pushU8 3
    pushN16 n₁
    pushN16 eidx₁
    pushN16 eidx₂
    pushNat16s names
  | .opaqueInfo _ => do
    panic! "No opaques in binary mode"
  | .quotInfo _ =>
    panic! "No quotients in binary mode"
  | .inductInfo val => do
    dumpInductive val
  | .ctorInfo val =>
    if val.isUnsafe then
      return
    if !((← get).visitedConstants.contains val.induct) then
      return
    modify fun st => { st with visitedConstants := st.visitedConstants.insert c }
    dumpDeps val.type

    let eidx ← dumpExpr val.type
    let n ← dumpName c

    pushU8 6
    pushN16 n
    pushN16 eidx
  | .recInfo val =>
    -- Don't care
    discard <| dumpName val.name
    return
where
  dumpDeps e := do
    for c in e.getUsedConstants do
      dumpConstant c
  dumpInductiveInner (val : InductiveVal) : M Unit := do
    if val.isUnsafe then
      return
    if !(← shouldDump val.name) then
      return
    if (← get).visitedConstants.contains c then
      return
    modify fun st => { st with visitedConstants := st.visitedConstants.insert c }
    dumpDeps val.type
    for ctor in val.ctors do
      dumpDeps ((← read).env.find? ctor |>.get!.type)
      dumpConstant ctor

    let ctorNameIdxs ← val.ctors.mapM (fun ctor => dumpName ctor)

    let eidx ← dumpExpr val.type
    let n ← dumpName c

    pushU8 4
    pushN16 n
    pushN16 eidx
    pushNat16s ctorNameIdxs
  dumpInductive (val : InductiveVal) : M Unit := do
    if val.isUnsafe then
      return
    for iname in val.all do
      let some (.inductInfo ival) := (← read).env.find? iname | panic! "Unexpected inductive"
      dumpInductiveInner ival
    let numParams := val.numParams
    let indNameIdxs ← val.all.mapM dumpName
    let levelIdxs ← dumpNames val.levelParams

    IO.println s!"{val.all}"
    IO.println s!"{levelIdxs}"

    pushU8 5
    pushN8 numParams
    pushNat16s indNameIdxs
    pushNat16s levelIdxs

end Binary
