#pragma once

/*
;Macros

Program -> *Statement
Statement -> Label | Instruc | DirectiveStmt

Terminator -> *("\n" | "\\") | EOF
Arguments -> (Expr *("," Expr))

;Basic Assembly
Label -> IDENT ":" 

Instruc -> Opcode Arguments? Terminator
Opcode -> "add" | "adc" | "ld" | ...

DirectiveStmt -> "." Directive Arguments? Terminator
Directive -> "db" | "dw" | ...

;Expressions
Expr -> Logical
Logical -> Bitwise ("&&" | "||") Bitwise
Bitwise -> Comparison (*("&" | "^" | "|") Comparison)
Comparison -> Equality *(("<" | "<=" | ">" | ">=") Equality)
Equality -> Bitshift *(("==" | "!=") Bitshift)
Bitshift -> Term *(("<<" | ">>") Term) 
Term -> Factor *(("+" | "-") Factor)
Factor -> Unary *(("*" | "/" | "%") Unary) 
Unary -> (("-" | "!" | "~") Unary) | Primary
Primary -> NUMBER | STRING | IDENT | REGISTER "(" expr ")" | "$"
*/