// Z80 Compiler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Parser.h"
#include "Lexer.h"
#include "ILGenerator.h"
#include "ILPrinter.h"

/* Things to do:
* Make DeepCopy a visitor class. dont know why we embedded it in the classes
* Allow Viistor class to have default
*/

const char* test =
R"(
bin<T : type> Pos:
    x : T
    y : T

bin<U : type> Rectangle:
    left : Pos<U>
    top  : Pos<U>

fn equal_msg(var : u8) -> u8: 
    let error : u16 = 5
    let arg : mut i16 = 2 + 54 * 82 / 200
    if 10:
        1
    else:
        2 + 33
    12
)";

int main()
{
    auto tokens = Lexer{ test }.generateTokens();
    auto program = Parser{ tokens }.program();
    auto il = ILGenerator{}.generate(std::move(program));
    if (!il.has_value()) return 1;
    for (auto& instr : il.value()) 
    {
        IL::Printer{}.printIL(instr);
    }

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


;;; TOUGH EXAMPLE

bin<T : type> Pos:
    x : T
    y : T

bin<U : type> Rectangle:
    left : Pos<U>
    top  : Pos<U>

fn is_square(rect : Rectangle<u8>) -> bool:
    let other = deref(0x12) as (u8, bool)->(u8)->bool
    let result = other(1, true)
    let temp : Rectangle<u8> = [{
        left:{1, 2},
        top:{0, 1}
    }, 1, 2, 3, "hello"]
    ;let list : mut u8[2][3]
    ;list[2,3] = 3
    let other : u8 = 4 + temp.top.y
    ;let ref : u8& = &other
    ;let next : u8&& = &ref

fn equal_msg(var : u8) -> u8:
    let error : u16 = 5
    let arg : mut i16 = 2 + 54 * 82 / 200
    ;let other : bool = 2 + 40 + error;
    if 10:
        1
    else: ;line 25
        2 + 33
    12
    ;return myVar

*/