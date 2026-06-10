jmp L1
L0:
load r0, 0
cmpi r0, 1
jnz L2
load r0, 2
writed r0
loadi r1, 32
writec r1
load r0, 3
writed r0
loadi r1, 10
writec r1
jmp L3
L2:
load r0, 2
push r0
load r0, 4
push r0
load r0, 3
push r0
load r0, 0
store r0, 6
loadi r0, 1
load r1, 6
subr r1, r0
store r1, 6
load r0, 6
push r0
call L0
addi sp, 4
load r0, 2
writed r0
loadi r1, 32
writec r1
load r0, 3
writed r0
loadi r1, 10
writec r1
load r0, 4
push r0
load r0, 3
push r0
load r0, 2
push r0
load r0, 0
store r0, 7
loadi r0, 1
load r1, 7
subr r1, r0
store r1, 7
load r0, 7
push r0
call L0
addi sp, 4
L3:
halt
L1:
loadi r0, 5
store r0, 0
loadi r0, 1
push r0
loadi r0, 2
push r0
loadi r0, 3
push r0
load r0, 0
push r0
call L0
addi sp, 4
halt
