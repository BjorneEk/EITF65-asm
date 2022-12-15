/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 *
 * a assembler built for the MCU constructed in the course
 * EIT65 at Lunds tekniska högskola
 *
 *==========================================================*/


#include "asm.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>



#define ERR_UNEXP "unexpected token"
#define ERR_UNCLOSED_LIT "unclosed string literal"
#define ERR_INVALID_LBL "Unexpected token, invalid label"
#define ERR_LBL  "[\033[31;1;4mError\033[0m]"
#define FAILED_LBL  "[\033[31;1;4mCompilation failed\033[0m]"

#define MAX_LBL 4096

#define TOKEN_INIT(_ln, _col)	(tok_t){.line = (_ln), .col = (_col)}

#define TK_INS(_tk, _int)	((_tk).type = (_int),		(_tk).int_val = 0,      _tk)
#define TK_INT(_tk, _i)	((_tk).type = TK_INTLIT,		(_tk).int_val = (_i),   _tk)
#define TK_IDENT(_tk, _str)	((_tk).type = TK_LBL,		(_tk).str_val = (_str), _tk)
#define TK_REF(_tk, _str)	((_tk).type = TK_LBL_REF,	(_tk).str_val = (_str), _tk)

#define NEW_TK() TOKEN_INIT(ctx.line, ctx.col)

static struct ctx {
	FILE *f;
	i32_t line;
	i32_t col;
	u32_t addr;
} ctx;


const char *token_str(enum ttype t);
__attribute__((noreturn))
void error(const char *error)
{
	fprintf(stderr,FAILED_LBL " %s\n",error);
	exit(-1);
}



void log_parse_error(tok_t t, const char * fmt, va_list args)
{
        fprintf(stderr, ERR_LBL ": line: %i:%i ", t.line+1, t.col+1);
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
}
__attribute__((noreturn))
void parse_error(tok_t t, const char * fmt, ...)
{
        va_list args;

        va_start(args, fmt);
        log_parse_error(t, fmt, args);
        error("error while parsing");
}

void tk_error(struct token t, const char * message, const char * parsed, i32_t len)
{
	char buff[len+1];
	strncpy(buff, parsed, len);
	buff[len] = '\0';
	fprintf(stderr, ERR_LBL ": line: %i:%i: %s: %s\n", t.line+1, t.col+1, message, buff);
	error("error generating tokens");
}

bool islblbegin(i32_t c)
{
        return (isalnum(c) || c == '_') && c != ';';
}

bool islblchr(i32_t c)
{
        return (isalnum(c) || c == '_') && c != ';';
}

i32_t next()
{
        ctx.col++;
        return getc(ctx.f);
}

/* pushback to buffer */
void pb(i32_t c)
{
        ctx.col--;
        ungetc(c, ctx.f);
}


char * read_ident(const char * pref)
{
	i32_t i;
	char buff[MAX_LBL], *res;
	i = 0;

	if(pref == NULL && !islblbegin(buff[i++] = next()))
		return NULL;

	if(pref != NULL) {
		i = strlen(pref);
		strncpy(buff, pref, i);
	}
	while(i < MAX_LBL && islblchr(buff[i++] = next()))
		;
	pb(buff[i-1]);
	buff[i-1] = '\0';

	if(strlen(buff) == 0)
		return NULL;

	res = malloc((i+1) * sizeof(char));

	if(!res)
		error("Allocating memory");

	strcpy(res, buff);
	return res;
}

static u32_t hex(char c)
{
        if ('0' <= c && c <= '9')
                return c - '0';
        if ('a' <= c && c <= 'f')
                return c - 'a' + 10;
        if ('A' <= c && c <= 'F')
                return c - 'A' + 10;
        error("error reading hexadecimal in literal");
}

u64_t read_hex()
{
	u64_t res;
	i32_t c;
	res = 0;
	while(isxdigit(c = next()))
		res = res * 16 + hex(c);
	pb(c);
	return res;
}


