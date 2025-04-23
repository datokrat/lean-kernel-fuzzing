#include "std/mem.pat"

using SIdx = be u16;
using NIdx = be u16;
using UIdx = be u16;
using EIdx = be u16;

struct StringName {
  NIdx par;
  SIdx string;
};

struct NumName {
  NIdx par;
  be u16 i;
};

enum NameType : u8 {
  String = 0x00,
  Num = 0x01
};

struct Names {
  u8 numNames;
  NIdx names[numNames];
};

struct Levels {
  u8 numLevels;
  UIdx levels[numLevels];
};

struct Name {
  NameType type;
  
  match (type) {
    (NameType::String): StringName name;
    (NameType::Num): NumName name;
  }
} [[color("000000")]];

struct SuccLevel {
  UIdx par;
};

struct MaxLevel {
  UIdx lhs;
  UIdx rhs;
};

struct IMaxLevel {
  UIdx lhs;
  UIdx rhs;
};

struct LevelParam {
  NIdx name;
};

enum LevelType : u8 {
  Succ = 0x00,
  Max = 0x01,
  IMax = 0x02,
  Param = 0x03
};

struct Level {
  LevelType type;
  
  match (type) {
    (LevelType::Succ): SuccLevel level;
    (LevelType::Max): MaxLevel level;
    (LevelType::IMax): IMaxLevel level;
    (LevelType::Param): LevelParam level;
  }
} [[color("E69F00")]];

struct BoundVariable {
  be u16 deBruijnIndex;
};

struct Sort {
  UIdx level;
};

struct Constant {
  NIdx name;
  Levels levels;
};

struct Application {
  EIdx lhs;
  EIdx rhs;
};

struct Lambda {
  NIdx binderName;
  EIdx binderType;
  EIdx body;
};

struct Pi {
  NIdx binderName;
  EIdx binderType;
  EIdx body;
};

struct Let {
  NIdx binderName;
  EIdx binderType;
  EIdx boundValue;
  EIdx body;
};

struct Proj {
  NIdx typeName;
  be u16 index;
  EIdx expr;
};

struct NatLiteral {
  u8 numBytes;
  u8 bytes[numBytes];
};

struct StringLiteral {
  u8 numBytes;
  u8 butes[numBytes];
};

enum ExprType : u8 {
  Variable = 0x00,
  Sort = 0x01,
  Constant = 0x02,
  Application = 0x03,
  Lambda = 0x04,
  Pi = 0x05,
  Let = 0x06,
  Proj = 0x07,
  NatLiteral = 0x08,
  StringLiteral = 0x09
};

struct Expr {
  ExprType type;
  
  match (type) {
    (ExprType::Variable): BoundVariable bvar;
    (ExprType::Sort): Sort sort;
    (ExprType::Constant): Constant constant;
    (ExprType::Application): Application app;
    (ExprType::Lambda): Lambda lambda;
    (ExprType::Pi): Pi pi;
    (ExprType::Let): Let let;
    (ExprType::Proj): Proj proj;
    (ExprType::NatLiteral): NatLiteral natLiteral;
    (ExprType::StringLiteral): StringLiteral stringLiteral;
  }
} [[color("0072B2")]];

enum HintType : u8 {
  Opaque = 0x00,
  Abbrev = 0x01,
  Regular = 0x02
};

struct Hint {
  HintType type;
  
  if (type == HintType::Regular) {
    be u32 val;
  }
};

struct Definition {
  NIdx name;
  EIdx type;
  EIdx value;
  Hint hint;
  Names levels;
} [[color("56B4E9")]];

struct Theorem {
  NIdx name;
  EIdx type;
  EIdx value;
  Names levels;
} [[color("009E73")]];

struct Inductive {
  NIdx name;
  EIdx type;
  Names constructors;
} [[color("F0E442")]];

struct InductiveFamily {
  u8 numParams;
  Names inductives;
  Names levels;
} [[color("D55E00")]];

struct Constructor {
  NIdx name;
  EIdx type;
} [[color("CC79A7")]];

enum DeclarationType : u8 {
  Level = 0x00,
  Expr = 0x01,
  Definition = 0x02,
  Theorem = 0x03,
  Inductive = 0x04,
  InductiveFamily = 0x05,
  Constructor = 0x06,
  Name = 0x07
} [[color("FFFFFF")]];

struct Declaration {
  DeclarationType type;
  
  match (type) {
    (DeclarationType::Level): Level level;
    (DeclarationType::Expr): Expr expr;
    (DeclarationType::Definition): Definition definition;
    (DeclarationType::Theorem): Theorem theorem;
    (DeclarationType::Inductive): Inductive inductive;
    (DeclarationType::InductiveFamily): InductiveFamily inductiveFamily;
    (DeclarationType::Constructor): Constructor constructor;
    (DeclarationType::Name): Name name;
  }
};

Declaration declarations[while(!std::mem::eof())] @ 0x00;