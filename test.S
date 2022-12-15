; huvudgata_sidogata
start:
	LD R0, 0b00001100
	OUT R0
main:
	IN R0
	AND R0, 0b00000001
	BZ $cycle
	IN R0
	AND R0, 0b00000010
	BZ $cycle

cycle:
	LD R0, 0b00001010
	OUT R0
	LD R0, 250
	CALL $delay
	LD R0, 0b00001001
	OUT R0
	LD R0, 250
	CALL $delay
	LD R0, 0b00011001
	OUT R0
	LD R0, 250
	CALL $delay
	LD R0, 0b00100001
	OUT R0
	LD R0, 250
	CALL $delay
	LD R0, 0b00010001
	OUT R0
	LD R0, 250
	CALL $delay
	LD R0, 0b00001001
	OUT R0
	LD R0, 250
	CALL $delay
	LD R0, 0b00001011
	OUT R0
	LD R0, 250
	CALL $delay
	LD R0, 0b00001100
	OUT R0
	LD R0, 250
	CALL $delay
night:

; @param R0: delay ungefär R0*4 cykler ca 4ms
; båda registers inehåll går förlorat
delay:
	IN R1
	SUB R0, 1
	BZ $delay_ret
	JMP $delay
delay_ret:
	RET

PAD 64
