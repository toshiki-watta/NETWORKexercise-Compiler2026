/*
  proc.c
  Copyright (C) 1996,2002 Toyohashi University of Technology
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "sr.h"
#include "proc.h"

#define insn_code_mode(WORD) (((unsigned int)(WORD) >> 6) & 0x3)
#define insn_code_regx(WORD) (((unsigned int)(WORD) >> 3) & 0x7)
#define insn_code_regy(WORD) (((unsigned int)(WORD) >> 0) & 0x7)

typedef enum {
  m_ds = 0,
  m_ps = 1,
  m_ds_br = 2
} addr_mode_t;

struct insn_t;

typedef void (*entry_t)(struct insn_t *);

typedef struct insn_t {
  entry_t entry;
  addr_mode_t mode;
  word_t *regx;
  word_t *regy;
  addr_t  addr;
  struct insn_t *next;
} insn_t;

static insn_t *free_insn_list;
static insn_t *alloc_insn(void);
static void free_insn(insn_t *);
/*static void interrupt_handler(int, int, struct sigcontext *, char *);*/
static void interrupt_handler(int);
static word_t expand_signed(addr_t);

typedef int opcode_t;

#define OPTAB_SIZE (1 << 8)
#define NUMBER_OF_REGS 6

#define BR_ADD_COST 2

#define entry_of(NMEM) do_ ## NMEM
#define length(ARRAY) (sizeof(ARRAY)/sizeof(*(ARRAY)))

#define defop(NMEMONIC,TYPE,OPCODE,CYCLE,ACTION) \
static void entry_of(NMEMONIC)(insn_t *);
#include "insn.def"
#undef defop

static entry_t optab[OPTAB_SIZE];
static char *opname[1256];
static char *regname[] = {
    "r0",
    "r1",
    "r2",
    "r3",
    "sp",
    "br",
};

static word_t *memory;
static insn_t **insn_cache;

static word_t regs[NUMBER_OF_REGS];
#define REG_SP (regs[4])
#define REG_BR (regs[5])
static addr_t program_counter = 0;
static addr_t last_program_counter;
static word_t condition_reg = 0;
static int exe_cycle_counter = 0;

#define accum_cycle(CYCLE) (exe_cycle_counter += (CYCLE))

#define defop(NMEMONIC,TYPE,OPCODE,CYCLE,ACTION) {OPCODE,entry_of(NMEMONIC)},
static struct {
  opcode_t code;
  entry_t entry;
} packed_optab[] = {
#include "insn.def"
};
#undef defop

static addr_t mod_addr(addr_mode_t mode, addr_t addr)
{
  switch (mode) {
  case m_ds:
    return make_dseg_addr(addr);
    break;
  case m_ps:
    return make_pseg_addr(addr);
    break;
  case m_ds_br:
    accum_cycle(BR_ADD_COST);
    return make_dseg_addr(expand_signed(addr) + REG_BR);
    break;
  default:
    break;
  }
  fatal("invalid addressing mode");
  return -1;
}

static void store_mem(word_t value, addr_t addr)
{
  if (pseg_addr_p(addr)) {
    addr_t offset = make_pseg_offset(addr);

    if (insn_cache[offset] != 0) {
      free_insn(insn_cache[offset]);
      insn_cache[offset] = 0;
    }
  }
  memory[addr] = value;
}

static word_t expand_signed(addr_t addr)
{
  word_t value = addr;

  if ((value & (1 << 15)) != 0)
    value |= (word_t)0xffff << 16;
  return value;
}

static insn_t *alloc_insn(void)
{
  insn_t *new;

  if (free_insn_list != 0) {
    new = free_insn_list;
    free_insn_list = free_insn_list->next;
  } else {
    new = (insn_t *)malloc(sizeof(insn_t));
    if (new == 0)
      fatal("can't allocate insn cell");
  }
  return new;
}

static void free_insn(insn_t *insn)
{
  insn->next = free_insn_list;
  free_insn_list = insn;
}

static void illegal_insn_entry(insn_t *insn)
{
  char msg[80];
  sprintf(msg, "illegal insn at %d", program_counter);
  fatal(msg);
}

static word_t check_nonzero(word_t word)
{
  if (word == 0)
    fatal ("division by zero");
  return word;
}

static void read_digit(word_t *dest)
{
  static char buf[32];
  char *p;

  fgets(buf, 20, stdin);
  *dest = (word_t)strtol(buf, &p, 10);
  if (p == buf)
    fatal("illegal input: must be digits");
}

#define write_char(WORD) (printf("%c",(WORD)))
#define write_digit(WORD) (printf("%d",(WORD)))

