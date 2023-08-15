#pragma once
#include <array> 
#include "StringUtil.h"
#include "Token.h"

inline auto getKeyword(std::string_view keyword) {
	using enum Token::Type;
	static std::array<std::pair<std::string_view, Token::Type>, 17> keywords = { {
		{"fn", FN}, {"bin", BIN}, {"let", LET}, {"if", IF}, {"else", ELSE}, {"type", TYPE},
		{"count", COUNT}, {"with", WITH}, {"from", FROM}, {"mut", MUT}, {"true", TRUE}, {"false", FALSE},
		{"return", RETURN}, {"and", AND}, {"or", OR}, {"module", MODULE}, {"export", EXPORT}
	} };
	auto it = std::find_if(keywords.begin(), keywords.end(), [keyword](auto pair) {
		return keyword == pair.first;
	});
	bool wasFound = it != keywords.end();
	return std::make_tuple(wasFound, wasFound ? it->second : EOF_);
}

inline bool isOpcode(std::string_view opcode) {
	static std::array<std::string_view, 64> opcodes = {
		"ld","push","pop","ex","ldi","ldir","ldd","lddr","cpi","cpir","cpd","cpdr","add","adc","sub","abc","and","or",
		"xor","cp","inc","dec","daa","cpl","neg","ccf","scf","nop","halt","di","ei","im","rcla","rla","rrca","rra","rlc",
		"rl","rrc","rr","sla","sra","srl","rld","rrd","bit","set","res","jr","jp","djnz","call","ret","reti","retn","rst",
		"in","ini","inir","out","outi","otir","outd","otdr"
	};
	auto it = std::find(opcodes.begin(), opcodes.end(), toLower(opcode));
	return it != opcodes.end();
}

inline bool isRegisterAF(std::string_view reg) {
	return toLower(reg) == "af";
}

inline bool isRegister(std::string_view reg) {
	static std::array<std::string_view, 17> registers = {
		"a", "b", "c", "d", "e", "h", "l", "ix", "iy", "sp", "pc",
		"af", "bc", "de", "hl", "i", "r"
	};
	auto it = std::find(registers.begin(), registers.end(), toLower(reg));
	return it != registers.end();
}

inline bool isFlag(std::string_view flag) {
	static std::array<std::string_view, 8> flags = {
		"p", "m", "z", "nz", "c", "nc", "po", "pe"
	};
	auto it = std::find(flags.begin(), flags.end(), toLower(flag));
	return it != flags.end();
}