#pragma once
#include <cstdint>
#include "Stmt.h"
#include "Parser.h"
#include "Assembler.h"

/*Will eventually support lang features*/
/* Error Handling:
	There can be errors while, 
	Lexing: Handled in Compiler
	Parsing: Handled in Compiler
	Converting to Operands: Handled in Compiler
	Assembling Operands: Handled in Compiler
*/
class Compiler : 
	public Stmt::VisitorReturner<Asm::Bytes>
{
public:
	Compiler(std::string_view input);
	Asm::Bytes compile();
private:
	void runIdentifierPass();

	virtual void visit(Stmt::Function& func) override {}
	virtual void visit(Stmt::Bin& bin) override {}
	virtual void visit(Stmt::Module& mod) override {}
	virtual void visit(Stmt::Import& imp) override {}
	virtual void visit(Stmt::VarDef& varDef) override {}
	virtual void visit(Stmt::CountLoop& loop) override {}
	virtual void visit(Stmt::Assign& assign) override {}
	virtual void visit(Stmt::If& ifStmt) override {}
	virtual void visit(Stmt::Label& label) override;
	virtual void visit(Stmt::Instruction& stmt) override;
	virtual void visit(Stmt::NullStmt& nullStmt) override {} //do nothing

	Parser::Program program;
	Asm::LabelContext labelContext;
	bool isIdentifierPass = false;
	uint16_t pc = 0;
};

