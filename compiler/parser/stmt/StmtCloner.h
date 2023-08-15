#pragma once
#include "Stmt.h"
#include "ExprCloner.h"
#include "VectorUtil.h"

namespace Stmt
{
    class Cloner
        : public Stmt::CloneVisitor<Cloner>
    {
        using Stmt::CloneVisitor<Cloner>::cloneList;
    public:
        using Stmt::CloneVisitor<Cloner>::clone;

        virtual void visit(Function const& func) override {
            returnCloned(func,
                clone(func.templateInfo), func.name,
                this->cloneList(func.params), clone(func.retType),
                this->cloneList(func.body), func.isExported
            );
        }
        virtual void visit(Bin const& bin) override {
            returnCloned(bin,
                clone(bin.templateInfo), bin.name,
                this->cloneList(bin.body), bin.isExported
            );
        }
    private:

        virtual void visit(Label const& stmt) override {
            returnCloned(stmt, stmt.label);
        }

        virtual void visit(Instruction const& stmt) override {
            returnCloned(stmt, stmt.opcode, cloneList(stmt.argList));
        }

        virtual void visit(ExprStmt const& expr) override {
            returnCloned(expr, clone(expr.expr));
        }
        virtual void visit(Module const& mod) override {
            returnCloned(mod, mod.title);
        }
        virtual void visit(Import const& imp) override {
            returnCloned(imp, imp.file);
        }
        virtual void visit(VarDef const& varDef) override {
            returnCloned(varDef, 
                clone(varDef.decl), varDef.isExported,
                clone(varDef.initializer)
            );
        }
        virtual void visit(CountLoop const& loop) override {
            returnCloned(loop,
                loop.counter, clone(loop.initializer), 
                cloneList(loop.body)                
            );
        }
        virtual void visit(Assign const& assign) override {
            returnCloned(assign, clone(assign.lhs), clone(assign.rhs));
        }
        virtual void visit(If const& ifStmt) override {
            returnCloned(ifStmt,
                clone(ifStmt.ifBranch), cloneList(ifStmt.elseIfBranch),
                cloneList(ifStmt.elseBranch)
            );
        }
        virtual void visit(Return const& stmt) override {
            returnCloned(stmt, clone(stmt.expr));
        }
        virtual void visit(NullStmt const& nullStmt) override {
            returnCloned(nullStmt);
        }


        template<typename T>
        std::optional<T> clone(std::optional<T> const& ref) {
            if(ref.has_value()) return {clone(ref.value())};
            else return std::nullopt;
        }

        Conditional clone(Conditional const& ref) {
            return {clone(ref.expr), cloneList(ref.body)};
        }

        TypeDecl clone(TypeDecl const& ref) {
            return ref.name;
        }
        VarDecl clone(VarDecl const& ref) {
            return { ref.name, clone(ref.type) };
        }
        GenericDecl clone(GenericDecl const& ref) {
            return std::visit([&](auto&& decl) {
                return GenericDecl(clone(decl));
            }, ref);
        }
        TemplateDecl clone(TemplateDecl const& ref) {   
            return TemplateDecl{ cloneList(ref.params) };
        }
        Expr::UniquePtr clone(Expr::UniquePtr const& expr) {
            return Expr::Cloner{}.clone(expr);
        }
        
        template<typename T>
        std::vector<T> cloneList(std::vector<T> const& list) {
            return util::transform_vector(list, [&](T const& elem) {
                return this->clone(elem);
            });
        }

        template<typename T, typename...Args>
		void returnCloned(T const& stmt, Args&&...new_args) {
			returnValue(
				makeStmt<T>(
                    stmt.sourcePos,
					std::forward<Args>(new_args)...
				)
			);
		}
    };
}