#pragma once
#include <vector>

template<typename First, typename Second>
class ArrayMap
{
public:
	template<typename T, typename U>
	void insert(T&& first, U&& second) {
		auto it = getIterator(first);
		if (it == firstList.end()) {
			firstList.emplace_back(std::forward<T>(first));
			secondList.emplace_back(std::forward<U>(second));
		}
		else {
			secondList[std::distance(firstList.cbegin(), it)] = std::forward<U>(second);
		}
	}
	Second const& get(First const& first) const {
		if (lastIndex != -1 && first == firstList[lastIndex]) {
			return secondList[lastIndex];
		}
		else {
			return *getIterator(first);
		}
	}
	Second& get(First const& first) {
		return const_cast<Second&>(static_cast<const decltype(this)>(this)->get(first));
	}
	bool has(First const& first) const {
		if (auto it = getIterator(first); it != firstList.end()) {
			lastIndex = std::distance(firstList.cbegin(), it);
			return true;
		}
		else {
			lastIndex = -1;
			return false;
		}
	}
	void operator+=(ArrayMap const& other) {
		firstList.insert(firstList.end(), other.firstList.begin(), other.firstList.end());
		secondList.insert(secondList.end(), other.secondList.begin(), other.secondList.end());
	}

private:
	auto getIterator(First const& first) const {
		return std::find_if(firstList.begin(), firstList.end(), [&first](auto const& firstEntry) {
			return first == firstEntry;
		});
	}

	std::vector<First> firstList;
	std::vector<Second> secondList;
	mutable int lastIndex = -1;
};