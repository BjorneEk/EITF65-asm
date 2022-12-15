


	LD R0, 10
check:
	SUB R0, 1 	; Else, subtract one from R0
	; code here executed 10 times, R0:  9->0
	BZ $end 	;If R0 == 0, goto End
	JMP $check ; goto #Check
end:
	JMP $end
