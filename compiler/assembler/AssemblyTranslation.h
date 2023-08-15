

fn foo(#0 i8) -> i8 {

}

//z80
// 
// Each function will clean up the registers it used.
// HL and A are subject to being destroyed as parameters
// 
// passing values:
// if its one 8 bits, it comes in a register
// if its 16 bits, it comes in hl register
// otherwise, its pushed onto the stack, with hl at base of arguments
// 
// returning values
// if its 8 bits, returned in a register
// if its 16 bits, return in hl register 
// otherwise itll be stored in a value before the arguments, pointed to by hl
//
// [<Return Value>]-[Arg 1]-[Arg 2]-...-[Arg N]: SP
//
// Multiply function and Division function will be provided
//


foo:
	