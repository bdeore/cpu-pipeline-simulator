#include "apex_cpu.h"
#include "apex_macros.h"

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc) {
  return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage) {
  switch (stage->opcode) {

    case OPCODE_ADD:
    case OPCODE_SUB:
    case OPCODE_MUL:
    case OPCODE_AND:
    case OPCODE_OR:
    case OPCODE_XOR:
    case OPCODE_LDR: {
      printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
             stage->rs2);
      break;
    }

    case OPCODE_LOAD:
    case OPCODE_SUBL:
    case OPCODE_ADDL: {
      printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
             stage->imm);
      break;
    }

    case OPCODE_STORE: {
      printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
             stage->imm);
      break;
    }

    case OPCODE_STR: {
      printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2,
             stage->rs3);
      break;
    }

    case OPCODE_MOVC: {
      printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
      break;
    }

    case OPCODE_CMP: {
      printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2);
      break;
    }

    case OPCODE_BZ:
    case OPCODE_BNZ: {
      printf("%s,#%d ", stage->opcode_str, stage->imm);
      break;
    }

    case OPCODE_HALT:
    case OPCODE_NOP: {
      printf("%s", stage->opcode_str);
      break;
    }

    case OPCODE_DIV: {
      break;
    }

  }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage) {
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
//static void
//print_reg_file(const APEX_CPU *cpu) {
//
//  printf("----------\n%s\n----------\n", "Register Status:");
//
//  for (int i = 0; i < REG_FILE_SIZE; ++i) {
//    printf("R%-3d[%-3d] ", i, cpu->status[i]);
//    if (i == (REG_FILE_SIZE / 2) - 1) {
//      printf("\n");
//    }
//  }
//
//  printf("\n----------\n%s\n----------\n", "Register Content:");
//
//  for (int i = 0; i < REG_FILE_SIZE; ++i) {
//    printf("R%-3d[%-3d] ", i, cpu->regs[i]);
//    if (i == (REG_FILE_SIZE / 2) - 1) {
//      printf("\n");
//    }
//  }
//
//  printf("\n");
//}

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu) {
  APEX_Instruction *current_ins;

  if (cpu->fetch.has_insn) {
    /* This fetches new branch target instruction from next cycle */
    if (cpu->fetch_from_next_cycle == TRUE) {
      cpu->fetch_from_next_cycle = FALSE;

      /* Update PC for next instruction */
      if (cpu->debug_messages) {
        print_stage_content("Fetch", &cpu->fetch);
      }
      /* Skip this cycle*/
      return;
    }

    /* Store current PC in fetch latch */
    cpu->fetch.pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields
     * into fetch latch  */
    current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
    // printf("opcode_str: %s\n", current_ins->opcode_str);
    strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
    cpu->fetch.opcode = current_ins->opcode;
    cpu->fetch.rd = current_ins->rd;
    cpu->fetch.rs1 = current_ins->rs1;
    cpu->fetch.rs2 = current_ins->rs2;
    cpu->fetch.rs3 = current_ins->rs3;
    cpu->fetch.imm = current_ins->imm;

    /* Update PC for next instruction */
    cpu->pc += 4;

    /* Copy data from fetch latch to decode latch*/
    cpu->decode = cpu->fetch;

    if (cpu->debug_messages) {
      print_stage_content("Fetch", &cpu->fetch);
    }

    /* Stop fetching new instructions if HALT is fetched */
    if (cpu->fetch.opcode == OPCODE_HALT) {
      cpu->fetch.has_insn = FALSE;
    }
  }
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu) {
  bool stall = false;
  if (cpu->decode.has_insn) {
    /* Read operands from register file based on the instruction type */
    switch (cpu->decode.opcode) {
      case OPCODE_ADD:
      case OPCODE_SUB:
      case OPCODE_MUL:
      case OPCODE_AND:
      case OPCODE_OR:
      case OPCODE_XOR:
      case OPCODE_LDR: {
        // check if registers are valid
        if (cpu->status[cpu->decode.rs1] == 1 || cpu->status[cpu->decode.rs2] == 1) {
          stall = true;
        } else {
          // check if result has already been forwarded
          if (cpu->forwarded[cpu->decode.rs1] != 1)
            cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];

          if (cpu->forwarded[cpu->decode.rs2] != 1)
            cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

          // Mark register as invalid
          cpu->status[cpu->decode.rd] = 1;
        }
        break;
      }

      case OPCODE_CMP: {
        if (cpu->status[cpu->decode.rs1] == 1 || cpu->status[cpu->decode.rs2] == 1) {
          stall = true;
        } else {
          if (cpu->forwarded[cpu->decode.rs1] != 1)
            cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];

          if (cpu->forwarded[cpu->decode.rs2] != 1)
            cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
        }
        break;
      }

      case OPCODE_LOAD:
      case OPCODE_ADDL:
      case OPCODE_SUBL: {
        if (cpu->status[cpu->decode.rs1] == 1) {
          stall = true;
        } else {
          if (cpu->forwarded[cpu->decode.rs1] != 1)
            cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];

          cpu->status[cpu->decode.rd] = 1;
        }
        break;
      }

      case OPCODE_STR: {
        if (cpu->status[cpu->decode.rs1] == 1 || cpu->status[cpu->decode.rs2] == 1
            || cpu->status[cpu->decode.rs3] == 1) {
          stall = true;
        } else {
          if (cpu->forwarded[cpu->decode.rs1] != 1)
            cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];

          if (cpu->forwarded[cpu->decode.rs2] != 1)
            cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

          if (cpu->forwarded[cpu->decode.rs3] != 1)
            cpu->decode.rs3_value = cpu->regs[cpu->decode.rs3];
        }
        break;
      }

      case OPCODE_STORE: {
        if (cpu->status[cpu->decode.rs1] == 1 || cpu->status[cpu->decode.rs2] == 1) {
          stall = true;
        } else {
          if (cpu->forwarded[cpu->decode.rs1] != 1)
            cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];

          if (cpu->forwarded[cpu->decode.rs2] != 1)
            cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
        }
        break;
      }

      case OPCODE_MOVC: {
        /* MOVC doesn't have register operands */
        cpu->status[cpu->decode.rd] = 1;
        break;
      }

      case OPCODE_NOP:
      case OPCODE_DIV: {
        // Does Nothing
        break;
      }
    }

    if (!stall) {
      cpu->execute = cpu->decode;
    } else {
      CPU_Stage nop;
      nop = get_nop_stage(&nop);
      cpu->execute = nop;
      cpu->fetch_from_next_cycle = TRUE;
    }

    if (cpu->debug_messages) {
      print_stage_content("Decode/RF", &cpu->decode);
    }
  }
}

