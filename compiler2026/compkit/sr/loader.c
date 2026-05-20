/*
  loader.c
  Copyright (C) 1996 Toyohashi University of Technology
*/

#include <stdio.h>
#include <string.h>
#include "sr.h"
#include "loader.h"
#include "proc.h"

#ifndef MAGIC_NUMBER
#define MAGIC_NUMBER "PMV1.0"
#endif
char magic_number[] = MAGIC_NUMBER;

void loader(FILE *input)
{
  char buf[sizeof(magic_number)];
  word_t *pseg = get_memory_loc(make_pseg_addr(0));
  word_t wbuf;
  int count;
  int len;
  
  bzero(buf, sizeof(buf));
  if (fread(buf, strlen(magic_number), 1, input) <= 0)
    fatal("cannot read magic number");
  if (strcmp(buf, magic_number) != 0)
    fatal("bad magic number");

  count = 0;
  while (fread(&wbuf, sizeof(wbuf), 1, input) == 1) {
    if (count > PSEG_SIZE)
      fatal("PSEG overflow");
    pseg[count++] = wbuf;
  }
}
