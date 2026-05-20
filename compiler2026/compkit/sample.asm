;;; sample program
;;; Sum 1..100

begin:  loadi   r0, 0           ; sum
        loadi   r1, 1           ; number added to sum
        loadi   r2, ' '
        loadi   r3, '\n'

loop:   addr    r0, r1

        writed  r1
        writec  r2
        writed  r0
        writec  r3

        addi    r1, 1           ;  repeat until r1 > 100
        cmpi    r1, 100
        jle     loop
    
end:    writed  r0
        writec  r3
        halt


    
    
