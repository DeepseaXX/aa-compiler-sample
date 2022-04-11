#include <stdio.h>
#include <stdlib.h>
#include <getsym.h>
#include <string.h>

extern TOKEN tok;
extern FILE *infile;
extern FILE *outfile;

#define MAXICLEN 32
#define STACK_SIZE 32

#define TRUEDIV 2
#define TRUE 1
#define FALSE 0

#define BIGGER 1
#define EQUAL 0
#define SMALLER -1

#define LOCATION 7

#define DIVNUM '/'
#define ENDMARK '$'

//記号表
typedef struct
{
    int addr;
    char v[MAXIDLEN + 1];
} s_entry;
s_entry s_table[64];

//Expression()のスタックに関する操作
//普通のint型スタック
typedef struct Stack
{
    int array[STACK_SIZE];
    int pos;
} Stack;

//タイプと内容を格納するカプセル
typedef struct ResultCapsule
{
    //IDENTIFIER   1
    //NUMBER       2
    //LOCATION       7
    int type;
    //IDENTIFIER  addr
    //NUMBER      value
    int value;
} ResultCapsule;

//上記のカプセルを格納するスタック
typedef struct ResultStack
{
    ResultCapsule array[STACK_SIZE];
    int pos;
} ResultStack;
//演算数を格納するスタック
struct ResultStack resultStack;
//演算子を格納するスタック
struct Stack symbolStack;

//countは、定義された変数の個数
int stablelength = 0;
//Lはラベル
int Lcount = 0;
//データ演算の時のメモリポインタ
int MEMP = 0;

void Error(char *s);
void Outblock(void);
void Statement(void);
void Condition(void);
void Expression(void);
int calsymbolLoop(int symbol);
//判定関数
int IsIdentExist(char *charv);
int IsLess16Bit(int number);
//スタック操作関数
void resultPush(ResultCapsule rc);
ResultCapsule resultPop();
void symbolPush(int a);
int symbolPop();
int symbolPeek();
void resultStackPrint(void);
void symbolStackPrint(void);
//順位判定関数
//Expressionで使う符号は全部左結合演算子
int FSymbol(int symbol);
int GSymbol(int symbol);
int SymbolPriority(int symbolA, int symbolB);
//デバッグ用関数
//今のトークの中身を表示
//sはメッセージ
void PrintTok(char *s);

void resultPush(ResultCapsule rc)
{
    resultStack.array[(resultStack.pos)++] = rc;
}
ResultCapsule resultPop()
{
    if (resultStack.pos >= 0)
    {
        return resultStack.array[--(resultStack.pos)];
    }
    else
    {
        //空
        ResultCapsule r = {0, 0};
        return r;
    }
}
void resultStackPrint(void)
{
    printf("resultStack: ");
    int i = 0;
    for (i = 0; i < resultStack.pos; i++)
    {
        ResultCapsule temprc = resultStack.array[i];
        printf("%d:%d  ", temprc.type, temprc.value);
    }
    printf("\n");
}

void symbolPush(int a)
{
    symbolStack.array[(symbolStack.pos)++] = a;
}
int symbolPop()
{
    if (symbolStack.pos >= 0)
    {
        return symbolStack.array[--(symbolStack.pos)];
    }
    else
    {
        return FALSE;
    }
}
//取り出さずに、値を確認するだけ
int symbolPeek()
{
    if (symbolStack.pos >= 0)
    {
        return symbolStack.array[(symbolStack.pos - 1)];
    }
    else
    {
        return FALSE;
    }
}
void symbolStackPrint(void)
{
    printf("symbolStack: ");
    int i = 0;
    for (i = 0; i < symbolStack.pos; i++)
    {
        printf("%d  ", symbolStack.array[i]);
    }
    printf("\n");
}

//順位判定関数
//(最初のマイナス以外、)全部左結合なので
int FSymbol(int symbol)
{
    int result = -1;
    switch (symbol)
    {
    case PLUS:;
    case MINUS:
        result = 2;
        break;
    case TIMES:;
    case DIVNUM:;
    case DIV:
        result = 4;
        break;
    case '!':
        result = 6;
        break;
    case LPAREN:;
    case '$':
        result = 0;
        break;
    case RPAREN:
        result = 11;
        break;
    default:
        Error("No such symbol(F)");
    }
    return result;
}
int GSymbol(int symbol)
{
    int result = -1;
    switch (symbol)
    {
    case PLUS:;
    case MINUS:
        result = 1;
        break;
    case TIMES:;
    case DIVNUM:;
    case DIV:
        result = 3;
        break;
    case '!':
        result = 15;
        break;
    case LPAREN:
        result = 10;
        break;
    case '$':;
    case RPAREN:
        result = 0;
        break;
    default:
        Error("No such symbol(G)");
    }
    return result;
}
int SymbolPriority(int symbolA, int symbolB)
{
    int result = FSymbol(symbolA) - GSymbol(symbolB);
    if (result > 0)
    {
        return BIGGER;
    }

    else if (result < 0)
    {
        return SMALLER;
    }
    else
    {
        return EQUAL;
    }
}

