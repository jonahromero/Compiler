#pragma once
#include <string>

struct Templated {
	std::vector<class Type*> types;
};

struct Type {
	size_t size;
	std::string_view name;
	bool isMut;

	std::optional<Templated> templated;
};