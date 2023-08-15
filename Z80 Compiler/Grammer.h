#pragma once



Module -> "module" IDENT
Program->Module *DocStmt
DocStmt -> Bin | TypeDef | VarDef | Func | Import

Terminator -> *("\n") | EOF

;decls are helpers
VarDecl -> IDENT ":" Type
TypeDecl -> "type" IDENT

Type -> ("mut")? IDENT ("<" (Type * ("," Type)) ">")?
VarDef -> ("let" | "export") VarDecl ("=" Expr) ?
TypeDef -> ("export") ? TypeDecl "=" type
Template -> "<" (VarDecl | TypeDecl) * ("," (VarDecl | TypeDecl)) ">"

Import -> "import" IDENT
Bin -> "export"? "bin" Template? IDENT ":" BinBlock
BinBlock -> * (VarDecl Terminator)

Func -> "export"? "fn" IDENT "(" IDENT ":" Type *("," IDENT ":" Type) ")" ":" StmtBlock
StmtBlock -> *(For | If | Return | Label | Instruc | Assign)

Assign -> Expr "=" Expr
If -> "if" Expr ":" StmtBlock *("else" "if" ":" StmtBlock) ("else" ":" StmtBlock)?
Loop -> "count" "with" IDENT "from" Expr ":" StmtBlock
Return -> "return" Expr

Label->IDENT ":"
Instruc -> Opcode Arguments? Terminator
Opcode -> "add" | "adc" | "ld" | ...

;Expressions 
Expr -> Logical 
Logical -> Bitwise ("&&" | "||") Bitwise 
Bitwise -> Comparison (*("&" | "^" | "|") Comparison) 
Comparison -> Equality *(("<" | "<=" | ">" | ">=") Equality) 
Equality -> Bitshift *(("==" | "!=") Bitshift) 
Bitshift -> Term *(("<<" | ">>") Term) 
Term -> Factor *(("+" | "-") Factor) 
Factor -> Unary *(("*" | "/" | "%") Unary) 
Unary -> (("-" | "!" | "~" | "#") Unary) | Primary 
Secondary -> 

Arguments -> (Expr * ("," Expr)) 
Primary -> NUMBER | STRING | IDENT | REGISTER | "(" expr ")" | "$" | 
	 Expr "." IDENT | Expr "[" Expr "]" | Expr "(" Arguments ")"





fn<T: type, size : u8>

*/