//16bit を超える定数の判定
int Bitcount = 1;
int Bitlist[MAXICLEN];
int IsLess16Bit(int number)
{
    //flag1  >16bit need bitlabel
    if (number > 32767 || number < -32768)
    {
        Bitlist[Bitcount] = number;
        return Bitcount++;
    }
    //flag0  <16bit
    else
    {
        return 0;
    }
}

//定義された変数であるかどうか
int IsIdentExist(char *charv)
{
    //アドレスを返す
    const char *tempcharv = charv;
    int found = -1;
    int temp;
    for (temp = 0; temp < stablelength; temp++)
    {
        //iは変えられた変数。最後にstoreされる対象。(カウント)
        if (strcmp(tempcharv, s_table[temp].v) == 0)
        {
            found = s_table[temp].addr;
            printf("\tget IDENT:%s\n", tempcharv);
            break;
        }
    }
    if (found == -1)
    {
        printf("%s \n", charv);
        Error("Ident Undifined");
    }
    return found;
}
// + - * ( ) $ DIV であるかどうか
int IsCalSymbol()
{
    if ((tok.attr == SYMBOL && (tok.value == PLUS || tok.value == MINUS || tok.value == TIMES || tok.value == LPAREN || tok.value == RPAREN || tok.value == ENDMARK)))
    {
        return TRUE;
    }
    else if ((tok.attr == RWORD && tok.value == DIV))
    {
        return TRUEDIV;
    }
    else
    {
        return FALSE;
    }
}
//解析用関数が要る
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
                Outblock();
                getsym(); // .
                if (tok.attr == SYMBOL && tok.value == PERIOD)
                {
                    fprintf(stderr, "Parsing Completed. No errors found.(compiler)\n");
                }
                else
                    Error("At the end, a period is required.(compiler)");
            }
            else
                Error("After program name, a semicolon is needed.(compiler)");
        }
        else
            Error("Program identifier is needed.(compiler)");
    }
    else
        Error("At the first, program declaration is required.(compiler)");
}

void Error(char *s)
{
    PrintTok("Error out");
    fprintf(stderr, "%s\n", s);
    exit(1);
}

void PrintTok(char *s)
{
    printf("[tok.attr=%1d  tok.value=%6d  tok.charvalue=\"%6s\"  sline=%d]", tok.attr, tok.value, tok.charvalue, tok.sline);
    printf("%s\n", s);
}

//
// Parser
//
//(var+) Statement
void Outblock(void)
{
    printf("Outblock start.\n");
    //VARがあれば、変数の読み込み
    if (tok.attr == RWORD && tok.value == VAR)
    {
        do
        {
            getsym();
            if (tok.attr == IDENTIFIER)
            {
                s_table[stablelength].addr = stablelength;
                strcpy(s_table[stablelength].v, tok.charvalue);
                printf("get VAR:%s, no:%d\n", tok.charvalue, stablelength++);
            }
            else
            {
                Error("No indentifier.(Outblock)");
            }
            getsym();
        } while (tok.attr == SYMBOL && tok.value == COMMA);
        //MEMPの初期化、
        MEMP = stablelength + 1;
    }
    //PrintTok("Outblock semi");
    while (tok.attr == SYMBOL && tok.value == SEMICOLON)
    {
        getsym();
        Statement();
    }
    //プログラム主体部分終了のサイン
    fprintf(outfile, "halt\n");
    //16bitを超える定数用ラベル
    if (Bitcount > 1)
    {
        int i;
        for (i = 1; i < Bitcount; i++)
        {
            fprintf(outfile, "ld%d: data %d\n", i, Bitlist[i]);
        }
    }
}