/*
 * Execute Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_execute(APEX_CPU *cpu) {

  if (cpu->execute.has_insn) {
    /* Execute logic based on instruction type */
    switch (cpu->execute.opcode) {
      case OPCODE_ADD: {
        cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.rs2_value;
        break;
      }

      case OPCODE_ADDL: {
        cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.imm;
        break;
      }

      case OPCODE_SUB: {
        cpu->execute.result_buffer = cpu->execute.rs1_value - cpu->execute.rs2_value;
        break;
      }

      case OPCODE_SUBL: {
        cpu->execute.result_buffer = cpu->execute.rs1_value - cpu->execute.imm;
        break;
      }

      case OPCODE_MUL: {
        cpu->execute.result_buffer = cpu->execute.rs1_value * cpu->execute.rs2_value;
        break;
      }

      case OPCODE_AND: {
        cpu->execute.result_buffer = (unsigned char) cpu->execute.rs1_value & (unsigned char) cpu->execute.rs2_value;
        break;
      }

      case OPCODE_OR: {
        cpu->execute.result_buffer = (unsigned char) cpu->execute.rs1_value | (unsigned char) cpu->execute.rs2_value;
        break;
      }

      case OPCODE_XOR: {
        cpu->execute.result_buffer = (unsigned char) cpu->execute.rs1_value ^ (unsigned char) cpu->execute.rs2_value;
        break;
      }

      case OPCODE_LOAD: {
        cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.imm;
        //printf("Memory Address for load: %d\n", cpu->execute.memory_address);
        break;
      }

      case OPCODE_LDR: {
        cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.rs2_value;
        break;
      }

      case OPCODE_STORE: {
        cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.imm;
        break;
      }

      case OPCODE_STR: {
        cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.rs3_value;
        break;
      }

      case OPCODE_CMP: {
        if (cpu->execute.rs1_value == cpu->execute.rs2_value) {
          cpu->zero_flag = TRUE;
          //printf("\nCMP Zero flag is set\n");
        } else {
          cpu->zero_flag = FALSE;
        }
        break;
      }

      case OPCODE_MOVC: {
        cpu->execute.result_buffer = cpu->execute.imm + 0;

        /* Set the zero flag based on the result buffer */
        if (cpu->execute.result_buffer == 0) {
          cpu->zero_flag = TRUE;
        } else {
          cpu->zero_flag = FALSE;
        }
        break;
      }

      case OPCODE_BZ: {
        if (cpu->zero_flag == TRUE) {
          /* Calculate new PC, and send it to fetch unit */
          cpu->pc = cpu->execute.pc + cpu->execute.imm;

          /* Since we are using reverse callbacks for pipeline stages,
           * this will prevent the new instruction from being fetched in the current cycle*/
          cpu->fetch_from_next_cycle = TRUE;

          /* Flush previous stages */
          cpu->decode.has_insn = FALSE;

          /* Make sure fetch stage is enabled to start fetching from new PC */
          cpu->fetch.has_insn = TRUE;
        }
        break;
      }

      case OPCODE_BNZ: {
        if (cpu->zero_flag == FALSE) {
          /* Calculate new PC, and send it to fetch unit */
          cpu->pc = cpu->execute.pc + cpu->execute.imm;

          /* Since we are using reverse callbacks for pipeline stages,
           * this will prevent the new instruction from being fetched in the current cycle*/
          cpu->fetch_from_next_cycle = TRUE;

          /* Flush previous stages */
          cpu->decode.has_insn = FALSE;

          /* Make sure fetch stage is enabled to start fetching from new PC */
          cpu->fetch.has_insn = TRUE;
        }
        break;
      }

      case OPCODE_NOP:
      case OPCODE_DIV: {
        // Does Nothing
        break;
      }
    }
    // forward data from ex to decode stage
    forward_data_ex(cpu);

    /* Copy data from execute latch to memory latch*/
    cpu->memory = cpu->execute;
    cpu->execute.has_insn = FALSE;

    if (cpu->debug_messages) {
      print_stage_content("Execute", &cpu->execute);
    }
  }
}

