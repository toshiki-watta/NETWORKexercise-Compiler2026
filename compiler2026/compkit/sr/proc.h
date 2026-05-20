/*
  proc.h
  Copyright (C) 1996,2002 Toyohashi University of Technology
*/

void init_proc(void);
void fetch_and_exec(void);
word_t *get_memory_loc(addr_t addr);

void print_info(void);
