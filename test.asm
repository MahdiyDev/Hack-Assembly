(INIT)
@256
D=A
@SP
M=D

@300
D=A
@LCL
M=D

@400
D=A
@ARG
M=D

@3000
D=A
@THIS
M=D

@3010
D=A
@THAT
M=D

// push constant 10
@10
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 7
@7
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 8
@8
D=A
@SP
M=M+1
A=M-1
M=D
// add
@SP
AM=M-1
D=M
@13
M=D
@SP
AM=M-1
D=M
@14
M=D
@13
D=M
@14
D=D+M
@15
M=D
@15
D=M
@SP
M=M+1
A=M-1
M=D
// add
@SP
AM=M-1
D=M
@13
M=D
@SP
AM=M-1
D=M
@14
M=D
@13
D=M
@14
D=D+M
@15
M=D
@15
D=M
@SP
M=M+1
A=M-1
M=D
// pop local 0
@LCL
D=M
@0
D=D+A
@15
M=D
@SP
AM=M-1
D=M
@15
A=M
M=D
// call math 0
@math$return.7
D=A
@SP
M=M+1
A=M-1
M=D
@R1
D=M
@0
A=D+A
D=M
@SP
M=M+1
A=M-1
M=D
@R2
D=M
@0
A=D+A
D=M
@SP
M=M+1
A=M-1
M=D
@R3
D=M
@0
A=D+A
D=M
@SP
M=M+1
A=M-1
M=D
@R4
D=M
@0
A=D+A
D=M
@SP
M=M+1
A=M-1
M=D
@SP
D=M
@5
D=D-A
@ARG
M=D
@SP
D=M
@0
D=D+A
@LCL
M=D
@math
0;JMP
(math$return.7)
// push constant 8
@8
D=A
@SP
M=M+1
A=M-1
M=D
// function math 2
(math)
@0
D=A
@SP
M=M+1
A=M-1
M=D
@0
D=A
@SP
M=M+1
A=M-1
M=D
// return
@LCL
D=M
@R14
M=D
@R14
D=M
@5
D=D-A
@R15
M=D
@ARG
D=M
@0
D=D+A
@15
M=D
@SP
AM=M-1
D=M
@15
A=M
M=D
@ARG
D=M
@1
D=D+A
@SP
M=D
@R14
D=M
@1
A=D-A
D=M
@THAT
M=D
@R14
D=M
@2
A=D-A
D=M
@THIS
M=D
@R14
D=M
@3
A=D-A
D=M
@ARG
M=D
@R14
D=M
@4
A=D-A
D=M
@LCL
M=D
@R15
0;JMP
// add
@SP
AM=M-1
D=M
@13
M=D
@SP
AM=M-1
D=M
@14
M=D
@13
D=M
@14
D=D+M
@15
M=D
@15
D=M
@SP
M=M+1
A=M-1
M=D

@INIT
0;JMP
