#include <stdio.h>
#include <stdlib.h>
#include <getsym.h>
#include <string.h>
extern TOKEN tok;
extern FILE *infile;
extern FILE *outfile;

void error(char *s);
void outblock(void);
void statement(void);
void expression(void);
void condition(void);
//countは、定義された変数の個数
int count = 0;
//Lはラベル
int Lcount = 0;

int i, j, k;
typedef struct
{
	int addr;
	char v[MAXIDLEN + 1];
} s_entry;
s_entry s_table[32];
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
				getsym();
				outblock();
				//この間に問題がある
				getsym();
				if (tok.attr == SYMBOL && tok.value == PERIOD) // .
				{
					fprintf(stderr, "Parsing Completed. No errors found.(compiler)\n");
				}
				else
					error("At the end, a period is required.(compiler)");
			}
			else
				error("After program name, a semicolon is needed.(compiler)");
		}
		else
			error("Program identifier is needed.(compiler)");
	}
	else
		error("At the first, program declaration is required.(compiler)");
}

void error(char *s)
{
	fprintf(stderr, "%s\n", s);
	exit(1);
}

//
// Parser
//
//(var+)statement
void outblock(void)
{
	printf("outblock start.\n");
	if (tok.attr == RWORD && tok.value == VAR)
	{
		do
		{
			getsym();
			if (tok.attr == IDENTIFIER)
			{
				//charvalue\t->>>s_table
				strcpy(s_table[count].v, tok.charvalue);
				printf("get VAR:%s, no:%d\n", tok.charvalue, count++);
			}
			else
			{
				error("No indentifier.(outblock)");
			}
			getsym();
		} while (tok.attr == SYMBOL && tok.value == COMMA);
	}
	//printf("outblock semi(now tok.attr=%d, tok.value=%d, tok.charvalue=\"%s\" )\n", tok.attr, tok.value, tok.charvalue);
	while (tok.attr == SYMBOL && tok.value == SEMICOLON)
	{
		getsym();
		statement();
	}
}

