#pragma once
#include <string>
#include <optional>
#include "ExpectationErrors.h"

template<typename T>
class StreamViewer {
protected:
	class ExpectedElement
		: public ExpectedError<T> {
		using ExpectedError<T>::ExpectedError;
	};
	class UnexpectedElement
		: public UnexpectedError<T> {
		using UnexpectedError<T>::UnexpectedError;
	};

	struct View { const T* data; size_t size; };
	StreamViewer(const T* data, size_t size) : view{ data, size } {}

	virtual bool atEnd() { return current == view.size; }
	T const& peek() { return view.data[current]; }
	T const& peekNext() { return view.data[current + 1]; }
	T const& peekPrevious() { return view.data[current - 1]; }
	T const& advance() { return view.data[current++]; }

	void expectedElement(T const& expected) {
		throw ExpectedError(expected, atEnd() ? std::nullopt : std::optional(peek()));
	}
	void unexpectedElement(T const& unexpected) {
		throw UnexpectedElement(unexpected);
	}
	void expect(T const& expected) {
		if (atEnd() || expected != peek()) {
			expectedElement(expected);
		}
		else {
			advance();
		}
	}
	bool match(std::same_as<T> auto...targets) {
		if (auto l = peek(); ((l == targets) || ...)) {
			advance();
			return true;
		}
		return false;
	}
	void consumeWhile(auto callback) {
		while (!atEnd() && callback(peek())) {
			advance();
		}
	}

	View currentView() { return View{ view.data + start, current - start }; }
	size_t currentPos() { return start; }
	void readjustStart() { start = current; }
	virtual void reset() { start = current = 0; }

	virtual ~StreamViewer() {}
private:
	View view;
	size_t start = 0, current = 0;
};
