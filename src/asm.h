/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 *
 * a assembler built for the MCU constructed in the course
 * EIT65 at Lunds tekniska högskola
 *
 *==========================================================*/

#ifndef _EIT65_ASM_H_
#define _EIT65_ASM_H_
#include "types.h"
#include <stdio.h>
#define PACKED        __attribute__((__packed__))


#define INSTR_NAMES(X)	\
	X(CALL)		\
	X(RET)		\
	X(BZ)		\
	X(B)		\
	X(ADD)		\
	X(SUB)		\
	X(LD)		\
	X(IN)		\
	X(OUT)		\
	X(AND)

/**
 *	these values should be changed to the ones selected
 *	to represent the 12:9 pits of the instruction
 **/
enum {
	CALL = 0b0110,
	RET  = 0b0100,
	BZ   = 0b0010,
	JMP  = 0b1100,
	ADD  = 0b0011,
	SUB  = 0b1111,
	LD   = 0b0101,
	IN   = 0b0111,
	OUT  = 0b1010,
	AND  = 0b0000
};

enum {
	REG_R0 = 0,
	REG_R1 = 1
};





#define TOKEN_TYPES(X)	\
	X(CALL)		\
	X(RET)		\
	X(BZ)		\
	X(JMP)		\
	X(ADD)		\
	X(SUB)		\
	X(LD)		\
	X(IN)		\
	X(OUT)		\
	X(AND)		\
	X(LBL)		\
	X(LBL_REF)	\
	X(INTLIT)	\
	X(REG_R0)	\
	X(REG_R1)	\
	X(PAD)		\
	X(PUT)		\
	X(END)


#define TTYPE_NAME(name) TK_##name,
enum ttype {
	TK_CALL = CALL,
	TK_RET  = RET,
	TK_BZ   = BZ,
	TK_JMP  = JMP,
	TK_ADD  = ADD,
	TK_SUB  = SUB,
	TK_LD   = LD,
	TK_IN   = IN,
	TK_OUT  = OUT,
	TK_AND  = AND,
	TK_LBL =0b10000,
	TK_LBL_REF,
	TK_INTLIT,
	TK_REG_R0,
	TK_REG_R1,
	TK_PAD,
	TK_PUT,
	TK_END
};
#undef TTYPE_NAME

typedef struct token {
	union {
		i64_t int_val;
		char *str_val;
	};
	i32_t type;

	/* debug information */
	i32_t line;
	i32_t col;
	u32_t addr;
} tok_t;

typedef struct label {
	char *ident;
	u8_t addr;
} lbl_t;

typedef struct ins {
	struct PACKED {
		u8_t DATA   : 8;
		u8_t DST    : 1;
		u8_t INS    : 4;
		u8_t UNUSED : 3;
	};
	tok_t tok;
} ins_t;

enum {
	FMT_BIN = 0,
	FMT_HEX = 1
};

void assemble(const char *src, const char *dst, i32_t fmt, i32_t verbosity);

#endif /* _EIT65_ASM_H_ */
