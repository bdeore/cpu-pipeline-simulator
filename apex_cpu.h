#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_

#include "apex_macros.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Format of an APEX instruction  */
typedef struct APEX_Instruction {
  char opcode_str[128];
  int opcode;
  int rd;
  int rs1;
  int rs2;
  int rs3;
  int imm;
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage {
  int pc;
  char opcode_str[128];
  int opcode;
  int rs1;
  int rs2;
  int rs3;
  int rd;
  int imm;
  int rs1_value;
  int rs2_value;
  int rs3_value;
  int result_buffer;
  int memory_address;
  int has_insn;
} CPU_Stage;

/* Model of APEX CPU */
typedef struct APEX_CPU {
  int pc;                        /* Current program counter */
  int clock;                     /* Clock cycles elapsed */
  int insn_completed;            /* Instructions retired */
  int regs[REG_FILE_SIZE];       /* Integer register file */
  int status[REG_FILE_SIZE];     /* status bits for each register in register file */
  int forwarded[REG_FILE_SIZE];  /* status bits to indicate if result has been forwarded */
  int code_memory_size;          /* Number of instruction in the input file */
  APEX_Instruction *code_memory; /* Code Memory */
  int data_memory[DATA_MEMORY_SIZE]; /* Data Memory */
  int single_step;               /* Wait for user input after every cycle */
  int zero_flag;                 /* {TRUE, FALSE} Used by BZ and BNZ to branch */
  int fetch_from_next_cycle;     /* flag to enable disable debug messages */
  int debug_messages;
  /* Pipeline stages */
  CPU_Stage fetch;
  CPU_Stage decode;
  CPU_Stage execute;
  CPU_Stage memory;
  CPU_Stage writeback;
} APEX_CPU;

APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename);
void APEX_cpu_run(APEX_CPU *cpu, int count, bool print_contents);
void APEX_cpu_stop(APEX_CPU *cpu);
void print_arf(APEX_CPU *cpu);
void print_mem(APEX_CPU *cpu);
void display(APEX_CPU *cpu);
void print_stage_contents(CPU_Stage *stage, char *name);
void show_mem(APEX_CPU *cpu, int address);
CPU_Stage get_nop_stage(CPU_Stage *nop);
void forward_data_mem(APEX_CPU *cpu);
void forward_data_ex(APEX_CPU *cpu);

#endif