#define defop(NMEMONIC,TYPE,OPCODE,CYCLE,ACTION) \
static void entry_of(NMEMONIC)(insn_t *insn) {ACTION; accum_cycle(CYCLE);}
#define INSN insn
#define MODE(INSN) ((INSN)->mode)
#define REGX(INSN) (*((INSN)->regx))
#define REGY(INSN) (*((INSN)->regy))
#define ADDR(INSN) ((INSN)->addr)
#define IMMED(INSN) (expand_signed(ADDR(INSN)))
#define SET(DEST,SRC) ((DEST)=(SRC))
#define MODADDR(MODE,ADDR) mod_addr(MODE,ADDR)
#define FETCHMEM(ADDR) (memory[(addr_t)(ADDR)])
#define STOREMEM(VALUE,ADDR) store_mem((VALUE),(ADDR))
#define CREG condition_reg
#define PC program_counter
#define SP REG_SP
#define ADD(X,Y) ((X)+(Y))
#define SUB(X,Y) ((X)-(Y))
#define MUL(X,Y) ((X)*(Y))
#define DIV(X,Y) ((X)/check_nonzero(Y))
#define PSEG(ADDR) make_pseg_addr(ADDR)
#define DSEG(ADDR) make_dseg_addr(ADDR)
#define WHEN(COND,ACTION) ((COND) && (ACTION))
#define NEQ(CR) ((CR) != 0)
#define EQ(CR) ((CR) == 0)
#define GT(CR) ((CR) > 0)
#define GEQ(CR) ((CR) >= 0)
#define LT(CR) ((CR) < 0)
#define LEQ(CR) ((CR) <= 0)
#define READ(X) (read_digit(&(X)))
#define WRITEC(X) (write_char(X))
#define WRITED(X) (write_digit(X))
#define HALT() (program_counter = make_dseg_addr(0))
#include "insn.def"
#undef defop

typedef union {
    word_t word;
    struct {
      unsigned char opcode;
      unsigned char mode_regx_regy;
      short addr;
    } code;
  } insn_word_t;

static void print_cmd(insn_word_t word);

static insn_t *decode_insn(word_t word)
{
  insn_t *new = alloc_insn();
  insn_word_t insn_word;
  int insn_mode;
  int insn_regx;
  int insn_regy;

  insn_word.word = word;
  insn_mode = insn_code_mode(insn_word.code.mode_regx_regy);
  insn_regx = insn_code_regx(insn_word.code.mode_regx_regy);
  insn_regy = insn_code_regy(insn_word.code.mode_regx_regy);
  new->entry = optab[(opcode_t)insn_word.code.opcode];
  new->mode = insn_mode;
  if (insn_regx >= NUMBER_OF_REGS)
    fatal("invalid regx field");
  new->regx = &regs[insn_regx];
  if (insn_regy >= NUMBER_OF_REGS)
    fatal("invalid regy field");
  new->regy = &regs[insn_regy];
  new->addr = insn_word.code.addr;
  return new;
}

void init_proc(void)
{
  int i;

  for (i = 0; i < length(optab); i++)
    optab[i] = illegal_insn_entry;

  for (i = 0; i < length(packed_optab); i++)
    optab[packed_optab[i].code] = packed_optab[i].entry;

#define defop(NMEMONIC,TYPE,OPCODE,CYCLE,ACTION) \
  opname[OPCODE] = #NMEMONIC;
#include "insn.def"
#undef defop

  memory = (word_t *)calloc(MEMORY_SIZE, sizeof(word_t));
  if (memory == 0)
    fatal("cannot allocate memory space");
  insn_cache = (insn_t **)calloc(PSEG_SIZE, sizeof(insn_t *));
  if (insn_cache == 0)
    fatal("cannot allocate insn cache");
  free_insn_list = 0;

  program_counter = 0;
  condition_reg = 0;
  exe_cycle_counter = 0;
  REG_SP = (word_t)-1;
  signal(SIGINT, interrupt_handler);
}

void fetch_and_exec(void)
{
  insn_t *insn;
  char inbuf[1024];

  while (pseg_addr_p(program_counter)) {
     if (trace_mode){
         print_info();
     }
     insn = insn_cache[program_counter];
     if (insn == 0) {
       insn = decode_insn(memory[make_pseg_addr(program_counter)]);
       insn_cache[program_counter] = insn;
     }
     if (trace_mode){
         print_cmd((insn_word_t)memory[make_pseg_addr(program_counter)]);
     }
     if (step_mode){
         fgets(inbuf, 1024, stdin);
     }
     last_program_counter = program_counter++;
     (*insn->entry)(insn);
  }
  printf("\nHALT at %d.\n", last_program_counter);
  printf(";;; exec time = %d micro sec.\n", exe_cycle_counter);
}

void print_cmd(insn_word_t word){
    
     printf("fetch %s %s (rx:%s ry:%s) addr/offset/immed:%d\n",
            opname[(unsigned char)word.code.opcode],
            ((insn_code_mode(word.code.mode_regx_regy) == 2)
             ? "(BR)" : ""),
            regname[insn_code_regx(word.code.mode_regx_regy)],
            regname[insn_code_regy(word.code.mode_regx_regy)],
            word.code.addr);
}

void print_info(void){
    int i;
    int mem;

    printf("PC:%d(0x%x) CR:%d(0x%x) time:%d\n",
           program_counter, program_counter,
           condition_reg, condition_reg,
           exe_cycle_counter);
    for (i = 0 ; i < 4 ; i++){
        printf("R%d:%d(0x%x) ", i, regs[i], regs[i]);
    }
    printf("SP:%d(0x%x) ", regs[4], regs[4]);
    printf("BR:%d(0x%x) ", regs[5], regs[5]);
    printf("\n");
    for (i = 0 ; i <= REG_SP ; i++){
        mem = FETCHMEM(DSEG(i));
        printf("  %d %d(%x)\n", i, mem, mem);
    }
}

word_t *get_memory_loc(addr_t addr)
{
  if (memory == 0)
    fatal("memory not allocated yet");
  return &memory[addr];
}

static void
/* interrupt_handler(int sig, int code, struct sigcontext *scp, char *addr)*/
interrupt_handler(int sig)
{
  fprintf(stderr, "\nINTERRUPT at %d.\n", last_program_counter);
  fprintf(stderr, ";;; exec time = %d micro sec.\n", exe_cycle_counter);
  exit(0);
}
