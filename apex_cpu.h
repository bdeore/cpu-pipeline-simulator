#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_

#include "apex_macros.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

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

typedef struct IQ_Entry {
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
  int cycle_number;
  bool valid;
} IQ_Entry;

/* Format of ROB entry */
typedef struct ROB_Entry {
  bool status;
  char instruction_type[50];
  int pc_value;
  int dest_physical_reg;
  int dest_architectural_reg;
  int mready;
} ROB_Entry;

typedef struct ROB_Queue {
  int head, tail;
  ROB_Entry buffer[ROB_SIZE];
} ROB_Queue;

/* Model of APEX CPU */
typedef struct APEX_CPU {
  int pc;                                       /* Current program counter */
  int clock;                                    /* Clock cycles elapsed */
  int insn_completed;                           /* Instructions retired */
  int regs[REG_FILE_SIZE];                      /* Unified Integer register file */
  int status[REG_FILE_SIZE];                    /* status bits for each register in register file */
  int forwarded[REG_FILE_SIZE];                 /* status bits to indicate if result has been forwarded */
  int rename_table[RENAME_TABLE_SIZE];
  int allocation_list[REG_FILE_SIZE];
  int renamed[REG_FILE_SIZE];
  int iq_entry_used[IQ_SIZE];                   /* status bits to indicate empty issue queue entries */
  IQ_Entry issue_queue[IQ_SIZE];                /* issue queue */
  ROB_Queue reorder_buffer;                     /* reorder buffer */
  APEX_Instruction *code_memory;                /* Code Memory */
  int code_memory_size;                         /* Number of instruction in the input file */
  int data_memory[DATA_MEMORY_SIZE];            /* Data Memory */
  int single_step;                              /* Wait for user input after every cycle */
  int zero_flag;                                /* {TRUE, FALSE} Used by BZ and BNZ to branch */
  int fetch_from_next_cycle;                    /* flag to enable disable debug messages */
  int debug_messages;
  bool rob_full;
  bool iq_full;

  /* Pipeline stages */
  CPU_Stage fetch;
  CPU_Stage decode;
  CPU_Stage execute;
  CPU_Stage memory;
  CPU_Stage intu;
  CPU_Stage jbu1;
  CPU_Stage jbu2;
  CPU_Stage mulu;

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
void forward_data_to_decode(APEX_CPU *cpu, CPU_Stage *stage);
void forward_data_to_iq(APEX_CPU *cpu, CPU_Stage *stage);

void APEX_INTU(APEX_CPU *cpu);

void APEX_issue(APEX_CPU *cpu);
void APEX_dispatch(APEX_CPU *cpu);
ROB_Queue get_reorder_buffer();
bool queue_insert(APEX_CPU *cpu, ROB_Entry rob_entry);
void insert_rob_entry(APEX_CPU *cpu);
bool increment_rob_head(APEX_CPU *cpu);
void insert_iq_entry(APEX_CPU *cpu);
void validate_iq_entry(APEX_CPU *cpu, IQ_Entry *entry);

void print_issue_queue(APEX_CPU *cpu);
void print_reorder_buffer(APEX_CPU *cpu);
void print_rename_table(APEX_CPU *cpu);
static int get_code_memory_index_from_pc(int pc);
static void print_instruction(const CPU_Stage *stage);
static void print_stage_content(const char *name, const CPU_Stage *stage);

int find_free_register(APEX_CPU *cpu);

#endif
