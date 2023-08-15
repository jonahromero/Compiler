#pragma once
#include <utility>
#include <memory>

namespace visit {

	template<typename>
	class VisitorReturner;

	class Visitor;
	/* Inheriting from Visitable Requires they overload doAccept
	 *
	 */
	class VisitableBase {
	public:

		template<typename ReturnType>
		ReturnType accept(VisitorReturner<ReturnType>& visitor);
		void accept(Visitor& visitor);

	private:
		virtual void doAccept(Visitor& visitor) = 0;
	};

	template<typename Self, typename Base>
	class Visitable : public Base {
	private:
		virtual void doAccept(Visitor& visitor) override {
			//this just calls correct overload in Visitor based on template parameters to Visitable
			static_cast<void (Visitor::*)(Self&)>(Visitor::visit)(*dynamic_cast<Self*>(this));
		}
	};
	
	template<typename Self>
	class Visitor {
	public:
		void visitPtr(std::unique_ptr<VisitableBase>& visitable) {
			visitable->accept(*this);
		}
		template<typename VisitableChild> requires //has method
		void visit(VisitableChild& visitable) {
			static_cast<Self*>(this)->visit(visitable);
		}
		//otherwise do nothing
		template<typename VisitableChild>
		void visit(VisitableChild& visitable) {}
	};

	//worry later
	template<typename ReturnType>
	class VisitorReturner {
	public:
		ReturnType visitPtr(std::unique_ptr<Visitable>& visitable) {
			return visitable->accept(*this);
		}
	protected:
		ReturnType&& flushRetval() { return std::move(*getPtr()); }

		void returnValue(ReturnType&& retval) { new(this->retval) ReturnType(std::move(retval)); }
		void returnValue(ReturnType const& retval) { new(this->retval) ReturnType(retval); }
		template<typename U>
		void returnValue(U&& retval) {
			new(this->retval) ReturnType(std::forward<U>(retval));
		}
		virtual ~VisitorReturner() {
			getPtr()->~ReturnType();
		}
	private:
		alignas(ReturnType) char retval[sizeof(ReturnType)];
		ReturnType* getPtr() {
			return static_cast<ReturnType*>(static_cast<void*>(&retval));
		}
	};


	template<typename T>
	inline T Visitor::accept(VisitorReturner<T>& visitor)
	{
		doAccept(visitor);
		return visitor.flushRetval();
	}
	void Visitable::accept(Visitor& visitor) {
		doAccept(visitor);
	}
}