void Statement(void)
{
    //ident := Expression
    //変数の代入
    printf("------Statement------\n");
    PrintTok("");
    if (tok.attr == IDENTIFIER)
    {
        int tempcharaddr = IsIdentExist(tok.charvalue);
        getsym();
        if (tok.attr == SYMBOL && tok.value == BECOMES)
        {
            printf("\tget \":=\"\n");
            getsym();
            Expression();
        }
        else
        {
            Error(":= is needed.(Statement ident)");
        }
        //数式の部分が終わったら
        fprintf(outfile, "store r0,%d\n", tempcharaddr);
    }
    //begin Statement() end
    //再帰構文の処理
    else if (tok.attr == RWORD && tok.value == BEGIN)
    {
        printf("\tget \"begin\" \n");
        do
        {
            getsym();
            Statement();
        } while (tok.attr == SYMBOL && tok.value == SEMICOLON);
        PrintTok("Statement begin ;break");
        if (tok.attr == RWORD && tok.value == END)
        {
            printf("\tget \"end\"\n");
        }
        else
        {
            Error("End or \";\" is needed.(Statement begin end)");
        }
        printf("\tBGEIN+END block end\n");
    }
    //Lcount使用
    //if Condition() then Statement() (else Statement())
    else if (tok.attr == RWORD && tok.value == IF)
    {
        printf("\tget \"if\"\n");
        getsym();
        int tempLcount = Lcount;
        Condition();
        //PrintTok("then");
        if (tok.attr == RWORD && tok.value == THEN)
        {
            printf("\tLabel used:L%d(ifthenelse)\n", (tempLcount));
            fprintf(outfile, "L%d:\n", (tempLcount));
            getsym();
            Statement();
        }
        else
        {
            Error("Then is needed.(Statement if)\n");
        }
        if (tok.attr == RWORD && tok.value == ELSE)
        {
            Lcount++;
            printf("\tLabel jumped:L%d(ifthenelse)\n", (tempLcount + 2));
            fprintf(outfile, "jmp L%d\n", (tempLcount + 2));
            //ELSEでは、ラベルもう一つ使う
            printf("else get\n");
            printf("\tLabel used:L%d(ifthenelse)\n", (tempLcount + 1));
            fprintf(outfile, "L%d:\n", tempLcount + 1);
            getsym();
            Statement();
            printf("\tLabel used:L%d(ifthenelse)\n", (tempLcount + 2));
            fprintf(outfile, "L%d:\n", tempLcount + 2);
        }
        else
        {
            printf("\tLabel used:L%d(ifthenelse)\n", (tempLcount + 1));
            fprintf(outfile, "L%d:\n", tempLcount + 1);
            //IF THENでは ELSEより使ったラベル一つ少ない
        }
    }
    //Lcount使用
    //while Condition() do Statement()
    else if (tok.attr == RWORD && tok.value == WHILE)
    {

        printf("\tget \"while\"\n");
        //whileの先頭にlabel
        printf("\tLabel used:L%d(whiledo)\n", (Lcount));
        fprintf(outfile, "L%d:\n", (Lcount++));
        getsym();
        int tempLcount = Lcount;
        Condition();
        if (tok.attr == RWORD && tok.value == DO)
        {
            printf("\tget \"do\"\n");
            printf("\tLabel used:L%d(whiledo)\n", (tempLcount));
            fprintf(outfile, "L%d:\n", tempLcount);
            getsym();
            Statement();
            getsym();
            printf("\tLabel jumped:L%d(whiledo)\n", (tempLcount - 1));
            fprintf(outfile, "jmp L%d\n", tempLcount - 1);
            printf("\tLabel used:L%d(whiledo)\n", (tempLcount + 1));
            fprintf(outfile, "L%d:\n", tempLcount + 1);
        }
        else
        {
            Error("Do is needed.(Statement while)");
        }
        printf("\t\"while\" end\n");
    }
    //write ident (,)
    else if (tok.attr == RWORD && tok.value == WRITE)
    {
        printf("\tget \"write\" \n");
        do
        {
            getsym();
            //PrintTok("write ident);
            if (tok.attr == IDENTIFIER)
            {
                int tempcharaddr = IsIdentExist(tok.charvalue);
                fprintf(outfile, "load r0,%d\n", tempcharaddr);
                //出力
                fprintf(outfile, "writed r0\n");
                fprintf(outfile, "loadi r1,'\\n'\n");
                fprintf(outfile, "writec r1\n");
            }
            else
            {
                Error("Program identifier is needed.(Statement write)");
            }
            getsym();
        } while (tok.attr == SYMBOL && tok.value == COMMA);
    }
}

