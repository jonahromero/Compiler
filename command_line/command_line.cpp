// Z80 Compiler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

/* Things to do:
* Make DeepCopy a visitor class. dont know why we embedded it in the classes
* Allow Viistor class to have default
*/

const char* test =
R"(
bin Pos:
    x : u8
    y : u8

fn equal_msg(var : u8) -> u8: ;line 5 
    let arg : i16 = 2
    arg = 5
    if arg == 2:
        arg = 1
    else:
        2 + 33
    arg = arg + 12
    ;return myVar
)";

int main()
{
    /*
    Compiler compiler;
    auto il_program = compiler.compile(test);
    for (auto& il : il_program) {
        ILPrinter{}.printIL(il);
        //std::cout << std::hex << +byte << std::endl;
    }*/
    return 0;
}


/* TEST CASES
bin Pos:
    x : u8
    y : u8

fn equal_msg(var : u8) -> u8: ;line 5
    if 1 == 2:
        if 2:
            let myVar : u8 = (1 + 2 * 20 / !500)
        else:
            let poop : u8 = 3
        let something : i8 = 45
        here:
        let Rawr : i8 = 12 * 3
        end:
    else if 1 == 3:
        1 + 2
    else:
        let ARg : u8 = 2
    ;return myVar

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