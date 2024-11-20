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
D=A
@SP
M=M+1
A=M-1
M=D

@INIT
0;JMP
