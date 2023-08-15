
#pragma once
#include <unordered_set>
#include <string>
#include <stack>

class ParserContext 
{
public:
	template<typename T> struct ScopedChange {
		ScopedChange(T& ref, T newValue) : ref(ref) {
			oldValue = ref;
			ref = newValue;
		}
		~ScopedChange() {
			ref = oldValue;
		}
	private:
		T oldValue;
		T& ref;
	};

	template<typename T>
	struct ScopedChange<std::stack<T>> 
	{
		ScopedChange(std::stack<T>& stack, T newValue) : stack(stack) {
			stack.push(newValue);
		}
		~ScopedChange() {
			stack.pop();
		}
	private:
		std::stack<T>& stack;
	};

	void addTemplate(std::string_view name) { templates.insert(name); }
	bool isTemplate(std::string_view name) const { return templates.count(name) != 0; }
	bool isInTemplateMode() const { return templateMode;  }
	bool isInTypeMode() const { return typeMode; }
	bool isNested() const { return expr.nestingLevel.top() != 0; }


	[[nodiscard]] ScopedChange<size_t> increaseNesting() noexcept {
		return ScopedChange<size_t>(expr.nestingLevel.top(), expr.nestingLevel.top() + 1);
	}
	[[nodiscard]] ScopedChange<std::stack<size_t>> startNewNesting() noexcept {
		return ScopedChange<std::stack<size_t>>(expr.nestingLevel, 0);
	}
	[[nodiscard]] ScopedChange<bool> turnOffTypeMode() noexcept {
		return ScopedChange<bool>(typeMode, true);
	}
	[[nodiscard]] ScopedChange<bool> turnOnTypeMode() noexcept {
		return ScopedChange<bool>(typeMode, true);
	}
	[[nodiscard]] ScopedChange<bool> turnOnTemplateMode() noexcept {
		return ScopedChange<bool>(templateMode, true);
	}
	[[nodiscard]] ScopedChange<bool> turnOffTemplateMode() noexcept {
		return ScopedChange<bool>(templateMode, true);
	}

private:
	std::unordered_set<std::string_view> templates;
	bool templateMode = false, typeMode = false;
	struct 
	{
		std::stack<size_t> nestingLevel;
	} expr;
};