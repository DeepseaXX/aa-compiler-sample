/*
  getsym() -- simple lexical analyzer
  getsym.h
  Copyright (C) 1996 Toyohashi University of Technology
*/

#define MAXIDLEN 15

typedef struct
{
  int attr;
  int value;
  char charvalue[MAXIDLEN + 1];
  int sline;
} TOKEN;

#define IDENTIFIER 1
#define NUMBER 2
#define RWORD 3
#define SYMBOL 4
#define ENDFILE 5
#define SYMERR 6
//#define LOCATION    7
//#define PROCIDENTIFIER 8

#define BEGIN 260
#define DIV 261
#define DO 262
#define ELSE 263
#define END 264
#define IF 265
#define PROCEDURE 266
#define PROGRAM 267
#define THEN 268
#define VAR 269
#define WHILE 270

#define READ 271
#define WRITE 272

#define PLUS '+'
#define MINUS '-'
#define TIMES '*'
#define LPAREN '('
#define RPAREN ')'
#define EQL '='
#define COMMA ','
#define PERIOD '.'
#define SEMICOLON ';'
#define BECOMES 256 /* := */
#define LESSTHAN '<'
#define LESSEQL 257 /* <= */
#define NOTEQL 258  /* <> */
#define GRTRTHAN '>'
#define GRTREQL 259 /* >= */
#define COLON ':'
//#define DIVNUM '/'
//#define ENDMARK '$'

extern void init_getsym();
extern void getsym();
