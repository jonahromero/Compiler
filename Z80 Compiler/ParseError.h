#pragma once
#include "CompilerError.h"

/*
* There are basically three types of Parsing Errors that will occur. Ones that are very specific
* and require lots of additional data from the context of the parser. Ones that happen very often 
* and deserve their own class, as theyre constantly being called. Or finally ones that are generic 
* to only need an error message in their constructor.
*/

class ParseError : public CompilerError {
public:
	using CompilerError::CompilerError;
};