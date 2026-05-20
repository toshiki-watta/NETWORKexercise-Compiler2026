/*
  sr.c
  Copyright (C) 1996,2002 Toyohashi University of Technology
*/

#include <stdio.h>
#include <stdlib.h>
#include "sr.h"
#include "loader.h"
#include "proc.h"

static char *procname;

int trace_mode = 0;
int step_mode = 0;

int main(int argc, char **argv)
{
  extern char *rindex(const char *, int);
  FILE *input;
  int argpos = 1;
  char *filename = NULL;

  procname = rindex(argv[0], '/');
  procname = (procname == 0) ? argv[0] : procname + 1;
  
  /* init internal */
  init_proc();

  while (argpos < argc){
      if (argv[argpos][0] == '-'){
          if (argv[argpos][1] == 't'){
              trace_mode = 1;
              argpos ++;
          }
          else if (argv[argpos][1] == 's'){
              step_mode = 1;
              argpos ++;
          }
          else {
              fprintf(stderr, "Unknown option %s\n", argv[argpos]);
              fprintf(stderr, "USAGE: %s [options] <objfile>\n", procname);
              fprintf(stderr, "  options: -t: trace mode\n");
              fprintf(stderr, "           -s: step mode\n");
              exit(1);
          }
      }
      else {
          /* filename二重定義 */
          if (filename != NULL){
              fprintf(stderr, "USAGE: %s [-t]<objfile>", procname);
              exit(1);
          }
          else {
              filename = argv[argpos];
              argpos ++;
          }
      }
  }
  /* ファイル名指定なしなら標準入力からロード */
  if (filename == NULL){
      loader(stdin);
  }
  else {
  printf("loading '%s'\n", filename);
      if ((input = fopen(filename, "r")) != 0) {
          loader(input);
          fclose(input);
      }
      else {
          fatal("cannot open object file");
      }
  }
  fetch_and_exec();
  return(0);
}

#ifdef DUMMY
  /* load object file */
  if (argc > 2) {
    fprintf(stderr, "USAGE: %s <objfile>", procname);
    exit(1);
  } else if (argc == 1) {
    loader(stdin);
  } else if ((input = fopen(argv[1], "r")) == 0) {
    fatal("cannot open object file");
  } else {
    loader(input);
    fclose(input);
  }
  fetch_and_exec();
  return(0);
}
#endif

void fatal(const char *message)
{
  fprintf(stderr, "%s:%s\n", procname, message);
  exit(1);
}
