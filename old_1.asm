MOVC R1,#10
MOVC R5,#45
MOVC R10,#230
ADD R6,R10,R5
MOVC R13,#20
SUB R12,R13,R6
SUBL R12,R6,#10
STORE R10,R1,#22
ADDL R15,R12,#15
MOVC R0,#12
LOAD R14,R13,#12
STR R13,R13,R1
MUL R10,R0,R13
AND R1,R10,R14
OR R2,R1,R15
EXOR R10,R12,R13
LDR R6,R13,R0
HALT
AND R13,R6,R5
SUB R9,R0,R14