/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_memory(APEX_CPU *cpu) {

  if (cpu->memory.has_insn) {
    switch (cpu->memory.opcode) {
      case OPCODE_ADD:
      case OPCODE_SUB:
      case OPCODE_MUL:
      case OPCODE_ADDL:
      case OPCODE_SUBL:
      case OPCODE_AND:
      case OPCODE_OR:
      case OPCODE_XOR:
      case OPCODE_CMP:
      case OPCODE_MOVC: {
        /* No work needs to be done for above instructions. case statements
         * are written just for the sake of completeness */
        break;
      }

      case OPCODE_LOAD:
      case OPCODE_LDR: {
        cpu->memory.result_buffer = cpu->data_memory[cpu->memory.memory_address];
        break;
      }

      case OPCODE_STORE:
      case OPCODE_STR: {
        if (cpu->memory.memory_address < DATA_MEMORY_SIZE) {
          cpu->data_memory[cpu->memory.memory_address] = cpu->memory.rs1_value;
        } else {
          printf("\nError: Invalid Address Provided for Data Memory\n");
          exit(1);
        }
        //printf("Data Memory: %d rs1_value: %d\n", cpu->memory.memory_address, cpu->memory.rs1_value);
        break;
      }

      case OPCODE_NOP:
      case OPCODE_DIV: {
        // Does Nothing
        break;
      }
    }

    // forward data from mem to decode stage
    forward_data_mem(cpu);

    /* Copy data from memory latch to writeback latch*/
    cpu->writeback = cpu->memory;
    cpu->memory.has_insn = FALSE;

    if (cpu->debug_messages) {
      print_stage_content("Memory", &cpu->memory);
    }
  }
}

