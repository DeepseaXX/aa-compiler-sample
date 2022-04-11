#include <stdio.h>
#include <stdlib.h>
#include <getsym.h>
extern TOKEN tok;
extern FILE *infile;
extern FILE *outfile;

void error(char *s);
void statement(void);

void compiler(void)
{
	init_getsym();

	getsym(); //program
	if (tok.attr == RWORD && tok.value == PROGRAM)
	{
		getsym(); //example123
		if (tok.attr == IDENTIFIER)
		{
			getsym(); //;
			if (tok.attr == SYMBOL && tok.value == SEMICOLON)
			{
				//program body
				getsym();
				statement();
				if (tok.attr == SYMBOL && tok.value == PERIOD) // .
				{
					fprintf(stderr, "Parsing Completed. No errors found.\n");
				}
				else
					error("At the end, a period is required.");
			}
			else
				error("After program name, a semicolon is needed.");
		}
		else
			error("Program identifier is needed.");
	}
	else
		error("At the first, program declaration is required.");
}

void error(char *s)
{
	fprintf(stderr, "%s\n", s);
	exit(1);
}

//
// Parser
//

void statement(void) // number ? number
{
	if (tok.attr == NUMBER)
	{
		//tok.attr=NUMBER(?)
		fprintf(outfile, "loadi r0,%d\n", tok.value);
		getsym();
		if (tok.attr == SYMBOL)
		{
			//+-*
			switch (tok.value)
			{
			case PLUS:
				getsym();
				fprintf(outfile, "addi r0,%d\n", tok.value);
				break;
			case MINUS:
				getsym();
				fprintf(outfile, "subi r0,%d\n", tok.value);
				break;
			case TIMES:
				getsym();
				fprintf(outfile, "muli r0,%d\n", tok.value);
				break;
			}
		}
		else if (tok.attr == RWORD && tok.value == DIV)
		//div
		{
			getsym();
			fprintf(outfile, "divi r0,%d\n", tok.value);
		}else error("no mathematical symbol.");
		fprintf(outfile, "writed r0\n");
		fprintf(outfile, "loadi r1,'\\n'\n");
		fprintf(outfile, "writec r1\n");
	}
	else if (tok.attr == RWORD && tok.value == BEGIN)
	{
		getsym();
		statement();
		if (tok.attr == RWORD && tok.value == END)
		{
		}
	}
	getsym();
	if (tok.attr == SYMBOL && tok.value == SEMICOLON)
	{
		getsym();
		statement();
	}
}
