
start:
	LD R1, 0
empty:
	IN R0
	AND R0, 1 << 2
	BZ $THIS + 3
	ADD R1, 1
	JMP $full
_empty:
	IN R0
	AND R0, 1 << 3
	BZ $empty
full:
	IN R0
	AND R0, 1 << 2
	BZ $_full
	SUB R1, 1
	JMP $empty
_full:
	IN R0
	AND R0, 1 << 3
	BZ $full
	JMP $empty

PAD 0x00, 8*8
; equivalent to PAD 8*8