void statement(void)
{
	//ident+:=+expression
	//変数の代入
	printf("--statement start(now tok.attr=%d, tok.value=%d, tok.charvalue=\"%s\")\n", tok.attr, tok.value, tok.charvalue);
	if (tok.attr == IDENTIFIER)
	{
		int flag = 0;
		for (i = 0; i < count; i++)
		{
			//iは変えられた変数。最後にstoreされる対象。(カウント)
			if (strcmp(tok.charvalue, s_table[i].v) == 0)
			{
				flag = 1;
				printf("\tget IDENT:%s\n", tok.charvalue);
				break;
			}
		}
		if (flag == 0)
		{
			error("Undefined ident.(statement ident)");
		}
		getsym();
		if (tok.attr == SYMBOL && tok.value == BECOMES)
		{
			printf("\tget \":=\"\n");
			getsym();
			expression();
		}
		else
		{
			error(":= is needed.(statement ident)");
		}
		//数式の部分が終わったら
		//iはアドレス
		fprintf(outfile, "store r0,%d\n", i);
	}
	//begin+statement+end
	//再帰構文の処理
	else if (tok.attr == RWORD && tok.value == BEGIN)
	{
		printf("\tget \"begin\" \n");
		do
		{
			getsym();
			statement();
					printf("---------------tok.attr=%d  tok.value=%d\n",tok.attr,tok.value);
		} while (tok.attr == SYMBOL && tok.value == SEMICOLON);
		if (tok.attr == RWORD && tok.value == END)
		{
			printf("\tget \"end\"\n");
		}
		else
		{
			error("End or \";\" is needed.(statement begin end)");
		}
		printf("\tBGEIN+END block end\n");
	}
	//Lcount使用
	//if+condition+then+statement(+else+statement)
	else if (tok.attr == RWORD && tok.value == IF)
	{
		printf("\tget \"if\"\n");
		getsym();
		int tempLcount = Lcount;
		condition();
		//printf("then(now tok.attr=%d, tok.value=%d, tok.charvalue=\"%s\" )\n",tok.attr, tok.value, tok.charvalue);
		if (tok.attr == RWORD && tok.value == THEN)
		{
			printf("\tLabel used:L%d(ifthenelse)\n", (tempLcount));
			fprintf(outfile, "L%d:\n", (tempLcount));
			getsym();
			statement();
		}
		else
		{
			error("Then is needed.(statement if)\n");
		}
		if (tok.attr == RWORD && tok.value == ELSE)
		{
			Lcount++;
			printf("\tLabel jumped:L%d(ifthenelse)\n", (tempLcount + 2));
			fprintf(outfile, "jmp L%d\n", (tempLcount + 2));
			//IF THEN ELSEでは、ラベルもう一つ使う
			printf("else get\n");
			printf("\tLabel used:L%d(ifthenelse)\n", (tempLcount + 1));
			fprintf(outfile, "L%d:\n", tempLcount + 1);
			getsym();
			statement();
			printf("\tLabel used:L%d(ifthenelse)\n", (tempLcount + 2));
			fprintf(outfile, "L%d:\n", tempLcount + 2);
		}
		else
		{
			printf("\tLabel used:L%d(ifthenelse)\n", (tempLcount + 1));
			fprintf(outfile, "L%d:\n", tempLcount + 1);
			//IF THENでは IF THEN ELSEより使ったラベル一つ少ない
		}
	}
	//Lcount使用
	//while+condition+do+statement
	else if (tok.attr == RWORD && tok.value == WHILE)
	{

		printf("\tget \"while\"\n");
		//whileの先頭にlabel
		printf("\tLabel used:L%d(whiledo)\n", (Lcount));
		fprintf(outfile, "L%d:\n", (Lcount++));
		getsym();
		int tempLcount = Lcount;
		condition();
		if (tok.attr == RWORD && tok.value == DO)
		{
			printf("\tget \"do\"\n");
			printf("\tLabel used:L%d(whiledo)\n", (tempLcount));
			fprintf(outfile, "L%d:\n", tempLcount);
			getsym();
			statement();
			getsym();
			printf("\tLabel jumped:L%d(whiledo)\n", (tempLcount - 1));
			fprintf(outfile, "jmp L%d\n", tempLcount - 1);
			printf("\tLabel used:L%d(whiledo)\n", (tempLcount + 1));
			fprintf(outfile, "L%d:\n", tempLcount + 1);
		}
		else
		{
			error("Do is needed.(statement while)");
		}
		printf("\t\"while\" end\n");
	}
	//write+ident(+,)
	else if (tok.attr == RWORD && tok.value == WRITE)
	{
		printf("\tget \"write\" \n");
		do
		{
			getsym();
			//printf("write ident(now tok.attr=%d, tok.value=%d, tok.charvalue=\"%s\" )\n", tok.attr, tok.value, tok.charvalue);
			if (tok.attr == IDENTIFIER)
			{
				int flag = 0;
				for (k = 0; k < count; k++)
				{
					//kは変えられた変数。最後にstoreされる対象。(カウント)
					if (strcmp(tok.charvalue, s_table[k].v) == 0)
					{
						flag = 1;
						printf("\tget IDENT:%s\n", tok.charvalue);
						fprintf(outfile, "load r0,%d\n", k);
						//出力
						fprintf(outfile, "writed r0\n");
						fprintf(outfile, "loadi r1,'\\n'\n");
						fprintf(outfile, "writec r1\n");
						break;
					}
				}
				if (flag == 0)
				{
					error("Undefined.(statement write)");
				}
			}
			else
			{
				error("Program identifier is needed.(statement write)");
			}
			getsym();

		} while (tok.attr == SYMBOL && tok.value == COMMA);
	}
}

//一回のconditionでは、Lcount+2
void condition(void)
{
	printf("--condition start.\n");
	expression();					   //一番目の結果はr0にあるので
	fprintf(outfile, "loadr r2,r0\n"); //一番目の結果はr2に
	//比較演算子を
	//printf("comparesymbol(now tok.attr=%d, tok.value=%d, tok.charvalue=\"%s\" )\n", tok.attr, tok.value, tok.charvalue);
	if (tok.attr == SYMBOL)
	{
		int compsymbol = tok.value;
		printf("\tget COMPSYMBOL: %d\n", tok.value);
		getsym();
		expression(); //二番目の結果はr0->r3
		fprintf(outfile, "loadr r3,r0\n");
		//r2 r3を比較
		fprintf(outfile, "cmpr r2,r3\n");
		//printf("jumpexp2(now tok.attr=%d, tok.value=%d, tok.charvalue=\"%s\" )\n", tok.attr, tok.value, tok.charvalue);
		switch (compsymbol)
		{
		case EQL: // =
			fprintf(outfile, "jz ");
			break;
		case NOTEQL: //<>
			fprintf(outfile, "jnz ");
			break;
		case LESSTHAN: //<
			fprintf(outfile, "jlt ");
			break;
		case GRTRTHAN: //>
			fprintf(outfile, "jgt ");
			break;
		case LESSEQL: //<=
			fprintf(outfile, "jle ");
			break;
		case GRTREQL: //>=
			fprintf(outfile, "jge ");
			break;
		default:
			error("compare symbol is needed.(condition symbol)");
			break;
		}
		printf("\tLabel used:L%d(condition)\n", Lcount);
		fprintf(outfile, "L%d\n", Lcount++);
		//else か whileの終わり
		printf("\tLabel jumped:L%d(condition)\n", Lcount);
		fprintf(outfile, "jmp L%d\n", Lcount++);
		printf("  --condition end.\n");
	}
	else
	{
		error("compare symbol is needed.(condition)");
	}
}

