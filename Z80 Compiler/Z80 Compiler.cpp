// Z80 Compiler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Compiler.h"
const char* test =
R"(
bin Pos:
    x : int
    y : int

fn equal_msg(var : int) -> u8: ;line 5 
    count with apple from 9: 
        add a, 3
    label: djnz a, 2    ;line 8
    if print("20"):
        jojo = fifo
    add a, $ + 20, 45   ;line 11
    ;add a, (ix + a+ 0)
    adc a, b


)";

int main()
{
    Compiler compiler;
    auto bytes = compiler.compile(test);
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