u64_t read_bin()
{
	u64_t res;
	i32_t c;

	res = 0;
	while(isdigit(c = next()) && (c == '1' || c == '0'))
		res = res * 2 + c - '0';
	pb(c);
	return res;
}

u64_t read_int()
{
	u64_t res;
	i32_t c;

	res = 0;
	while(isdigit(c = next()))
		res = res * 10 + c - '0';
	pb(c);
	return res;
}

#define RETIFELS(retval, cmpto, elslbl) do{		\
	char *str = cmpto;				\
	i32_t idx;					\
	for(idx = 1; str[idx] != '\0'; idx++)		\
		if((buff[++i] = next()) != str[idx]) 	\
			goto elslbl;			\
	return retval;}while(0);


tok_t next_tok()
{
	struct token tk;
	i32_t i;
	u64_t ival;
	char buff[MAX_LBL], *tmp;

	tk = NEW_TK();

	buff[i = 0] = next();
	//printf("first: %c\n", buff[i]);
	//fflush(stdout);
	switch(buff[i]) {
		case 'C': 	RETIFELS(TK_INS(tk, TK_CALL),	"CALL",	labl);
		case 'R': 	RETIFELS(TK_INS(tk, TK_RET),	"RET",	labl);
		case 'B': 	RETIFELS(TK_INS(tk, TK_BZ),	"BZ",	labl);
		case 'J': 	RETIFELS(TK_INS(tk, TK_JMP),	"JMP",	labl);
		case 'A': 	RETIFELS(TK_INS(tk, TK_ADD),	"ADD",	and);
		and:		RETIFELS(TK_INS(tk, TK_AND),	"AND",	labl);
		case 'S': 	RETIFELS(TK_INS(tk, TK_SUB),	"SUB",	labl);
		case 'L': 	RETIFELS(TK_INS(tk, TK_LD),	"LD",	labl);
		case 'I':	RETIFELS(TK_INS(tk, TK_IN),	"IN",	labl);
		case 'O':	RETIFELS(TK_INS(tk, TK_OUT),	"OUT",	labl);
		case '$':
			tmp = read_ident(NULL);
			if(tmp == NULL)
				tk_error(tk, ERR_INVALID_LBL, buff, i+1);
			return TK_REF(tk, tmp);
		case '0':
			if((buff[++i] = next()) == 'x' || buff[i] == 'X')
				ival = read_hex();
			else if(buff[i] == 'b' || buff[i] == 'B')
				ival = read_bin();
			else
				goto intlit;
			return TK_INT(tk, ival);
		case ',': return TK_INS(tk, ',');
		case EOF:
			return TK_INS(tk, TK_END);
		default: //LBL, INTLIT
			if(!isdigit(buff[i]))
				goto labl;
		intlit:
			pb(buff[i]);
			ival = read_int();
			return TK_INT(tk, ival);
		labl:
			buff[i+1] = '\0';
			tmp = read_ident(buff);
			//printf("read label: %s\n", tmp);
			if(tmp == NULL)
				tk_error(tk, ERR_INVALID_LBL, buff, i+1);
			if(!strcmp(tmp, "R0"))
				return TK_INS(tk, TK_REG_R0);
			else if(!strcmp(tmp, "R1"))
				return TK_INS(tk, TK_REG_R1);
			else if((buff[++i] = next()) != ':')
				tk_error(tk, "expected ':' after label", buff, i+1);
			return TK_IDENT(tk, tmp);
	}
}

bool is_spce(i32_t c)
{
        return c == ' ' || c == '\t';
}
bool is_wsc(i32_t c)
{
        return c == ' '
                || c == '\t'
                || c == '\r'
                || c == '\f'
                || c == '\n';
}