//expression はr0 r1を使う
//結果はr0
void expression(void)
{
	printf("--expression start\n");
	//一番目の数値/変数
	if (tok.attr == NUMBER)
	{
		//即値の場合
		printf("\tget NUMBER:%d\n", tok.value);
		fprintf(outfile, "loadi r0,%d\n", tok.value);
	}
	else if (tok.attr == IDENTIFIER)
	{ //変数の場合
		//jは:=のあとに一番目の変数
		int flag = 0;
		for (j = 0; j <= count; j++)
		{
			//tableと照会
			if (strcmp(tok.charvalue, s_table[j].v) == 0)
			{
				flag = 1;
				//変数->r0
				printf("\tget IDENT:%s\n", s_table[j].v);
				fprintf(outfile, "load r0,%d\n", j);
				break;
			}
		}
		if (flag == 0)
		{
			error("Undefined.(expression)");
		}
	}
	else
	{
		error("Identifier or number is needed.(expression)");
	}
	//演算子
	//printf("a1(now tok.attr=%d, tok.value=%d, tok.charvalue=\"%s\" )\n",tok.attr, tok.value, tok.charvalue);
	getsym();
	if ((tok.attr == SYMBOL && (tok.value == PLUS || tok.value == MINUS || tok.value == TIMES)) || (tok.attr == RWORD && tok.value == DIV))
	{
		//演算子の保存
		printf("\tget SYMBOL,tok.value=%d\n", tok.value);
		int calsymbol = tok.value;
		getsym();
		if (tok.attr == NUMBER)
		{
			printf("\tget NUMBER:%d\n", tok.value);
			//即値
			//+-*
			printf("calsymbol:%d, tok.attr:%d, tok.value:%d\n", calsymbol, tok.attr, tok.value);
			switch (calsymbol)
			{
			case PLUS:
				fprintf(outfile, "addi r0,%d\n", tok.value);
				break;
			case MINUS:
				fprintf(outfile, "subi r0,%d\n", tok.value);
				break;
			case TIMES:
				fprintf(outfile, "muli r0,%d\n", tok.value);
				break;
			case DIV:
				fprintf(outfile, "divi r0,%d\n", tok.value);
				break;
			}
		}
		else if (tok.attr == IDENTIFIER)
		{ //変数の場合
			//kは:=のあとに2番目の変数
			int flag = 0;
			for (k = 0; k < count; k++)
			{
				//tableと照会
				if (strcmp(tok.charvalue, s_table[k].v) == 0)
				{
					flag = 1;
					//r1に
					printf("\tget IDENT:%s\n", tok.charvalue);
					fprintf(outfile, "load r1,%d\n", k);
					break;
				}
			}
			if (flag == 0)
			{
				error("Undefined.(expression)");
			}
			switch (calsymbol)
			{
				//結果はr0にいれる
			case PLUS:
				fprintf(outfile, "addr r0,r1\n");
				break;
			case MINUS:
				fprintf(outfile, "subr r0,r1\n");
				break;
			case TIMES:
				fprintf(outfile, "mulr r0,r1\n");
				break;
			case DIV:
				fprintf(outfile, "divr r0,r1\n");
				break;
			}
		}
		getsym();
	}
	else
		;
	//演算子がなく、そのまま終わる場合
	//printf("a2(now tok.attr=%d, tok.value=%d, tok.charvalue=\"%s\" )\n",tok.attr, tok.value, tok.charvalue);
	printf("  --expression end\n");
}
