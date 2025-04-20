A simple declaration exporter for Lean 4. 

An exporter is a program which emits Lean declarations using Lean's kernel language, for consumption by external type checkers. Producing an export file is a complete exit from the Lean toolchain, and the data in the file can be checked with entirely external software.

The file format is described later in this document, and examples of the exporter's output can be found in the `examples` folder of this repository.

## How to Run

After building the program, users can invoke the exporter from the command line.

```sh
$ lake exe lean4export <mods> [-- <decls>]
```
This exports the contents of the given Lean modules, looked up in the core library or `LEAN_PATH` (as e.g. initialized by an outer `lake env`) and their transitive dependencies.

A specific list of declarations to be exported from these modules can be given after a separating `--`.

If you're getting an error about not finding your project, you can run the exporter against your current directory without modifying the `LEAN_PATH`:

```sh
$ lake env <path to lean4export bin> <mods> [-- <decls>]
```

## Format Extensions

The following commands have been added to represent new features of the Lean 4 type system.

```
<eidx'> #EJ <nidx> <integer> <eidx>
```
A primitive projection of the `<integer>`-nth field of a value `<eidx>` of the record type `<nidx>`.
Example: the primitive projection corresponding to `Prod.snd` of the innermost bound variable
```
1 #NS 0 Prod
0 #EV 0
1 #EJ 1 1 0
```

```
<eidx'> #ELN <integer>
<eidx'> #ELS <hexhex>*
```
Primitive literals of type `Nat` and `String` (encoded as a sequence of UTF-8 bytes in hexadecimal), respectively.
Examples:
```
0 #ELN 1000000000000000
1 #ELS 68 69  # "hi"
```

```
<eidx'> #EZ <nidx> <eidx_1> <eidx_2> <eidx_3>
```
A binding `let <nidx> : <eidx_1> := <eidx_2>; <eidx_3>`.
Already supported by the Lean 3 export format, but not documented.
Example: the encoding of `let x : Nat := Nat.zero; x` is
```
1 #NS 0 x
2 #NS 0 Nat
0 #EC 2 
3 #NS 2 zero
1 #EC 3 
2 #EV 0
3 #EZ 1 0 1 2
```

## Export file format (ver 0.1.2)

For clarity, some of the compound items are decorated here with a name, for example `(name : T)`, but they appear in the export file as just an element of `T`.

**NOTE:** readers writing their own parsers and/or checkers should initialize names[0] as the anonymous name, and levels[0] as universe zero, as they are not emitted by the exporter, but are expected to occupy the name and level indices for 0.

```
File ::= ExportFormatVersion Item*

ExportFormatVersion ::= nat '.' nat '.' nat

Item ::= Name | Universe | Expr | Declaration

Declaration ::= 
    | Axiom 
    | Definition 
    | Theorem 
    | Inductive 
    | InductiveFamily
    | Opaque
    | Constructor 
    | Quotient

nidx, uidx, eidx, ridx ::= nat

Name ::=
  | "#NAME" "#NS" nidx string
  | "#NAME" "#NI" nidx nat

Universe ::=
  | "#LVL" "#US"  uidx
  | "#LVL" "#UM"  uidx uidx
  | "#LVL" "#UIM" uidx uidx
  | "#LVL" "#UP"  nidx

Expr ::=
  | "#EXPR" "#EV"  nat
  | "#EXPR" "#ES"  uidx
  | "#EXPR" "#EC"  nidx uidx*
  | "#EXPR" "#EA"  eidx eidx
  | "#EXPR" "#EL"  Info nidx eidx
  | "#EXPR" "#EP"  Info nidx eidx eidx
  | "#EXPR" "#EZ"  Info nidx eidx eidx eidx
  | "#EXPR" "#EJ"  nidx nat eidx
  | "#EXPR" "#ELN" nat
  | "#EXPR" "#ELS" (hexhex)*

Info ::= "#BD" | "#BI" | "#BS" | "#BC"

Hint ::= "O" | "A" | "R" nat

Axiom ::= "#AX" (name : nidx) (type : eidx) (uparams : nidx*)

Def ::= "#DEF" (name : nidx) (type : eidx) (value : eidx) (hint : Hint) (uparams : nidx*)
  
Theorem ::= "#THM" (name : nidx) (type : eidx) (value : eidx) (uparams: nidx*)

Quotient ::= "#QUOT"

Inductive ::= 
  "#IND"
  (name : nidx) 
  (type : eidx) 
  (numConstructors : nat) 
  (constructorNames : nidx {numConstructors}) 

InductiveFamily ::=
  "#INDF"
  (numParams: nat)
  (numInductives: nat)
  (inductiveNames: nat)
  (uparams: nidx*)

Constructor ::= 
  "#CTOR"
  (name : nidx) 
  (type : eidx) 
  (parentInductive : nidx) 
  (constructorIndex : nat)
  (numParams : nat)
  (numFields : nat)
  (uparams: uidx*)

```
