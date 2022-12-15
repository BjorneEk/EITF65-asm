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
	lbl_t *lbls;
	u32_t lc;
	tok_t *toks;
	u32_t tc;
	i32_t ti;
} ctx;

void print_tok(tok_t tk);

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

void consume_whitespaces()
{
	while (consume_wsc())
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
#define putlbl(lbl) printf("\033[32;1;4mlbl\033[0m: %s:	0x%02X\n",(lbl).str_val, (lbl).addr+1)

void *paste_buffer(void *dst, u32_t *sz, void *buff, u32_t *bz, size_t width)
{
	void *res;

	res = realloc(dst, (*sz + *bz) * width);
	memcpy(res, buff, *bz * width);
	*sz += *bz;
	*bz = 0;
	return res;
}

void tokenize(FILE *f, i32_t verbosity)
{
	lbl_t lbuff[100];
	tok_t tbuff[255];
	u32_t li, ti, lsz, tsz;
	tok_t tk;

	li = ti = 0;
	ctx = (struct ctx){.f = f, .line = 0, .col = 0, .addr=-1};
	do {
		consume_whitespaces();
		tk = next_tok();
		if(is_instr(tk))
			ctx.addr++;
		tk.addr = ctx.addr;

		if(tk.type == TK_LBL) {
			if(verbosity > 1)
				putlbl(tk);
			lbuff[li++] = new_lbl(tk.str_val, tk.addr+1);
			if(li >= 100)
				ctx.lbls = paste_buffer(ctx.lbls, &lsz, &lbuff, &li, sizeof(lbl_t));
		} else {
			if(verbosity > 2)
				print_tok(tk);
			tbuff[ti++] = tk;
			if(ti >= 100)
				ctx.toks = paste_buffer(ctx.toks, &tsz, &tbuff, &ti, sizeof(tok_t));
		}
	} while(tk.type != TK_END);

	ctx.lbls = paste_buffer(ctx.lbls, &lsz, &lbuff, &li, sizeof(lbl_t));
	ctx.lc = lsz;
	ctx.toks = paste_buffer(ctx.toks, &tsz, &tbuff, &ti, sizeof(tok_t));
	ctx.lc = lsz;
}


tok_t ntok()
{
	return ctx.toks[ctx.ti++];
}

i32_t lbl_idx(const char *id)
{
	i32_t i;

	for(i = 0; i < ctx.lc; i++)
		if(!strcmp(ctx.lbls[i].ident,id))
			return i;
	return -1;
}

u32_t expect_addr()
{
	tok_t tk;
	u32_t i;
	lbl_t res;
	tk = ntok();
	if(tk.type != TK_LBL_REF && tk.type != TK_INTLIT) {
		parse_error(tk, "expected label reference or literal address, found: '%s'",
		token_str(tk.type));
	}
	if(tk.type == TK_INTLIT)
		return tk.int_val;
	else if ((i = lbl_idx(tk.str_val)) == -1)
		parse_error(tk, "use of undeclared identifier: '%s'", tk.str_val);
	free(tk.str_val);
	return ctx.lbls[i].addr;
}

u32_t expect_int()
{
	tok_t tk;
	tk = ntok();
	if(tk.type != TK_INTLIT) {
		parse_error(tk, "expected integer literal, found: '%s'",
		token_str(tk.type));
	}
	return tk.int_val;

}

u32_t expect_register()
{
	tok_t tk;

	tk = ntok();
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

	tk = ntok();
	if (tk.type != ',')
		parse_error(tk, "expected comma, found: '%s'",
		token_str(tk.type));
	return;
}

void free_ctx()
{
	u32_t i;

	for(i = 0; i < ctx.tc; i++)
		if(ctx.toks[i].type == TK_LBL_REF)
			free(ctx.toks[i].str_val);
	for(i = 0; i < ctx.lc; i++)
		free(ctx.lbls[i].ident);
	free(ctx.toks);
	free(ctx.lbls);
}

#define putins(in) printf("\e[1m|\e[m[\033[31;1;4m%s\033[0m]	\
\e[1m|\e[m%i	\e[1m|\e[m% 3i	\e[1m|\e[m%02X%02X	\
\e[1m|\e[m0x%02X\e[1m|\e[m\n",token_str(in.tok.type), \
in.DST, in.DATA,(in.INS << 1) | in.DST, in.DATA, in.tok.addr);

void assemble(const char *src, const char *dst, i32_t fmt, i32_t verbosity)
{
	FILE *out;
	FILE *in;
	ins_t instr;
	tok_t tk;
	u32_t i;

	in = fopen(src, "r");
	if (!in) {
		fprintf(stderr, ERR_LBL ": could not open file: '%s' for reading\n", src);
		exit(-1);
	}

	tokenize(in, verbosity);

	out = fopen(dst, "w");
	if (!out) {
		fprintf(stderr, ERR_LBL ": could not open file: '%s' for writing\n", dst);
		exit(-1);
	}
	if(verbosity)
		printf("\nProgram summary:\n\e[1m|INS	|DEST	|DATA	|HEX 	|ADDR|\e[m\n");
	ctx.ti = 0;
	do {
		tk = ntok();
		if(!is_instr(tk)) {
			if(tk.type != TK_END)
				parse_error(tk, "expected instruction keyword, found '%s'",
					token_str(tk.type));
			continue;
		}

		instr.INS = tk.type;
		instr.tok = tk;
		switch(tk.type) {
			case TK_CALL:
			case TK_BZ:
			case TK_JMP:
				instr.DATA = expect_addr();
				instr.DST = 0;
				break;
			case TK_RET: break;
			case TK_SUB:
			case TK_ADD:
			case TK_AND:
			case TK_LD:
				instr.DST = expect_register();
				expect_comma();
				instr.DATA = expect_int();
				break;
			case TK_IN:
			case TK_OUT:
				instr.DST = expect_register();
		}
		switch(fmt) {
			case FMT_BIN:
				fputc((instr.INS << 1) | instr.DST, out);
				fputc(instr.DATA, out);
				break;
			case FMT_HEX:
				fprintf(out, "%02X%02X;\n",
					(instr.INS << 1) | instr.DST, instr.DATA);
				break;
		}
		if (verbosity)
			putins(instr);
	} while(tk.type != TK_END);
	free_ctx();
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