/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu) {
  if (cpu->writeback.has_insn) {
    /* Write result to register file based on instruction type */
    switch (cpu->writeback.opcode) {
      case OPCODE_ADD:
      case OPCODE_SUB:
      case OPCODE_MUL:
      case OPCODE_AND:
      case OPCODE_OR:
      case OPCODE_XOR:
      case OPCODE_LDR:
      case OPCODE_LOAD:
      case OPCODE_MOVC:
      case OPCODE_ADDL:
      case OPCODE_SUBL: {
        cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
        cpu->status[cpu->writeback.rd] = 0;
        cpu->forwarded[cpu->writeback.rd] = 0;
        break;
      }

      case OPCODE_STR:
      case OPCODE_STORE:
      case OPCODE_CMP:
      case OPCODE_NOP:
      case OPCODE_DIV: {
        /* No work needs to be done for above instructions. case statements
        * are written just for the sake of completeness */
        break;
      }
    }

    cpu->insn_completed++;
    cpu->writeback.has_insn = FALSE;

    if (cpu->debug_messages) {
      print_stage_content("Writeback", &cpu->writeback);
    }

    if (cpu->writeback.opcode == OPCODE_HALT) {
      /* Stop the APEX simulator */
      return TRUE;
    }
  }

/* Default */
  return 0;
}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename) {
  int i;
  APEX_CPU *cpu;

  if (!filename) {
    return NULL;
  }

  cpu = calloc(1, sizeof(APEX_CPU));

  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
  memset(cpu->status, 0, sizeof(int) * REG_FILE_SIZE);
  memset(cpu->forwarded, 0, sizeof(int) * REG_FILE_SIZE);
  memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);

  cpu->single_step = ENABLE_SINGLE_STEP;
  cpu->clock = 1;

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }
  cpu->debug_messages = 1;

  if (cpu->debug_messages) {
    fprintf(stderr, "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n", cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
    fprintf(stderr, "APEX_CPU: Printing Code Memory\n\n");
    printf("-----------------------------------------------------------\n");
    printf("|  %-9s  %-9s %-9s  %-9s   %-9s |\n", "opcode_str", "rd", "rs1", "rs2",
           "imm");
    printf("-----------------------------------------------------------\n");

    for (i = 0; i < cpu->code_memory_size; ++i) {
      printf("|  %-9s   %-9d  %-9d  %-9d   %-9d|\n", cpu->code_memory[i].opcode_str,
             cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
    }
    printf("-----------------------------------------------------------\n");

  }
  cpu->debug_messages = 0;

  /* To start fetch stage */
  cpu->fetch.has_insn = TRUE;
  // print_reg_file(cpu);
  return cpu;
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_run(APEX_CPU *cpu, int count, bool print_contents) {
  bool run = true;
  int cycle = 0;
  if (count > 0) cpu->single_step = 0;
  if (print_contents) cpu->debug_messages = 1;
  else cpu->debug_messages = 0;

  while (run) {
    if (count == 0) run = false;

    if (cpu->debug_messages) {
      printf("\n--------------------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("--------------------------------------------\n");
    }

    if (APEX_writeback(cpu)) {
      /* Halt in writeback stage */
      // printf("\nAPEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
      break;
    }

    APEX_memory(cpu);
    APEX_execute(cpu);
    APEX_decode(cpu);
    APEX_fetch(cpu);

    if (cycle == count - 1 && count != 0) {
      run = false;
      cpu->single_step = 1;
    }

    cpu->clock++;
    cycle++;
  }
}

/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_stop(APEX_CPU *cpu) {
  free(cpu->code_memory);
  free(cpu);
}

/**
 * Method to print contents of all registers in architectural register file
 *
 * @param cpu  pointer to current instance of cpu
 */
void print_arf(APEX_CPU *cpu) {
  printf("\n------------------------------------------------------------------\n");
  printf("|              State of Architectural Register File              |\n");
  printf("------------------------------------------------------------------\n");

  for (int i = 0; i < REG_FILE_SIZE; ++i) {
    printf("|   Register: R [%2d]    |   Value: %-5d  |   Status: %-7s    |\n",
           i, cpu->regs[i], cpu->status[i] ? "Invalid" : "valid");
  }
  printf("------------------------------------------------------------------\n");

  printf("\n");
}

/**
 * Method to print state of data memory
 * only prints memory locations that contain valid data
 *
 * @param cpu pointer to current instance of cpu
 */
void print_mem(APEX_CPU *cpu) {
  printf("----------------------------------------------\n");
  printf("|            State of Data Memory            |\n");
  printf("----------------------------------------------\n");
  int count = 0;
  for (int i = 0; i < DATA_MEMORY_SIZE; i++) {
    if (cpu->data_memory[i] != 0) {
      printf("|   Memory [%4d]   |   Data Value: %-7d  |\n", i, cpu->data_memory[i]);
      count++;
    }
    if (count == 10) break;
  }
  if (count == 0) {
    printf("|       all memory location are empty        |\n");
  }
  printf("----------------------------------------------\n");
}

/**
 * Method to display state of cpu, contents of each stage, contents of ARF and Data Memory
 *
 * @param cpu pointer to current instance of cpu
 */
void display(APEX_CPU *cpu) {
  printf("\n-------------------------------------------------\n");
  printf("                    CPU Details                  \n");
  printf("-------------------------------------------------\n");
  printf("|   Instructions : %2d    Retired    : %-4d       |\n", cpu->code_memory_size, cpu->insn_completed);
  printf("|   Cycles       : %2d    Zero flag  : %-5s      |\n", cpu->clock, (cpu->zero_flag) ? "True" : "False");

  print_stage_contents(&cpu->fetch, "Fetch");
  print_stage_contents(&cpu->decode, "Decode");
  print_stage_contents(&cpu->execute, "Execute");
  print_stage_contents(&cpu->memory, "Memory");
  print_stage_contents(&cpu->writeback, "Writeback");

  print_arf(cpu);
  print_mem(cpu);
}

/**
 * Method to print contents of each stage i.e. register addresses, values, pc, literals, etc.
 *
 * @param stage - whose contents are to be printed
 * @param name - name of the stage
 */
void print_stage_contents(CPU_Stage *stage, char *name) {
  printf("\n-------------------------------------------------\n");
  printf("                 Stage: %-6s                   \n", name);
  printf("-------------------------------------------------\n");

  printf("|   pc  : %-4d       Opcode         : %-5s     |\n", stage->pc, stage->opcode_str);
  printf("|   rd  : R%-2d        result_buffer  : %-5d     |\n", stage->rd, stage->result_buffer);
  printf("|   rs1 : R%-2d        rs1_value      : %-5d     |\n", stage->rs1, stage->rs1_value);
  printf("|   rs2 : R%-2d        rs2_value      : %-5d     |\n", stage->rs2, stage->rs2_value);
  printf("|   rs3 : R%-2d        rs3_value      : %-5d     |\n", stage->rs3, stage->rs3_value);
  printf("|   imm : %-4d       memory_address : %-5d     |\n", stage->imm, stage->memory_address);

}

/**
 * Method to show contents of data memory location
 *
 * @param cpu pointer to current instance of cpu
 * @param address - of memory location
 */
void show_mem(APEX_CPU *cpu, int address) {
  printf("\n-----------------------------------\n");
  printf("|     Address     |     Content   |\n");
  printf("-----------------------------------\n");
  printf("|     %d          |     %7d    |\n", address, cpu->data_memory[address]);
  printf("-----------------------------------\n");
}

/**
 * Method to create a no-operation stage data
 *
 * @param nop - pointer to the instance of cpu stage
 * @return NOP cpu stage
 */
CPU_Stage get_nop_stage(CPU_Stage *nop) {
  nop->pc = 0;
  nop->has_insn = TRUE;
  nop->opcode = 0x12;
  nop->rs1 = 0;
  nop->rs2 = 0;
  nop->rs3 = 0;
  nop->rs1_value = 0;
  nop->rs2_value = 0;
  nop->rs3_value = 0;
  nop->rd = 0;
  nop->imm = 0;
  nop->result_buffer = 0;
  nop->memory_address = 0;
  strcpy(nop->opcode_str, "NOP");
  return *nop;
}

/**
 * Method to forward data from execute stage to decode/rf stage
 *
 * @param cpu pointer to current instance of cpu
 */
void forward_data_ex(APEX_CPU *cpu) {
  if (!(cpu->execute.opcode == 0x8 || cpu->execute.opcode == 0x9 || cpu->execute.opcode == 0xf
      || cpu->execute.opcode == 0x10)) {
    switch (cpu->decode.opcode) {
      case OPCODE_ADD:
      case OPCODE_SUB:
      case OPCODE_MUL:
      case OPCODE_AND:
      case OPCODE_OR:
      case OPCODE_XOR:
      case OPCODE_LDR:
      case OPCODE_CMP:
      case OPCODE_MOVC: {
        if (cpu->execute.rd == cpu->decode.rs1) {
          cpu->decode.rs1_value = cpu->execute.result_buffer;
          cpu->forwarded[cpu->execute.rd] = 1;
          cpu->status[cpu->execute.rd] = 0;
        }

        if (cpu->execute.rd == cpu->decode.rs2) {
          cpu->decode.rs2_value = cpu->execute.result_buffer;
          cpu->forwarded[cpu->execute.rd] = 1;
          cpu->status[cpu->execute.rd] = 0;
        }
        break;
      }

      case OPCODE_ADDL:
      case OPCODE_SUBL:
      case OPCODE_LOAD: {
        if (cpu->execute.rd == cpu->decode.rs1) {
          cpu->decode.rs1_value = cpu->execute.result_buffer;
          cpu->forwarded[cpu->execute.rd] = 1;
          cpu->status[cpu->execute.rd] = 0;
        }
        break;
      }

      case OPCODE_STORE: {
        if (cpu->execute.rd == cpu->decode.rs2) {
          cpu->decode.rs2_value = cpu->execute.result_buffer;
          cpu->forwarded[cpu->execute.rd] = 1;
          cpu->status[cpu->execute.rd] = 0;
        }
        break;
      }

      case OPCODE_STR: {
        if (cpu->execute.rd == cpu->decode.rs3) {
          cpu->decode.rs3_value = cpu->execute.result_buffer;
          cpu->forwarded[cpu->execute.rd] = 1;
          cpu->status[cpu->execute.rd] = 0;
        }

        if (cpu->execute.rd == cpu->decode.rs2) {
          cpu->decode.rs2_value = cpu->execute.result_buffer;
          cpu->forwarded[cpu->execute.rd] = 1;
          cpu->status[cpu->execute.rd] = 0;
        }

        if (cpu->execute.rd == cpu->decode.rs1) {
          cpu->decode.rs1_value = cpu->execute.result_buffer;
          cpu->forwarded[cpu->execute.rd] = 1;
          cpu->status[cpu->execute.rd] = 0;
        }
        break;
      }
    }
  }
}

/**
 * Method to forward data from memory stage to decode/rf stage
 *
 * @param cpu pointer to current instance of cpu
 */
void forward_data_mem(APEX_CPU *cpu) {
  if (!(cpu->memory.opcode == 0x9 || cpu->memory.opcode == 0x10)) {
    switch (cpu->decode.opcode) {
      case OPCODE_ADD:
      case OPCODE_SUB:
      case OPCODE_MUL:
      case OPCODE_AND:
      case OPCODE_OR:
      case OPCODE_XOR:
      case OPCODE_LDR:
      case OPCODE_CMP:
      case OPCODE_MOVC: {
        if (cpu->memory.rd == cpu->decode.rs1) {
          cpu->decode.rs1_value = cpu->memory.result_buffer;
          cpu->forwarded[cpu->memory.rd] = 1;
          cpu->status[cpu->memory.rd] = 0;
        } else if (cpu->memory.rd == cpu->decode.rs2) {
          cpu->decode.rs2_value = cpu->memory.result_buffer;
          cpu->forwarded[cpu->memory.rd] = 1;
          cpu->status[cpu->memory.rd] = 0;
        }
        break;
      }

      case OPCODE_ADDL:
      case OPCODE_SUBL:
      case OPCODE_LOAD: {
        if (cpu->memory.rd == cpu->decode.rs1) {
          cpu->decode.rs1_value = cpu->memory.result_buffer;
          cpu->forwarded[cpu->memory.rd] = 1;
          cpu->status[cpu->memory.rd] = 0;
        }
        break;
      }

      case OPCODE_STORE: {
        if (cpu->memory.rd == cpu->decode.rs2) {
          cpu->decode.rs2_value = cpu->memory.result_buffer;
          cpu->forwarded[cpu->memory.rd] = 1;
          cpu->status[cpu->memory.rd] = 0;
        }
        break;
      }

      case OPCODE_STR: {
        if (cpu->memory.rd == cpu->decode.rs3) {
          cpu->decode.rs3_value = cpu->memory.result_buffer;
          cpu->status[cpu->memory.rd] = 0;
        } else if (cpu->memory.rd == cpu->decode.rs2) {
          cpu->decode.rs2_value = cpu->memory.result_buffer;
          cpu->forwarded[cpu->memory.rd] = 1;
          cpu->status[cpu->memory.rd] = 0;
        }
        break;
      }
    }
  }
}
