/*
  sr.h
  Copyright (C) 1996,2002 Toyohashi University of Technology
*/

typedef signed int word_t;

typedef unsigned short addr_t;

#define PSEG_SIZE (1 << 15)
#define DSEG_SIZE (1 << 15)
#define MEMORY_SIZE ((PSEG_SIZE)+(DSEG_SIZE))
#define PSEG_ADDR_MASK ((addr_t)0x7fff)
#define DSEG_ADDR_MASK ((addr_t)0x8000)
#define make_pseg_addr(ADDR) ((addr_t)((ADDR) & PSEG_ADDR_MASK))
#define make_pseg_offset(ADDR) ((addr_t)((ADDR) & PSEG_ADDR_MASK))
#define make_dseg_addr(ADDR) ((addr_t)((ADDR) | DSEG_ADDR_MASK))
#define pseg_addr_p(ADDR) (((ADDR) & DSEG_ADDR_MASK) == 0)
#define dseg_addr_p(ADDR) (((ADDR) & DSEG_ADDR_MASK) != 0)

void fatal(const char *);

extern int trace_mode;
extern int step_mode;
