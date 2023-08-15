#pragma once
#include <cassert>
#include "Stmt.h"
#include "TemplateReplacer.h"

class TemplateSystem
{
	template<typename...TempArgs>
	auto instantiateTemplate(std::string_view name, TempArgs...tempArgs) -> Stmt::Function {

	}
	template<typename...TempArgs>
	auto instantiate(std::string_view name, TempArgs...tempArgs) -> Stmt::Function {

	}

	void addTemplate(Stmt::Bin bin) { 
		templateBins.emplace_back(std::move(bin));
	}
	void addTemplate(Stmt::Function function) {
		templateFunctions.emplace_back(std::move(function));
	}

private:
	
	std::vector<Stmt::Function> templateFunctions, compiledFunctions;
	std::vector<Stmt::Bin> templateBins, compiledBins;
};