bool consume_wsc()
{
	i32_t c;

	c = getc(ctx.f);

	if (c == '\n') {
		ctx.line++;
		ctx.col = 0;
		return true;
	} else if (is_wsc(c)) {
		ctx.col++;
		return true;
	} else if(c == ';') {
		while((c = getc(ctx.f)) != '\n')
			;
		ctx.line++;
		ctx.col = 0;
		ungetc(c, ctx.f);
		return true;
	}
	ungetc(c, ctx.f);
	return false;
}

bool consume_spc()
{
	i32_t c;

	c = getc(ctx.f);

	if (is_spce(c)) {
		ctx.col++;
		return true;
	}
	ungetc(c, ctx.f);
	return false;
}
void consume_whitespaces()
{
	while (consume_wsc())
		;
}
void consume_spaces()
{
	while (consume_spc())
		;
}

bool is_instr(tok_t t)
{
	return
		t.type == TK_CALL	||
		t.type == TK_RET	||
		t.type == TK_BZ		||
		t.type == TK_JMP	||
		t.type == TK_ADD	||
		t.type == TK_SUB	||
		t.type == TK_LD		||
		t.type == TK_IN		||
		t.type == TK_OUT	||
		t.type == TK_AND;
}

lbl_t new_lbl(char *ident, u32_t addr)
{
	lbl_t res;
	res.addr  = addr;
	res.ident = ident;
	return res;
}



i32_t cmp_lbl(const void *a, const void *b)
{
	return strcmp(((lbl_t*)a)->ident, (char*)b);
}

i32_t lbl_idx(lbl_t *lbls, u32_t cnt, const char *id)
{
	i32_t i;

	for(i = 0; i < cnt; i++)
		if(!strcmp(lbls[i].ident,id))
			return i;
	return -1;
}
u32_t expect_addr(lbl_t *lbls, u32_t cnt)
{
	tok_t tk;
	u32_t i;
	lbl_t res;
	tk = next_tok();
	if(tk.type != TK_LBL_REF && tk.type != TK_INTLIT) {
		parse_error(tk, "expected label reference or literal address, found: '%s'",
		token_str(tk.type));
	}
	if(tk.type == TK_INTLIT)
		return tk.int_val;
	else if ((i = lbl_idx(lbls, cnt, tk.str_val)) == -1)
		parse_error(tk, "use of undeclared identifier: '%s'", tk.str_val);
	free(tk.str_val);
	return lbls[i].addr;
}
u32_t expect_int()
{
	tok_t tk;
	tk = next_tok();
	if(tk.type != TK_INTLIT) {
		parse_error(tk, "expected integer literal, found: '%s'",
		token_str(tk.type));
	}
	return tk.int_val;

}
u32_t expect_register()
{
	tok_t tk;

	tk = next_tok();
	if (tk.type != TK_REG_R0 && tk.type != TK_REG_R1)
		parse_error(tk, "expected register R0 or R1, found: '%s'",
		token_str(tk.type));
	else if(tk.type == TK_REG_R0)
		return REG_R0;
	else
		return REG_R1;
}

void expect_comma()
{
	tok_t tk;

	tk = next_tok();
	if (tk.type != ',')
		parse_error(tk, "expected comma, found: '%s'",
		token_str(tk.type));
	return;
}

void printbits(int n)
{
	u32_t i, step;

	if (0 == n) {
		fputs("0000", stdout);
		return;
	}

	i = 1<<(sizeof(n) * 8 - 1);

	step = -1; /* Only print the relevant digits */
	step >>= 4; /* In groups of 4 */
	while (step >= n) {
		i >>= 4;
		step >>= 4;
	}

	/* At this point, i is the smallest power of two larger or equal to n */
	while (i > 0) {
		if (n & i)
			putchar('1');
		else
			putchar('0');
		i >>= 1;
	}
}

