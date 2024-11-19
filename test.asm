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

// push local 3
@LCL
D=M
@3
A=D+A
D=M
@SP
M=M+1
A=M-1
M=D
// pop local 3
@LCL
D=M
@3
D=D+A
@15
M=D
@SP
AM=M-1
D=M
@15
A=M
M=D
// push constant 4
@4
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
D=M+D
@15
M=D
@15
D=A
@SP
M=M+1
A=M-1
M=D

@INIT
0;JMP
