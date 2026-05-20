loadi r0, 1
store r0, 0
loadi r0, 100
store r0, 6
L0:
load r0, 0
load r1, 6
cmpr r0, r1
jgt L1
loadi r0, 1
store r0, 1
loadi r0, 0
store r0, 2
L2:
load r0, 1
load r1, 0
cmpr r0, r1
jgt L3
load r0, 0
load r1, 1
divr r0, r1
store r0, 3
load r0, 3
load r1, 1
mulr r0, r1
store r0, 5
load r0, 0
load r1, 5
subr r0, r1
store r0, 4
load r0, 4
cmpi r0, 0
jnz L4
load r0, 2
addi r0, 1
store r0, 2
L4:
load r0, 1
addi r0, 1
store r0, 1
jmp L2
L3:
load r0, 2
cmpi r0, 2
jnz L6
load r0, 0
writed r0
loadi r1, 10
writec r1
L6:
load r0, 0
addi r0, 1
store r0, 0
jmp L0
L1:
halt