//一回のConditionでは、Lcount+2
void Condition(void)
{
    printf("------Condition start------\n");
    Expression();
    //一番目の結果はr0にあるのでr0->memory
    int tempsp = MEMP++;
    fprintf(outfile, "store r0,%d\n", tempsp);
    //比較演算子を
    //PrintTok("comparesymbol");
    if (tok.attr == SYMBOL)
    {
        int compsymbol = tok.value;
        printf("\tget COMPSYMBOL: %d\n", tok.value);
        getsym();
        Expression(); //二番目の結果はr0にある
        //一番目の結果tempsp
        fprintf(outfile, "load r1,%d\n", tempsp);
        //r1 r0を比較
        fprintf(outfile, "cmpr r1,r0\n");
        //PrintTok("jumpexp2");
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
            Error("compare symbol is needed.(Condition symbol)");
            break;
        }
        printf("\tLabel used:L%d(Condition)\n", Lcount);
        fprintf(outfile, "L%d\n", Lcount++);
        //else か whileの終わり
        printf("\tLabel jumped:L%d(Condition)\n", Lcount);
        fprintf(outfile, "jmp L%d\n", Lcount++);
        printf("\tCondition end.\n");
    }
    else
    {
        Error("compare symbol is needed.(Condition)");
    }
}

void Expression(void)
{
    printf("------Expression start------\n");
    //スタックのリセット
    resultStack.pos = 0;
    symbolStack.pos = 0;
    //終了符号
    symbolPush('$');
    int breakflag = 0;
    int tempmemp = MEMP;
    while (TRUE)
    {
        //数値か変数の場合、スタックにPUSH
        //PrintTok("Expression-while");
        if (tok.attr == NUMBER)
        {
            PrintTok("Expression-while-number");
            ResultCapsule rc = {tok.attr, tok.value};
            printf("\t\tNumPush:%d\n", tok.value);
            resultPush(rc);
            getsym();
        }
        else if (tok.attr == IDENTIFIER)
        {
            PrintTok("Expression-while-ident");
            int tempaddr = IsIdentExist(tok.charvalue);
            printf("\t\tIdentPush:%s\n", tok.charvalue);
            ResultCapsule rc = {tok.attr, tempaddr};
            resultPush(rc);
            getsym();
        }
        //+ - * ( ) $ DIV  演算子の場合
        else if (IsCalSymbol() > 0)
        {
            PrintTok("Expression-while-calsymbol");
            //順位判断
            //lastsymbol=ai, thisSymbol=aj
            int thisSymbol = tok.value;
            //DIV->'/'
            if (tok.attr == RWORD && tok.value == DIV)
            {
                thisSymbol = '/';
            }
            calsymbolLoop(thisSymbol);
            getsym();
        }
        else
        {
            PrintTok("Expression-before-end");
            //式の読み込みが終わり、スタックが空まで読み続ける
            //ループは$=$まで続ける
            int bitFlag = 0;
            while (breakflag == 0)
            {
                breakflag = calsymbolLoop('$');
            }
            ResultCapsule temprc = resultPop();
            switch (temprc.type)
            {
            case IDENTIFIER:
                printf("\t\tIdentToR0:%d...addr\n", temprc.value);
                fprintf(outfile, "load r0,%d\n", temprc.value);
                break;
            case NUMBER:
                printf("\t\tNumToR0:%d\n", temprc.value);
                bitFlag = IsLess16Bit(temprc.value);
                if (bitFlag == 0)
                {
                    fprintf(outfile, "loadi r0,%d\n", temprc.value);
                }
                else
                {
                    fprintf(outfile, "load r0,ld%d\n", bitFlag);
                }
                break;
            case LOCATION:
                printf("\t\tAddrToR0:%d\n", temprc.value);
                fprintf(outfile, "load r0,%d\n", temprc.value);
                break;
            default:
                printf("who are you ?:temprc.type=%d  temprc.value=%d\n", temprc.type, temprc.value);

                break;
            }
        }
        if (breakflag == 1)
        {
            break;
        }
        //resultStackPrint();
        //symbolStackPrint();
    }
    MEMP = tempmemp;
    printf("\tExpression end\n");
}

