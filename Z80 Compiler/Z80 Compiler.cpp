// Z80 Compiler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Compiler.h"
std::string_view test =
R"(
bin Pos:
    x : int
    y : int

export fn<T : mut type> equal_msg(var : int): 
    label: djnz a, 2
    add a, $ + 20, 45
    ;add a, (ix + a+ 0)
    adc a, b
)";

int main()
{
    Compiler compiler(test);
    auto bytes = compiler.compile();
    for (auto& byte : bytes) {
        //std::cout << std::hex << +byte << std::endl;
    }
    return 0;
}


/* TEST CASES

value1:
value2:
gtr_msg:
less_msg:
equal_msg:
    djnz a, 2 #
    add a, $ + 20, 45
    ;add a, (ix + a+ 0)
    adc a, b
_loop:
    ld a, (value1)
    ld hl, value2
    sub (hl)
    xor $80
nxor:
    ld hl, gtr_msg
less:
    ld hl, less_msg
equal:
    ld hl, equal_msg
end:
    ld hl, value2
    inc (hl)
    djnz _loop
    ret

*/