lbl_t *get_labels(FILE *f, u32_t *lbl_cnt, bool debug)
{
	lbl_t buff[100];
	lbl_t *res;
	i32_t i, sz;
	tok_t tk;

	i = 0;
	ctx = (struct ctx){.f = f, .line = 0, .col = 0, .addr=-1};
	do {
		consume_whitespaces();
		tk = next_tok();
		if(is_instr(tk))
			ctx.addr++;
		tk.addr = ctx.addr;
		if(tk.type == TK_LBL) {
			if(debug)
				printf("lbl: %s:0x%x\n",tk.str_val, tk.addr+1);
			buff[i++] = new_lbl(tk.str_val, tk.addr+1);
			if(i >= 100) {
				res = realloc(res, (sz + i) * sizeof(lbl_t));
				memcpy(res, buff, i * sizeof(lbl_t));
				sz += i;
				i = 0;
			}
		} else if(tk.type == TK_LBL_REF) {
			free(tk.str_val);
		}
	} while(tk.type != TK_END);

	res = realloc(res, (sz + i) * sizeof(lbl_t));
	memcpy(res, buff, i * sizeof(lbl_t));
	sz += i;
	*lbl_cnt = sz;
	return res;
}

void assemble(FILE *f, lbl_t *lbls, u32_t lbl_cnt, const char *dst, bool debug, bool hex)
{
	FILE *out;
	tok_t tk;
	ins_t instr;
	u32_t i, cnt = 0;

	out = fopen(dst, "w");
	if (!out) {
		fprintf(stderr, ERR_LBL ": could not open file: '%s' for writing\n", dst);
		exit(-1);
	}
	ctx = (struct ctx){.f = f, .line = 0, .col = 0, .addr=-1};
	i = 0;
	do {
		consume_whitespaces();

		tk = next_tok();

		if(!is_instr(tk))
			continue;

		ctx.addr++;
		tk.addr = ctx.addr;
		//print_tok(tk);

		instr.INS = tk.type;
		instr.tok = tk;
		switch(tk.type) {
			case TK_CALL:
			case TK_BZ:
			case TK_JMP:
				consume_spaces();
				instr.DATA = expect_addr(lbls, lbl_cnt);
				instr.DST = 0;
				break;
			case TK_RET: break;
			case TK_SUB:
			case TK_ADD:
			case TK_AND:
			case TK_LD:
				consume_spaces();
				instr.DST = expect_register();
				consume_spaces();
				expect_comma();
				consume_spaces();
				instr.DATA = expect_int();
				break;
			case TK_IN:
			case TK_OUT:
				consume_spaces();
				instr.DST = expect_register();
			case TK_LBL:
				free(tk.str_val);
			default:
				printf("ERROR\n");
				break;
		}
		if(hex) {
			fprintf(out, "%02X%02X;\n",(instr.INS << 1) | instr.DST, instr.DATA);
		} else {
			fputc((instr.INS << 1) | instr.DST, out);
			fputc(instr.DATA, out);
		}
		if (debug)
			printf("%s: DST: %i, DATA: %i\n",
			token_str(instr.tok.type), instr.DST, instr.DATA);
		cnt++;
	} while(tk.type != TK_END);
	for(i = 0; i < lbl_cnt; i++)
		free(lbls[i].ident);
}

void print_tok(tok_t t)
{
	switch(t.type) {
		#define TOKEN_MCHAR_CASE(name)  case TK_##name:
		TOKEN_TYPES(TOKEN_MCHAR_CASE)
		printf("%s", token_str(t.type));
		break;
		default:
			printf("%c", t.type);
	}
	switch(t.type) {
		case TK_INTLIT:
			printf("(%lli)\n", t.int_val);
			break;
		case TK_LBL_REF:
		case TK_LBL:
			printf("(%s)\n", t.str_val);
			break;
		default:
			printf("\n");
			break;
	}
}

const char *token_str(enum ttype t)
{
	switch (t) {
		#define TTYPE_STRING(name) case TK_##name: return #name;
		TOKEN_TYPES(TTYPE_STRING);
		#undef TTYPE_STRING
	}
}