int calsymbolLoop(int symbol)
{
    int registerCount = 0;
    int thisSymbol = symbol;
    int lastSymbol = symbolPeek();
    printf("last:%d  this:%d\n", lastSymbol, thisSymbol);
    int priorityFlag = SymbolPriority(lastSymbol, thisSymbol);
    do
    {
        //ai>>aj aiと演算数をスタックから降ろして出力
        //結果（場所）はスタックに
        if (priorityFlag > 0)
        {
            int tempsb = symbolPop();
            printf("\t\tSymbolPop:%d\n", tempsb);
            //=lastSymbol
            //一番目の数字
            ResultCapsule rc = resultPop();
            int firstRCount = registerCount;
            int bitFlag;
            switch (rc.type)
            {
            case IDENTIFIER:
                printf("\t\tIdentPop:%d...addr\n", rc.value);
                fprintf(outfile, "load r%d,%d\n", registerCount, rc.value);
                break;
            case NUMBER:
                printf("\t\tNumPop:%d\n", rc.value);
                bitFlag = IsLess16Bit(rc.value);
                if (bitFlag == 0)
                {
                    fprintf(outfile, "loadi r%d,%d\n", registerCount, rc.value);
                }
                else
                {
                    fprintf(outfile, "load r%d,ld%d\n", registerCount, bitFlag);
                }
                break;
            case LOCATION:
                printf("\t\tAddrPop:%d\n", rc.value);
                fprintf(outfile, "load r%d,%d\n", registerCount, rc.value);
                break;
            default:
                printf("who are you ?:temprc.type=%d  temprc.value=%d\n", rc.type, rc.value);

                break;
            }
            registerCount++;
            //二番目の数字
            int secondRCount = registerCount;
            rc = resultPop();
            switch (rc.type)
            {
            case IDENTIFIER:
                printf("\t\tIdentPop:%d...addr\n", rc.value);
                fprintf(outfile, "load r%d,%d\n", registerCount, rc.value);
                break;
            case NUMBER:
                printf("\t\tNumPop:%d\n", rc.value);
                bitFlag = IsLess16Bit(rc.value);
                if (bitFlag == 0)
                {
                    fprintf(outfile, "loadi r%d,%d\n", registerCount, rc.value);
                }
                else
                {
                    fprintf(outfile, "load r%d,ld%d\n", registerCount, bitFlag);
                }
                break;
            case LOCATION:
                printf("\t\tAddrPop:%d\n", rc.value);
                fprintf(outfile, "load r%d,%d\n", registerCount, rc.value);
                break;
            default:
                printf("who are you ?:temprc.type=%d  temprc.value=%d\n", rc.type, rc.value);

                break;
            }
            //計算
            //レジスタの順番は要注意
            switch (lastSymbol)
            {
            case PLUS:
                fprintf(outfile, "addr r%d,r%d\n", secondRCount, firstRCount);
                break;
            case MINUS:
                fprintf(outfile, "subr r%d,r%d\n", secondRCount, firstRCount);
                break;
            case TIMES:
                fprintf(outfile, "mulr r%d,r%d\n", secondRCount, firstRCount);
                break;
            case DIVNUM:
                fprintf(outfile, "divr r%d,r%d\n", secondRCount, firstRCount);
                break;
            default:
                printf("who are you ?:temprc.type=%d  temprc.value=%d\n", rc.type, rc.value);
                break;
            }
            //場所を保存、スタックへPUSH
            ResultCapsule temprc = {LOCATION, MEMP};
            fprintf(outfile, "store r%d,%d\n", secondRCount, MEMP);
            MEMP++;
            resultPush(temprc);
            printf("\t\tAddrPush:%d\n", temprc.value);
            registerCount = 0; //結局０と１しか使っていない
            //ai>>ajの間、順位比較の繰り返し
            lastSymbol = symbolPeek(); //=lastlastsymbol
            printf("last:%d  this:%d\n", lastSymbol, thisSymbol);
            priorityFlag = SymbolPriority(lastSymbol, thisSymbol);
        }
        //ai==aj '('
        else if (priorityFlag == 0)
        {
            int lastSymbol = symbolPeek();
            //')'ならば、'('もスタックから取り出す
            if (lastSymbol == LPAREN)
            {
                int tempsb = symbolPop();
                printf("\t\tSymbolPop:%d\n", tempsb);
            }
            //スタックに'$'だけならば、終了
            else if (lastSymbol == '$')
            {
                return 1;
            }
            break;
        }
        //ai<<aj スタックに積む
        else if (priorityFlag < 0)
        {
            printf("\t\tSymPush:%d\n", thisSymbol);
            symbolPush(thisSymbol);
            break;
        }
    } while (TRUE);
    return 0;
}