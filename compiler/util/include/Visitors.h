#pragma once
#include <utility>
#include <memory>
#include <assert.h>

namespace visit {
    namespace detail 
    {
        template<bool cond, typename T>
        using conditional_const_t = std::conditional_t<cond, const T, T>;
        template<typename T, typename...Args>
        concept IsIn = (std::is_same_v<std::remove_cvref_t<T>, Args> || ...);

        template<typename ConcreteBase, bool isConst, typename...ConcreteChildren>
        class VisitorVFunctions;

        template<typename ConcreteBase, bool isConst, typename ConcreteChild>
        class VisitorVFunctions<ConcreteBase, isConst, ConcreteChild> {
        protected:
            virtual void visit(conditional_const_t<isConst, ConcreteChild>&) = 0;
        };

        template<typename ConcreteBase, bool isConst, typename ConcreteChild, typename...ConcreteChildren>
        class VisitorVFunctions<ConcreteBase, isConst, ConcreteChild, ConcreteChildren...>
            : public VisitorVFunctions<ConcreteBase, isConst, ConcreteChildren...> {
        protected:
            using VisitorVFunctions<ConcreteBase, isConst, ConcreteChildren...>::visit;
            virtual void visit(conditional_const_t<isConst, ConcreteChild>&) = 0;
        };

        template<typename T>
        struct type_identity {
            using type = T;
        };
        template<typename T>
        using type_identity_t = typename type_identity<T>::type;

        template<typename ConcreteBase, bool isConst, typename...ConcreteChildren>
        class Visitor : protected VisitorVFunctions<ConcreteBase, isConst, ConcreteChildren...> {
        public:
            template<typename, typename> friend class Visitable;
            virtual ~Visitor() = default;

            void visitChild(std::unique_ptr<ConcreteBase> const& ptr) {
                ptr->accept(*this);
            }

            template<IsIn<ConcreteChildren...> T>
            void visitChild(type_identity_t<conditional_const_t<isConst, T>>& concrete) {
                concrete.accept(*this);
            }
        };

        template<typename ConcreteBase, typename ReturnType, bool isConst, typename...ConcreteChildren>
        class VisitorReturner : protected Visitor<ConcreteBase, isConst, ConcreteChildren...>
        {
        public:
            template<typename, typename> friend class Visitable;

            ReturnType visitChild(std::unique_ptr<ConcreteBase> const& ptr) {
                returnedValue = false;
                this->Visitor<ConcreteBase, isConst, ConcreteChildren...>::visitChild(ptr);
                return flushRetval();
            }
            template<IsIn<ConcreteChildren...> T>
            ReturnType visitChild(conditional_const_t<isConst, T>& concrete) {
                returnedValue = false;
                this->Visitor<ConcreteBase, isConst, ConcreteChildren...>::visitChild<T>(concrete);
                return flushRetval();
            }

            VisitorReturner() = default;
            VisitorReturner(VisitorReturner const&) = delete;
            VisitorReturner& operator=(VisitorReturner const&) = delete;
            
            virtual ~VisitorReturner() 
            {
                if (hasBeenConstructed) {
                    getPtr()->~ReturnType();
                }
            }
        protected:
            void returnValue(ReturnType&& retval) const { returnedValue = hasBeenConstructed = true; new(this->retval) ReturnType(std::move(retval)); }
            void returnValue(ReturnType const& retval) const { returnedValue = hasBeenConstructed = true; new(this->retval) ReturnType(retval); }
            template<typename U>
            void returnValue(U&& retval) const {
                returnedValue = hasBeenConstructed = true;
                new(this->retval) ReturnType(std::forward<U>(retval));
            }
        private:
            alignas(ReturnType) mutable char retval[sizeof(ReturnType)];
            mutable bool hasBeenConstructed = false, returnedValue = false;

            ReturnType&& flushRetval() const { 
                assert(("Visitor Function did not return a value." && returnedValue));
                return std::move(*getPtr()); 
            }

            ReturnType* getPtr() const {
                return static_cast<ReturnType*>(static_cast<void*>(&retval));
            }
        };

        template<typename ConcreteVisitor, typename ConcreteBase, typename...ConcreteChildren>
        class CloneVisitor : protected VisitorReturner<ConcreteBase, std::unique_ptr<ConcreteBase>, true, ConcreteChildren...>
        {
            using BaseReturner = VisitorReturner<ConcreteBase, std::unique_ptr<ConcreteBase>, true, ConcreteChildren...>;
        public:
            std::unique_ptr<ConcreteBase> clone(std::unique_ptr<ConcreteBase> const& other) {
                return this->BaseReturner::visitChild(other);
            } 
            template<IsIn<ConcreteChildren...> T>
            T clone(T const& other) {
                return this->BaseReturner::visitChild<T>(other);
            }
        protected:
            template<IsIn<ConcreteChildren...> T>
            auto cloneList(std::vector<T> const& list) const {
                std::vector<T> cloned; cloned.reserve(list.size());
                for (auto& val : list) cloned.emplace_back(ConcreteVisitor{}.clone(val));
                return cloned;
            }
            auto cloneList(std::vector<std::unique_ptr<ConcreteBase>> const& list) const
            {
                std::vector<std::unique_ptr<ConcreteBase>> cloned; cloned.reserve(list.size());
                for (auto& val : list) cloned.emplace_back(ConcreteVisitor{}.clone(val));
                return cloned;
            }
        };

        template<typename ConcreteBase, typename ConcreteChild>
        class Visitable : public ConcreteBase {
        private:
            void accept(typename ConcreteBase::VisitorType& visitor) override {
                visitor.visit(*static_cast<ConcreteChild*>(this));
            }
            void accept(typename ConcreteBase::ConstVisitorType& visitor) const override {
                visitor.visit(*static_cast<ConcreteChild const*>(this));
            }
        };

    }
    /*Visitor Base :
        Pretty Cool Class made by urs truly. For a Generic Class that should be Pure virtual
        and visitable, inerit VisitableBase with CRTP, and all the other classes that will inherit
        from it (No other easier way). VisitableBase will then have 3 type definitions:
        
        VisitorReturnType: Inherit this in your visitor that returns a value. templated return type
        VisitorType: Inherit this in you visitor that returns nothing.
        Visitable: Inherit this in the classes that derive from your generic class (using CRTP)
    */
    template<typename ConcreteBase, typename...ConcreteChildren>
    class VisitableBase 
    {
    public:
        // Different Visitor Types
        using VisitorType = detail::Visitor<ConcreteBase, false, ConcreteChildren...>;
        template<typename ReturnType>
        using VisitorReturnerType = detail::VisitorReturner<ConcreteBase, ReturnType, false, ConcreteChildren...>;
        // Const Visitor Types
        using ConstVisitorType = detail::Visitor<ConcreteBase, true, ConcreteChildren...>;
        template<typename ReturnType>
        using ConstVisitorReturnerType = detail::VisitorReturner<ConcreteBase, ReturnType, true, ConcreteChildren...>;
        // Other Visitor Types
        template<typename ConcreteVisitor>
        using CloneVisitor = detail::CloneVisitor<ConcreteVisitor, ConcreteBase, ConcreteChildren...>;

        // Visitable through VisitableBase
        template<typename ConcreteChild>
        using Visitable = detail::Visitable<ConcreteBase, ConcreteChild>;

        virtual ~VisitableBase() = default;
    private:
        template<typename, bool, typename...> friend class detail::Visitor;
        template<typename, typename, bool, typename...> friend class detail::VisitorReturner;

        virtual void accept(VisitorType& visitor) = 0;
        virtual void accept(ConstVisitorType& visitor) const = 0;
    };

}

