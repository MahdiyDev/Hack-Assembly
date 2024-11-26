# C-Instructions Reference

![C-Instructions Reference](c_instructions_reference.png "C-Instructions Reference")

## Hack Assembly Specification

Currently, assembly languages have become somewhat complex. However, we need basic concepts free of excessive abstraction. Therefore, according to the Nand2Tetris project, Hack Assembly is designed in a minimalistic form.

Hack Assembly instructions are divided into two types: **A (Address Mode)** and **C (Command Mode)**.

### **A-instruction**
- Represents either an address or a number in 15 bits.
  
### **C-instruction**
- Used for performing operations (e.g., addition, subtraction, data writing, etc.).

The distinction between A-instruction and C-instruction lies in the first bit of a 16-bit number:
- **0** for A-instruction.
- **1** for C-instruction. 

Thus, the remaining 15 bits are used to store the number in A-instruction.

**Maximum value representation of A-instruction:** `0111 1111 1111 1111`.

**Structure of a C-instruction:** `111 a c1,c2,c3,c4,c5,c6 d1,d2,d3 j1,j2,j3`.  
Although letters are used here for explanation, they are replaced with `0` or `1` in practice.

- **`a`** - Specifies the source of the ALU "y" operand during computation.
- **`c`** - Represents the computation (`comp`) operation.
- **`d`** - Indicates the destination (`dest`) register.
- **`j`** - Specifies the jump condition (`jump`).

You can observe this structure in the truth table shown above.

### Registers in Hack Assembly

Hack Assembly operates with three main registers: **D**, **A**, and **M**.

- **A**: Represents the value currently stored in the A register.
- **D**: Represents the value currently stored in the D register.
- **M**: Represents the value stored at the memory address specified by the A register (`M == RAM[A]`).

### A-instruction Usage
A-instructions are denoted using the `@` symbol.  
- Example: `@34` or via named variables like `@SP`.

### C-instruction Usage
C-instructions use the above structure to express operations, assignments (`D=A`), or jumps (`0;JMP`).

### Labels in Hack Assembly
Hack Assembly also supports **labels**, represented as `(LOOP)`.  
Labels are used to structure code and facilitate operations like loops. For instance:
- A label `(LOOP)` can be defined.
- By using `@LOOP` and `0;JMP`, operations can repeatedly execute until a condition is met.

[Hack assembly example](test.asm)

#lowlevel #asm

VM (virtual machine)

Java dasturlash tilidan virtual machine atamasini eshitgan bo'lsangiz kerak. Biz shuni soddaroq versiyasini yasaymiz.
Virtual machine - o'z syntax tiliga ega bo'lgan, stack va function'larni boshqarish imkoniyatini beruvchi tushuncha.

Virtual machine nega kerak?
Hozirgi zamonda ko'p ishlatiladigan 3 xil CPU turi mavjud (X86, X64 va ARM). Har bir CPU o'z instruction'lariga ega. High level dasturlash tilidan bu instruction'larga o'tkazib chiqish, ancha qimmatga tushadi (mehnat jihatidan). Virtual machine'ga o'tkazish esa ancha oson.

Stack - biz oldin stack haqida gaplashib o'tganmiz: [ko'ring](https://t.me/mahdiydev/121). Stack 2 xil amalni bajaradi. `push` stack'ga ma'lumot qo'shish. `pop` stack'dan oxirgi ma'lumotni olish. VM'da misol:
```
push constant 3
pop local 0
```
Stackni o'zgaruvchilar uchun ishlatadigan bo'lsak, ayan bir o'zgaruvchi olish uchun 
