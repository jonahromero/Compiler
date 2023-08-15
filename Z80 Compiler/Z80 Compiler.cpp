// Z80 Compiler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Compiler.h"
std::string_view test =
R"(
    djnz 10
    add a, $ + 20
    ;add a, (ix + 0)
    adc a, b
_loop:
    ld a, (value1)
    ld hl, value2
    sub (hl)
    jr z, equal
    jp po, nxor
    xor $80
nxor:
    jp m, less 
    ld hl, gtr_msg
    jr end
less:
    ld hl, less_msg
    jr end
equal:
    ld hl, equal_msg
end:
    ld hl, value2
    inc (hl)
    djnz _loop
    ret

value1:
value2:
gtr_msg:
less_msg: 
equal_msg: 
)";

int main()
{
    Compiler compiler(test);
    auto bytes = compiler.compile();
    for (auto& byte : bytes) {
        std::cout << std::hex << +byte << std::endl;
    }
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
