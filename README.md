# EIT65-asm

en assembler för MCUn som konstrueras i kursen EIT65 (Digitalteknik) på LTH
---
## syntax

### instruktioner
---
	CALL <expr>
	RET <expr>
	BZ <expr>
	JMP <expr>
	ADD <reg>, <expr>
	SUB <reg>, <expr>
	AND <reg>, <expr>
	LD <reg>, <expr>
	IN <reg>
	OUT <reg>

---
### PAD och PUT
`PAD <val> <len>` ser till att filen blir minst `len` instruktioner lång, utfilen paddas med `val` efter instruktionerna. syntaxen `PAD <len>` er ekvivalent med `PAD 0 <len>`.

---
`PUT <val> <len>` placerar ordet `val` `len` gånger på i utfilen på den platts där funktionen andvändes. syntaxen `PUT <len>` er ekvivalent med `PAD 0 <len>`.

### labels
---
En label skapas med syntaxen:
```asm
my_label:
```
---
### Expressions
---
Det finns två typed av grunduttryck, 'Label reference' och 'Integer literal' dessa kan sedan kombineras med operatorerna `/*+-%|&~^()` och skapa uttryck.

---
#### Label reference
---
En referens till en label görs med syntaxen `$my_label`, den speciella referensen `$THIS` kan andvändas för att få addressen av instruktionen den andvänds i.

---
#### Integer literals
---
Det finns tre typer av Integer literals: decimal, hexadecimal, och binär. hexadecimala literals har prefixet `0x` och binära har prefixet `0b`.

---
#### Exempel
---
```asm
	LD R0, ~((3 + 7) * $THIS)
```

---
### Register
---
Register specificeras med antingen `R0` eller `R1`

---
### Kommentarer
---
för att kommentera ut resten av en rad andvänd `;`

---
## Ett litet exempel
```asm
start:
	IN R0
	AND R0, 1 << 2	; kolla om bit 2 i R0 är satt
	BZ $THIS + 4	; hoppa fram 3 instruktioner
	LD R0, 0xF + 4  ; om bit 2 är satt ge 20 i output
	OUT R0
	JMP $main
	LD R0, 0x6 + 4  ; om bit 2 är 0 ge 10 i output
	OUT R0
	JMP $THIS	; oändlig loop :)
```
