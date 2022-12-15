/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 *
 * a assembler built for the MCU constructed in the course
 * EIT65 at Lunds tekniska högskola
 *
 *==========================================================*/


#include "asm.h"
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>


void usage(const char *prog)
{
	printf("Usage:\n");
	printf("%s <filename>\n", prog);
	printf("Options:\n");
	printf("	<\033[1;33m-o\033[0m> <filename>	specify output file\n");
	printf("	<\033[1;33m-v\033[0m> <\033[1;33m-vv\033[0m>	verbosity level, useful for debuging\n");
	printf("	<\033[1;33m-h\033[0m>		display usage info\n");
	printf("	<\033[1;33m-f\033[0m> <(x, hex) | (b, bin)>	specify output format\n");
	printf("Example: \n");
	printf("	%s main.S -o myprogram.bin\n", prog);
}
void generate_binary(const char * src, const char * dst, i32_t verbosity, bool hex)
{
	FILE *f;
	lbl_t *lbls;
	u32_t i, lbl_cnt;
	f = fopen(src, "r");
	if(verbosity > 0)
		putchar('\n');
	lbls = get_labels(f, &lbl_cnt, verbosity > 1);
	if(verbosity > 1)
		putchar('\n');
	rewind(f);
	assemble(f, lbls, lbl_cnt, dst, !!verbosity, hex);
	free(lbls);
}

i32_t main(i32_t argc, char *argv[])
{
	i32_t opt, verbosity = 0;
	char *input = NULL;
	char *output = "out.bin";
	bool hex = true;
	if(argc < 2){
		fprintf(stderr, "no input file specified, try -h for help");
		exit(-1);
	}

	input = argv[1];

	while((opt = getopt(argc-1, argv+1, ":o:f:vh")) != -1) {
		switch(opt) {
			case 'v':
				verbosity++;
				break;
			case 'o':
				output = optarg;
				break;
			case 'h':
				usage(argv[0]);
				break;
			case 'f':
				if(	!strcmp(optarg, "x")   ||
					!strcmp(optarg, "X")   ||
					!strcmp(optarg, "hex") ||
					!strcmp(optarg, "HEX"))
					hex = true;
				else if(!strcmp(optarg, "b")   ||
					!strcmp(optarg, "B")   ||
					!strcmp(optarg, "bin") ||
					!strcmp(optarg, "BIN"))
					hex = false;
				break;
			case ':':
				fprintf(stderr, "option %c needs a value\n", opt);
				return -1;
			case '?':
				fprintf(stderr, "unknown option: %c\n", optopt);
				return -1;
		}
	}

	if(verbosity > 1)
		printf("verbosity: %i\ninput: %s\noutput: %s\n",verbosity,input,output);

	generate_binary(input, output, verbosity, hex);
	return 0;
}
