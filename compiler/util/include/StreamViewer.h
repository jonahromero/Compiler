#pragma once
#include <string>
#include <optional>

//there are some possible errors that have to do with 
//using peek and peekNext, and not always checking at end
template<typename T>
class StreamViewer {
protected:

	struct View { const T* data; size_t size; };
	StreamViewer(const T* data, size_t size) : view{ data, size } {}

	virtual bool atEnd() const { return current == view.size; }
	T const& advance() { return view.data[current++]; }

#ifndef _DEBUG
	T const& peek() const { return view.data[current]; }
	T const& peekNext() const { return view.data[current + 1]; }
	T const& peekPrevious() const { return view.data[current - 1]; }
#else
	T const& peek() const { if (StreamViewer<T>::atEnd()) throw; return view.data[current]; }
	T const& peekNext() const { if (StreamViewer<T>::atEnd()) throw; return view.data[current + 1]; }
	T const& peekPrevious() const { if (current == 0) throw; return view.data[current - 1]; }
#endif

	bool match(std::same_as<T> auto...targets) {
		if (atEnd()) return false;
		if (auto l = peek(); ((l == targets) || ...)) {
			advance();
			return true;
		}
		return false;
	}

	bool matchConsecutive(std::same_as<T> auto...targets) 
	{
		size_t i = 0;
		bool matched = ((current != view.size 
						&& view.data[current + (i++)] == targets) 
						&& ...);
		if(matched) {
			current += sizeof...(targets);
		}
		return matched;
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
