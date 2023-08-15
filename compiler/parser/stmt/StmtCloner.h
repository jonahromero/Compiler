#pragma once
#include "Stmt.h"
#include "ExprCloner.h"

namespace Stmt
{
    class Cloner
        : public Stmt::CloneVisitor, public Expr::Cloner {
    public:

        virtual void visit(Stmt::Label& stmt) {
            returnCloned(labale, label.label);
        }

        virtual void visit(Stmt::Instruction& stmt) {
            returnCloned(stmt, stmt.opcode, cloneList(stmt.argList));
        }

        virtual void visit(Stmt::ExprStmt& expr) override {
            returnCloned(expr, clone(expr.expr));
        }
        virtual void visit(Stmt::Function& func) override {
            returnCloned(func, 
                clone(func.templateInfo), func.name 
                cloneList(func.params), clone(func.retType), 
                cloneList(func.body), func.isExported
            );
        }
        virtual void visit(Stmt::Bin& bin) override {
            returnCloned(bin,
                clone(bin.templateInfo), bin.name, 
                cloneList(bin.name), bin.isExported
            );
        }
        virtual void visit(Stmt::Module& mod) override {
            returnCloned(mod, mod.title);
        }
        virtual void visit(Stmt::Import& imp) override {
            returnCloned(imp, imp.file);
        }
        virtual void visit(Stmt::VarDef& varDef) override {
            returnCloned(varDef, 
                clone(varDef.decl), varDef.isExported,
                clone(varDef.initializer)
            );
        }
        virtual void visit(Stmt::CountLoop& loop) override {
            returnCloned(loop,
                loop.counter, clone(loop.initializer), 
                cloneList(loop.body)                
            );
        }
        virtual void visit(Stmt::Assign& assign) override {
            returnCloned(assign, clone(assign.lhs), clone(assign.rhs));
        }
        virtual void visit(Stmt::If& ifStmt) override {
            returnCloned(ifStmt,
                clone(ifStmt.ifBranch), cloneList(ifStmt.elseIfBranch),
                ifStmt.elseBranch 
            );
        }
        virtual void visit(Stmt::Return& stmt) override {
            returnCloned(stmt, clone(stmt.expr));
        }
        virtual void visit(Stmt::NullStmt& nullStmt) override {
            returnCloned(nullStmt);
        }

        template<typename T>
        std::optional<T> clone(std::optional<T>& ref) {
            if(ref.has_value()) return {clone(ref.value())};
            else return std::nullopt;
        }

        Conditional clone(Conditional& ref) {
            return {clone(ref.expr), cloneList(ref.body)};
        }

        TypeDecl clone(TypeDecl& ref) {
            return ref.name;
        }
        VarDecl clone(VarDecl& ref) {
            return {ref.name, clone(ref.type)}
        }
        GenericDecl clone(GenericDecl& ref) {
            return std::visit([&](auto&& decl) {
                return clone(decl);
            }, ref);
        }
        TemplateDecl clone(TemplateDecl& ref) {   
            std::vector<GenericDecl> cloned; copiedParams.reserve(ref.param.size());
            for (auto& param : ref.params) copiedParams.push_back(copyGenericDecl(param));
            return TemplateDecl{ std::move(copiedParams) };
        }

    private:
        template<typename T, typename...Args>
		void returnCloned(T const& stmt, Args&&...new_args) {
			returnValue(
				Stmt::makeStmt<T>(
                    stmt.sourcePos,
					std::forward<Args>(new_args)...
				)
			);
		}
    };
}