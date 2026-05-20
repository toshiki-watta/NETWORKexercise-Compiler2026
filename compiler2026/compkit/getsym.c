/*
  getsym() -- simple lexical analyzer
  getsym.h
  Copyright (C) 1996 Toyohashi University of Technology
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include "getsym.h"

extern TOKEN tok;
extern FILE *infile;

static int sline;
static int toklen;

static char symbols[] = {
  '+', '-', '*', '(', ')', '=',
  ',', '.', ';', ':', '<', '>' };

static int symvalue[] = {
  PLUS, MINUS, TIMES, LPAREN, RPAREN, EQL,
  COMMA, PERIOD, SEMICOLON, COLON, LESSTHAN, GRTRTHAN };

static int
get_char()
{
  register int c = getc(infile);
  if (c == '\n')
    sline++;
  return c;
}

static void
unget_char(c)
     register int c;
{
  if (c == '\n')
    sline--;
  ungetc(c, infile);
}

static int
skip_blanks()
{
  register int c;

  while ((c = get_char()) != EOF && isspace(c));
  return c;
}

static void
flush_tokenbuf()
{
  tok.charvalue[0] = '\0';
  toklen = 0;
}

static void
push_tokenbuf(c)
     int c;
{
  if (toklen < MAXIDLEN) {
    tok.charvalue[toklen++] = c;
    tok.charvalue[toklen] = '\0';
  }
}

void
init_getsym()
{
  sline = 1;
}

static void scan_identifier();
static void scan_number();
static void scan_symbol();

void
getsym()
{
  register int c = skip_blanks();
  char *symp;

  tok.sline = sline;

  flush_tokenbuf();

  if (c == EOF) {
    tok.attr = ENDFILE;
    return;
  }
  push_tokenbuf(c);
  if (isalpha(c)) {
    scan_identifier();
  } else if (isdigit(c)) {
    scan_number();
  } else if ((symp = index(symbols, c)) != 0) {
    scan_symbol(c, (symp - symbols));
  } else {
    tok.attr = SYMERR;
  }
}

/* Command-line: gperf -p -j1 -i 1 -o -t -N is_reserved_word -k1,2,3,$ getsym.gperf  */ 

struct resword { char *name; int token; };

#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 9
#define MIN_HASH_VALUE 5
#define MAX_HASH_VALUE 19
/*
   13 keywords
   15 is the maximum key range
*/

static int
hash (str, len)
     register char  *str;
     register int  len;
{
  static int hash_table[] =
    {
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,   5,   7,  19,
       1,   1,   8,   1,   6,   1,  19,  19,   3,   7,
       2,   1,   1,  19,   1,   1,   2,  19,   1,   1,
      19,  19,  19,  19,  19,  19,  19,  19,
    };
  register int hval = len ;

  switch (hval)
    {
      default:
      case 3:
        hval += hash_table[str[2]];
      case 2:
        hval += hash_table[str[1]];
      case 1:
        hval += hash_table[str[0]];
    }
  return hval + hash_table[str[len - 1]] ;
}

struct resword *
is_reserved_word  (str, len)
     register char *str;
     register int len;
{

  static struct resword wordlist[] =
    {
      { "", }, { "", }, { "", }, { "", }, { "", }, 
      {"do",  DO },
      { "", }, 
      {"div",  DIV },
      {"end",  END },
      {"write",  WRITE },
      {"else",  ELSE },
      {"var",  VAR },
      {"read",  READ },
      {"procedure",  PROCEDURE },
      {"while",  WHILE },
      {"then",  THEN },
      {"begin",  BEGIN },
      {"program",  PROGRAM },
      { "", }, 
      {"if",  IF },
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= MIN_HASH_VALUE)
        {
          register char *s = wordlist[key].name;

          if (*s == *str && !strcmp (str + 1, s + 1))
            return  &wordlist[key];
        }
    }
  return 0;
}

static void
scan_identifier()
{
  register int c;
  struct resword *rp;

  while ((c = get_char()) != EOF && isalnum(c)) {
    push_tokenbuf(c);
  }
  if (c != EOF) {
    unget_char(c);
  }
  if ((rp = is_reserved_word(tok.charvalue, toklen)) != 0) {
    tok.attr = RWORD;
    tok.value = rp->token;
  } else {
    tok.attr = IDENTIFIER;
  }
}

static void
scan_number()
{
  register int c;

  while ((c = get_char()) != EOF && isdigit(c)) {
    push_tokenbuf(c);
  }
  if (c != EOF) {
    unget_char(c);
  }
  tok.attr = NUMBER;
  tok.value = atoi(tok.charvalue);
}

static void
scan_symbol(c, k)
     int c;
     int k;
{
  tok.attr = SYMBOL;
  tok.value = symvalue [k];

  switch (c) {
  case '<':
  case '>':
  case ':':
    {
      int c2 = get_char();
      if ((c == '<' && c2 == '>') || c2 == '=') {
	push_tokenbuf(c2);
	switch (c) {
	case '<':
	  tok.value = (c2 == '>') ? NOTEQL : LESSEQL;
	  break;
	case '>':
	  tok.value = GRTREQL;
	  break;
	case ':':
	  tok.value = BECOMES;
	  break;
	}
      } else if (c2 != EOF) {
	unget_char(c2);
      }
    }
    break;
  }
}
