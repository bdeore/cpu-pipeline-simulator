#include "apex_cpu.h"
#include "apex_macros.h"

void APEX_dispatch(APEX_CPU *cpu) {
  if (cpu->rob_full || cpu->iq_full) {
    cpu->fetch_from_next_cycle = TRUE;

    if (cpu->rob_full)
      printf("\nROB is full\n");
    else if (cpu->iq_full)
      printf("\nIQ is full\n");
  } else {

//    insert_rob_entry(cpu);
    insert_iq_entry(cpu);

  }
}

void APEX_issue(APEX_CPU *cpu) {
  cpu->intu = pick_entry(cpu, "intu");
  if (cpu->mulu_count == 0)
    cpu->mulu = pick_entry(cpu, "mulu");
}

CPU_Stage pick_entry(APEX_CPU *cpu, char *function_unit) {
  IQ_Entry *iq_entry;
  CPU_Stage nop;
  int earliest_request = 999999999;

  for (int i = 0; i <= IQ_SIZE; i++) {
    if (i == IQ_SIZE - 1) {
      if (earliest_request == 999999999) {
        nop = get_nop_stage(&nop);
        return nop;
      } else {
        return remove_iq_entry(cpu, earliest_request);
      }
    } else {
      if (cpu->iq_entry_used[i] == 1) {
        iq_entry = &cpu->issue_queue[i];
        if (iq_entry->valid) {

          if (strcmp(function_unit, "intu") == 0) {
            if (!(iq_entry->opcode == 8 || iq_entry->opcode == 9 || iq_entry->opcode == 15 || iq_entry->opcode == 16
                || iq_entry->opcode == 2 || iq_entry->opcode == 0xa || iq_entry->opcode == 0xb
                || iq_entry->opcode == 0xc || iq_entry->opcode == 0x12)) {
              if (iq_entry->cycle_number < earliest_request) {
                earliest_request = i;
              }
            }
          } else if (strcmp(function_unit, "mulu") == 0) {
            if (iq_entry->opcode == 2) {
              if (iq_entry->cycle_number < earliest_request) {
                earliest_request = i;
              }
            }
          }
        }
      }
    }
  }
  return get_nop_stage(&nop);
}

void insert_iq_entry(APEX_CPU *cpu) {
  for (int i = 0; i < IQ_SIZE; i++) {
    if (i == IQ_SIZE - 1) {
      cpu->iq_full = true;
      cpu->fetch_from_next_cycle = TRUE;
    } else {
      if (cpu->iq_entry_used[i] == 0) {
        IQ_Entry *iq_entry = &cpu->issue_queue[i];

        switch (cpu->decode.opcode) {
          case OPCODE_ADD:
          case OPCODE_SUB:
          case OPCODE_MUL:
          case OPCODE_AND:
          case OPCODE_OR:
          case OPCODE_XOR:
          case OPCODE_LDR: {
            iq_entry->pc = cpu->decode.pc;
            iq_entry->opcode = cpu->decode.opcode;
            strcpy(iq_entry->opcode_str, cpu->decode.opcode_str);
            iq_entry->rs1 = cpu->decode.rs1;
            iq_entry->rs2 = cpu->decode.rs2;
            iq_entry->rd = cpu->decode.rd;
            iq_entry->rs1_value = cpu->decode.rs1_value;
            iq_entry->rs2_value = cpu->decode.rs2_value;
            iq_entry->cycle_number = cpu->clock;
            cpu->iq_entry_used[i] = 1;
            break;
          }

          case OPCODE_LOAD:
          case OPCODE_ADDL:
          case OPCODE_SUBL: {
            iq_entry->pc = cpu->decode.pc;
            iq_entry->opcode = cpu->decode.opcode;
            strcpy(iq_entry->opcode_str, cpu->decode.opcode_str);
            iq_entry->rs1 = cpu->decode.rs1;
            iq_entry->rd = cpu->decode.rd;
            iq_entry->imm = cpu->decode.imm;
            iq_entry->rs1_value = cpu->decode.rs1_value;
            iq_entry->cycle_number = cpu->clock;
            cpu->iq_entry_used[i] = 1;

            break;
          }

          case OPCODE_STORE: {
            iq_entry->pc = cpu->decode.pc;
            iq_entry->opcode = cpu->decode.opcode;
            strcpy(iq_entry->opcode_str, cpu->decode.opcode_str);
            iq_entry->rs1 = cpu->decode.rs1;
            iq_entry->rs2 = cpu->decode.rs2;
            iq_entry->imm = cpu->decode.imm;
            iq_entry->rs1_value = cpu->decode.rs1_value;
            iq_entry->rs2_value = cpu->decode.rs2_value;
            iq_entry->cycle_number = cpu->clock;
            cpu->iq_entry_used[i] = 1;

            break;
          }
          case OPCODE_STR: {
            iq_entry->pc = cpu->decode.pc;
            iq_entry->opcode = cpu->decode.opcode;
            strcpy(iq_entry->opcode_str, cpu->decode.opcode_str);
            iq_entry->rs1 = cpu->decode.rs1;
            iq_entry->rs2 = cpu->decode.rs2;
            iq_entry->rs3 = cpu->decode.rs3;
            iq_entry->rs1_value = cpu->decode.rs1_value;
            iq_entry->rs2_value = cpu->decode.rs2_value;
            iq_entry->rs3_value = cpu->decode.rs3_value;
            iq_entry->cycle_number = cpu->clock;
            cpu->iq_entry_used[i] = 1;
            break;
          }

          case OPCODE_CMP: {
            iq_entry->pc = cpu->decode.pc;
            iq_entry->opcode = cpu->decode.opcode;
            strcpy(iq_entry->opcode_str, cpu->decode.opcode_str);
            iq_entry->rs1 = cpu->decode.rs1;
            iq_entry->rs2 = cpu->decode.rs2;
            iq_entry->rs1_value = cpu->decode.rs1_value;
            iq_entry->rs2_value = cpu->decode.rs2_value;
            iq_entry->cycle_number = cpu->clock;
            cpu->iq_entry_used[i] = 1;
            break;
          }

          case OPCODE_MOVC: {
            iq_entry->pc = cpu->decode.pc;
            iq_entry->opcode = cpu->decode.opcode;
            strcpy(iq_entry->opcode_str, cpu->decode.opcode_str);
            iq_entry->rd = cpu->decode.rd;
            iq_entry->imm = cpu->decode.imm;
            iq_entry->cycle_number = cpu->clock;
            cpu->iq_entry_used[i] = 1;

            break;
          }
        }
        validate_iq_entry(cpu, iq_entry);

        break;
      }
    }
  }
}

CPU_Stage remove_iq_entry(APEX_CPU *cpu, int entry_index) {
  CPU_Stage stage;
  IQ_Entry *iq_entry = &cpu->issue_queue[entry_index];

  switch (iq_entry->opcode) {
    case OPCODE_ADD:
    case OPCODE_SUB:
    case OPCODE_MUL:
    case OPCODE_AND:
    case OPCODE_OR:
    case OPCODE_XOR:
    case OPCODE_LDR: {
      stage.pc = iq_entry->pc;
      stage.opcode = iq_entry->opcode;
      strcpy(stage.opcode_str, iq_entry->opcode_str);
      stage.rs1 = iq_entry->rs1;
      stage.rs2 = iq_entry->rs2;
      stage.rd = iq_entry->rd;
      stage.rs1_value = iq_entry->rs1_value;
      stage.rs2_value = iq_entry->rs2_value;

      cpu->iq_entry_used[entry_index] = 0;
      break;
    }

    case OPCODE_LOAD:
    case OPCODE_ADDL:
    case OPCODE_SUBL: {
      stage.pc = iq_entry->pc;
      stage.opcode = iq_entry->opcode;
      strcpy(stage.opcode_str, iq_entry->opcode_str);
      stage.rs1 = iq_entry->rs1;
      stage.rd = iq_entry->rd;
      stage.imm = iq_entry->imm;
      stage.rs1_value = iq_entry->rs1_value;

      cpu->iq_entry_used[entry_index] = 0;
      break;
    }

    case OPCODE_STORE: {
      stage.pc = iq_entry->pc;
      stage.opcode = iq_entry->opcode;
      strcpy(stage.opcode_str, iq_entry->opcode_str);
      stage.rs1 = iq_entry->rs1;
      stage.rs2 = iq_entry->rs2;
      stage.imm = iq_entry->imm;
      stage.rs1_value = iq_entry->rs1_value;
      stage.rs2_value = iq_entry->rs2_value;

      cpu->iq_entry_used[entry_index] = 0;
      break;
    }

    case OPCODE_STR: {
      stage.pc = iq_entry->pc;
      stage.opcode = iq_entry->opcode;
      strcpy(stage.opcode_str, iq_entry->opcode_str);
      stage.rs1 = iq_entry->rs1;
      stage.rs2 = iq_entry->rs2;
      stage.rs3 = iq_entry->rs3;
      stage.rs1_value = iq_entry->rs1_value;
      stage.rs2_value = iq_entry->rs2_value;
      stage.rs3_value = iq_entry->rs3_value;

      cpu->iq_entry_used[entry_index] = 0;
      break;
    }

    case OPCODE_CMP: {
      stage.pc = iq_entry->pc;
      stage.opcode = iq_entry->opcode;
      strcpy(stage.opcode_str, iq_entry->opcode_str);
      stage.rs1 = iq_entry->rs1;
      stage.rs2 = iq_entry->rs2;
      stage.rs1_value = iq_entry->rs1_value;
      stage.rs2_value = iq_entry->rs2_value;

      cpu->iq_entry_used[entry_index] = 0;
      break;
    }

    case OPCODE_MOVC: {
      stage.pc = iq_entry->pc;
      stage.opcode = iq_entry->opcode;
      strcpy(stage.opcode_str, iq_entry->opcode_str);
      stage.rd = iq_entry->rd;
      stage.imm = iq_entry->imm;

      cpu->iq_entry_used[entry_index] = 0;
      break;
    }
  }
  return stage;
}

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
  if (cpu->decode.has_insn) {
    if (find_free_register(cpu) == -1) {
      cpu->fetch_from_next_cycle = TRUE;
    } else {
      /* Read operands from register file based on the instruction type */
      switch (cpu->decode.opcode) {
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        case OPCODE_LDR: {

          if (cpu->rename_table[cpu->decode.rd] != -1) {
            cpu->renamed[cpu->rename_table[cpu->decode.rd]] = 1;
          }

          cpu->decode.rs1 = cpu->rename_table[cpu->decode.rs1];
          cpu->decode.rs2 = cpu->rename_table[cpu->decode.rs2];

          int physical_register = find_free_register(cpu);
          if (physical_register != -1) {
            cpu->rename_table[cpu->decode.rd] = physical_register;
            cpu->decode.rd = physical_register;
            cpu->allocation_list[physical_register] = 1;
          }

          if (cpu->forwarded[cpu->decode.rs1] != 1) {
            if (cpu->status[cpu->decode.rs1] && cpu->allocation_list[cpu->decode.rs1])
              cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
          }

          if (cpu->forwarded[cpu->decode.rs2] != 1) {
            if (cpu->status[cpu->decode.rs2] && cpu->allocation_list[cpu->decode.rs2])
              cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
          }
          cpu->status[cpu->decode.rd] = 0;
          break;
        }

        case OPCODE_CMP: {

          cpu->decode.rs1 = cpu->rename_table[cpu->decode.rs1];
          cpu->decode.rs2 = cpu->rename_table[cpu->decode.rs2];

          if (cpu->forwarded[cpu->decode.rs1] != 1) {
            if (cpu->status[cpu->decode.rs1] && cpu->allocation_list[cpu->decode.rs1])
              cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
          }

          if (cpu->forwarded[cpu->decode.rs2] != 1) {
            if (cpu->status[cpu->decode.rs2] && cpu->allocation_list[cpu->decode.rs2])
              cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
          }
          break;
        }

        case OPCODE_LOAD:
        case OPCODE_ADDL:
        case OPCODE_SUBL: {

          if (cpu->rename_table[cpu->decode.rd] != -1) {
            cpu->renamed[cpu->rename_table[cpu->decode.rd]] = 1;
          }

          cpu->decode.rs1 = cpu->rename_table[cpu->decode.rs1];

          int physical_register = find_free_register(cpu);
          if (physical_register != -1) {
            cpu->rename_table[cpu->decode.rd] = physical_register;
            cpu->decode.rd = physical_register;
            cpu->allocation_list[physical_register] = 1;
          }
          if (cpu->forwarded[cpu->decode.rs1] != 1) {
            if (cpu->status[cpu->decode.rs1] && cpu->allocation_list[cpu->decode.rs1])
              cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
          }

          cpu->status[cpu->decode.rd] = 0;
          break;
        }

        case OPCODE_STR: {

          cpu->decode.rs1 = cpu->rename_table[cpu->decode.rs1];
          cpu->decode.rs2 = cpu->rename_table[cpu->decode.rs2];
          cpu->decode.rs3 = cpu->rename_table[cpu->decode.rs3];

          if (cpu->forwarded[cpu->decode.rs1] != 1) {
            if (cpu->status[cpu->decode.rs1] && cpu->allocation_list[cpu->decode.rs1])
              cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
          }

          if (cpu->forwarded[cpu->decode.rs2] != 1) {
            if (cpu->status[cpu->decode.rs2] && cpu->allocation_list[cpu->decode.rs2])
              cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
          }

          if (cpu->forwarded[cpu->decode.rs3] != 1) {
            if (cpu->status[cpu->decode.rs3] && cpu->allocation_list[cpu->decode.rs3])
              cpu->decode.rs3_value = cpu->regs[cpu->decode.rs3];
          }

          break;
        }

        case OPCODE_STORE: {

          cpu->decode.rs1 = cpu->rename_table[cpu->decode.rs1];
          cpu->decode.rs2 = cpu->rename_table[cpu->decode.rs2];

          if (cpu->forwarded[cpu->decode.rs1] != 1) {
            if (cpu->status[cpu->decode.rs1] && cpu->allocation_list[cpu->decode.rs1])
              cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
          }

          if (cpu->forwarded[cpu->decode.rs2] != 1) {
            if (cpu->status[cpu->decode.rs2] && cpu->allocation_list[cpu->decode.rs2])
              cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
          }

          break;
        }

        case OPCODE_MOVC: {
          /* MOVC doesn't have register operands */

          if (cpu->rename_table[cpu->decode.rd] != -1) {
            cpu->renamed[cpu->rename_table[cpu->decode.rd]] = 1;
          }

          int physical_register = find_free_register(cpu);
          if (physical_register != -1) {
            cpu->rename_table[cpu->decode.rd] = physical_register;
            cpu->decode.rd = physical_register;
            cpu->allocation_list[physical_register] = 1;
          }

          cpu->status[cpu->decode.rd] = 0;
          break;
        }

        case OPCODE_NOP:
        case OPCODE_DIV: {
          // Does Nothing
          break;
        }
      }

//    print_rename_table(cpu);
      APEX_dispatch(cpu);
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
  APEX_issue(cpu);

  APEX_MULU(cpu);
  APEX_INTU(cpu);
}

void APEX_INTU(APEX_CPU *cpu) {
  if (cpu->intu.has_insn) {
    /* Execute logic based on instruction type */
    switch (cpu->intu.opcode) {
      case OPCODE_ADD: {
//        cpu->intu.rs1_value = cpu->regs[cpu->intu.rs1];
//        cpu->intu.rs2_value = cpu->regs[cpu->intu.rs2];

        cpu->intu.result_buffer = cpu->intu.rs1_value + cpu->intu.rs2_value;

        cpu->regs[cpu->intu.rd] = cpu->intu.result_buffer;
        if (cpu->renamed[cpu->intu.rd] == 1) {
          cpu->allocation_list[cpu->intu.rd] = 0;
        } else {
          cpu->status[cpu->intu.rd] = 1;
        }
        break;
      }

      case OPCODE_ADDL: {
//        cpu->intu.rs1_value = cpu->regs[cpu->intu.rs1];

        cpu->intu.result_buffer = cpu->intu.rs1_value + cpu->intu.imm;

        cpu->regs[cpu->intu.rd] = cpu->intu.result_buffer;
        if (cpu->renamed[cpu->intu.rd] == 1) {
          cpu->allocation_list[cpu->intu.rd] = 0;
        } else {
          cpu->status[cpu->intu.rd] = 1;
        }
        break;
      }

      case OPCODE_SUB: {
//        cpu->intu.rs1_value = cpu->regs[cpu->intu.rs1];
//        cpu->intu.rs2_value = cpu->regs[cpu->intu.rs2];

        cpu->intu.result_buffer = cpu->intu.rs1_value - cpu->intu.rs2_value;

        cpu->regs[cpu->intu.rd] = cpu->intu.result_buffer;
        if (cpu->renamed[cpu->intu.rd] == 1) {
          cpu->allocation_list[cpu->intu.rd] = 0;
        } else {
          cpu->status[cpu->intu.rd] = 1;
        }
        break;
      }

      case OPCODE_SUBL: {
//        cpu->intu.rs1_value = cpu->regs[cpu->intu.rs1];

        cpu->intu.result_buffer = cpu->intu.rs1_value - cpu->intu.imm;

        cpu->regs[cpu->intu.rd] = cpu->intu.result_buffer;
        if (cpu->renamed[cpu->intu.rd] == 1) {
          cpu->allocation_list[cpu->intu.rd] = 0;
        } else {
          cpu->status[cpu->intu.rd] = 1;
        }
        break;
      }

      case OPCODE_MUL: {
//        cpu->intu.rs1_value = cpu->regs[cpu->intu.rs1];
//        cpu->intu.rs2_value = cpu->regs[cpu->intu.rs2];

        cpu->intu.result_buffer = cpu->intu.rs1_value * cpu->intu.rs2_value;

        cpu->regs[cpu->intu.rd] = cpu->intu.result_buffer;
        if (cpu->renamed[cpu->intu.rd] == 1) {
          cpu->allocation_list[cpu->intu.rd] = 0;
        } else {
          cpu->status[cpu->intu.rd] = 1;
        }
        break;
      }

      case OPCODE_AND: {
//        cpu->intu.rs1_value = cpu->regs[cpu->intu.rs1];
//        cpu->intu.rs2_value = cpu->regs[cpu->intu.rs2];

        cpu->intu.result_buffer = (unsigned char) cpu->intu.rs1_value & (unsigned char) cpu->intu.rs2_value;

        cpu->regs[cpu->intu.rd] = cpu->intu.result_buffer;
        if (cpu->renamed[cpu->intu.rd] == 1) {
          cpu->allocation_list[cpu->intu.rd] = 0;
        } else {
          cpu->status[cpu->intu.rd] = 1;
        }
        break;
      }

      case OPCODE_OR: {
//        cpu->intu.rs1_value = cpu->regs[cpu->intu.rs1];
//        cpu->intu.rs2_value = cpu->regs[cpu->intu.rs2];

        cpu->intu.result_buffer = (unsigned char) cpu->intu.rs1_value | (unsigned char) cpu->intu.rs2_value;

        cpu->regs[cpu->intu.rd] = cpu->intu.result_buffer;
        if (cpu->renamed[cpu->intu.rd] == 1) {
          cpu->allocation_list[cpu->intu.rd] = 0;
        } else {
          cpu->status[cpu->intu.rd] = 1;
        }
        break;
      }

      case OPCODE_XOR: {
//        cpu->intu.rs1_value = cpu->regs[cpu->intu.rs1];
//        cpu->intu.rs2_value = cpu->regs[cpu->intu.rs2];

        cpu->intu.result_buffer = (unsigned char) cpu->intu.rs1_value ^ (unsigned char) cpu->intu.rs2_value;

        cpu->regs[cpu->intu.rd] = cpu->intu.result_buffer;
        if (cpu->renamed[cpu->intu.rd] == 1) {
          cpu->allocation_list[cpu->intu.rd] = 0;
        } else {
          cpu->status[cpu->intu.rd] = 1;
        }
        break;
      }

      case OPCODE_LOAD: {
//        cpu->intu.rs1_value = cpu->regs[cpu->intu.rs1];

        cpu->intu.memory_address = cpu->intu.rs1_value + cpu->intu.imm;
        //printf("Memory Address for load: %d\n", cpu->execute.memory_address);
        break;
      }

      case OPCODE_LDR: {
//        cpu->intu.rs1_value = cpu->regs[cpu->intu.rs1];
//        cpu->intu.rs2_value = cpu->regs[cpu->intu.rs2];

        cpu->intu.memory_address = cpu->intu.rs1_value + cpu->intu.rs2_value;
        break;
      }

      case OPCODE_STORE: {
//        cpu->intu.rs2_value = cpu->regs[cpu->intu.rs2];

        cpu->intu.memory_address = cpu->intu.rs2_value + cpu->intu.imm;
        break;
      }

      case OPCODE_STR: {
//        cpu->intu.rs2_value = cpu->regs[cpu->intu.rs2];
//        cpu->intu.rs3_value = cpu->regs[cpu->intu.rs3];

        cpu->intu.memory_address = cpu->intu.rs2_value + cpu->intu.rs3_value;
        break;
      }

      case OPCODE_CMP: {
        if (cpu->intu.rs1_value == cpu->intu.rs2_value) {
          cpu->zero_flag = TRUE;
          //printf("\nCMP Zero flag is set\n");
        } else {
          cpu->zero_flag = FALSE;
        }
        break;
      }

      case OPCODE_MOVC: {
        cpu->intu.result_buffer = cpu->intu.imm + 0;
        cpu->regs[cpu->intu.rd] = cpu->intu.result_buffer;

        if (cpu->renamed[cpu->intu.rd] == 1) {
          cpu->allocation_list[cpu->intu.rd] = 0;
        } else {
          cpu->status[cpu->intu.rd] = 1;
        }
        /* Set the zero flag based on the result buffer */
        if (cpu->intu.result_buffer == 0) {
          cpu->zero_flag = TRUE;
        } else {
          cpu->zero_flag = FALSE;
        }
        break;
      }

      case OPCODE_BZ: {
        if (cpu->zero_flag == TRUE) {
          /* Calculate new PC, and send it to fetch unit */
          cpu->pc = cpu->intu.pc + cpu->intu.imm;

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
          cpu->pc = cpu->intu.pc + cpu->intu.imm;

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

    cpu->insn_completed++;

    forward_data_to_decode(cpu, &cpu->intu);
    forward_data_to_iq(cpu, &cpu->intu);
    /* Copy data from execute latch to memory latch*/
//    cpu->memory = cpu->intu;

    if (cpu->debug_messages) {
      print_stage_content("INTU", &cpu->intu);
    }

    cpu->intu.has_insn = FALSE;
  }
}

void APEX_MULU(APEX_CPU *cpu) {
  if (cpu->mulu.opcode == 2) {
    if (cpu->mulu_count == 2) {
      cpu->mulu.result_buffer = cpu->mulu.rs1_value * cpu->mulu.rs2_value;
      cpu->mulu_count++;
      cpu->mulu_count %= 3;
      cpu->regs[cpu->mulu.rd] = cpu->mulu.result_buffer;
      cpu->status[cpu->mulu.rd] = 1;
      cpu->mulu.has_insn = FALSE;
      forward_data_to_decode(cpu, &cpu->mulu);
      forward_data_to_iq(cpu, &cpu->mulu);

    } else {
      cpu->mulu_count++;
    }
  }
  if (cpu->debug_messages) {
    print_stage_content("MULU", &cpu->mulu);
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


    /* Copy data from memory latch to writeback latch*/
//    cpu->writeback = cpu->memory;
    cpu->memory.has_insn = FALSE;

    if (cpu->debug_messages) {
      print_stage_content("Memory", &cpu->memory);
    }
  }
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
  memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
  memset(cpu->iq_entry_used, 0, sizeof(int) * DATA_MEMORY_SIZE);
  memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
  memset(cpu->status, 0, sizeof(int) * REG_FILE_SIZE);
  memset(cpu->rename_table, -1, sizeof(int) * RENAME_TABLE_SIZE);
  memset(cpu->allocation_list, 0, sizeof(int) * REG_FILE_SIZE);
  memset(cpu->renamed, 0, sizeof(int) * REG_FILE_SIZE);

  cpu->single_step = ENABLE_SINGLE_STEP;
  cpu->clock = 1;
  cpu->reorder_buffer = get_reorder_buffer();
  cpu->rob_full = false;
  cpu->iq_full = false;
  cpu->mulu_count = 0;

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

    if (cpu->decode.opcode == 0xc) {
      /* Halt in writeback stage */
      printf("\nAPEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
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
  print_rename_table(cpu);
  printf("\n------------------------------------------------------------------\n");
  printf("|              State of Architectural Register File              |\n");
  printf("------------------------------------------------------------------\n");

  for (int i = 0; i < REG_FILE_SIZE; ++i) {
    printf("|   Register: R [%2d]    |   Value: %-5d  |   Status: %-7s    |\n", i,
           cpu->regs[cpu->rename_table[i]],
           ((cpu->allocation_list[i]) && (cpu->status[i])) ? "Valid" : "Invalid");
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
//  print_stage_contents(&cpu->writeback, "Writeback");

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

  printf("|   pc  : %4d       Opcode         : %-5s     |\n", stage->pc, stage->opcode_str);
  printf("|   rd  : R%-2d        result_buffer  : %-5d     |\n", stage->rd, stage->result_buffer);
  printf("|   rs1 : R%-2d        rs1_value      : %-5d     |\n", stage->rs1, stage->rs1_value);
  printf("|   rs2 : R%-2d        rs2_value      : %-5d     |\n", stage->rs2, stage->rs2_value);
  printf("|   rs3 : R%-2d        rs3_value      : %-5d     |\n", stage->rs3, stage->rs3_value);
  printf("|   imm : R%-2d        memory_address : %-5d     |\n", stage->imm, stage->memory_address);

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
  nop->rs1 = -1;
  nop->rs2 = -1;
  nop->rs3 = -1;
  nop->rs1_value = 0;
  nop->rs2_value = 0;
  nop->rs3_value = 0;
  nop->rd = -1;
  nop->imm = 0;
  nop->result_buffer = 0;
  nop->memory_address = 0;
  strcpy(nop->opcode_str, "NOP");
  return *nop;
}

ROB_Queue get_reorder_buffer() {
  ROB_Queue queue;
  queue.head = -1;
  queue.tail = 0;
  return queue;
}

bool queue_insert(APEX_CPU *cpu, ROB_Entry rob_entry) {
  if (cpu->reorder_buffer.tail == 64) cpu->reorder_buffer.tail %= 64;
  else if (cpu->reorder_buffer.head == 64) cpu->reorder_buffer.head %= 64;

  if (cpu->reorder_buffer.tail == cpu->reorder_buffer.head) {
    cpu->rob_full = true;
    return false;
  }

  if (cpu->reorder_buffer.head == -1) {
    cpu->reorder_buffer.head = 0;
    cpu->reorder_buffer.tail = 0;
  }

  strcpy(cpu->reorder_buffer.buffer[cpu->reorder_buffer.tail].instruction_type, rob_entry.instruction_type);
  cpu->reorder_buffer.buffer[cpu->reorder_buffer.tail].pc_value = rob_entry.pc_value;
  cpu->reorder_buffer.buffer[cpu->reorder_buffer.tail].status = rob_entry.status;
  cpu->reorder_buffer.buffer[cpu->reorder_buffer.tail].dest_architectural_reg = rob_entry.dest_architectural_reg;
  cpu->reorder_buffer.buffer[cpu->reorder_buffer.tail].dest_physical_reg = rob_entry.dest_physical_reg;

  return true;
}

bool increment_rob_head(APEX_CPU *cpu) {
  cpu->reorder_buffer.head++;
  if (cpu->reorder_buffer.head == ROB_SIZE) cpu->reorder_buffer.head %= ROB_SIZE;
  return true;
}

bool increment_rob_tail(APEX_CPU *cpu) {
  cpu->reorder_buffer.tail++;
  if (cpu->reorder_buffer.tail == ROB_SIZE) cpu->reorder_buffer.tail %= ROB_SIZE;
  return true;
}

void insert_rob_entry(APEX_CPU *cpu) {
  ROB_Entry rob_entry;
  rob_entry.status = false;
  rob_entry.dest_physical_reg = cpu->decode.rd;
  rob_entry.pc_value = cpu->decode.pc;
  int opcode = cpu->decode.opcode;
  if (opcode == 0 || opcode == 1 || opcode == 2 || opcode == 3 || opcode == 4 || opcode == 5 || opcode == 6
      || opcode == 13 || opcode == 14) {
    strcpy(rob_entry.instruction_type, "r2r");
  } else {
    strcpy(rob_entry.instruction_type, "not_r2r");
  }

  queue_insert(cpu, rob_entry);
  increment_rob_tail(cpu);
}

int find_free_register(APEX_CPU *cpu) {
  int free = -1;
  for (int i = 0; i < REG_FILE_SIZE; i++) {
    if (cpu->allocation_list[i] == 0) {
      free = i;
      break;
    }
  }
  return free;
}

void print_issue_queue(APEX_CPU *cpu) {

  for (int i = 0; i < IQ_SIZE; i++) {
    if (cpu->iq_entry_used[i])
      printf("\nInstruction at IQ[%d]: %d", i, cpu->issue_queue[i].pc);
  }
}

void print_reorder_buffer(APEX_CPU *cpu) {

  for (int i = 0; i < cpu->reorder_buffer.tail; i++) {
    printf("\nInstruction at ROB[%d]: %d head: %d tail: %d", i, cpu->reorder_buffer.buffer[i].pc_value,
           cpu->reorder_buffer.head, cpu->reorder_buffer.tail);
  }
}

void print_rename_table(APEX_CPU *cpu) {
  printf("----------\n%s\n----------\n", "Rename Table:");
  for (int i = 0; i < RENAME_TABLE_SIZE; i++) {
    printf("R[%d]->P[%d] ", i, cpu->rename_table[i]);
    if (i == (RENAME_TABLE_SIZE / 2) - 1) {
      printf("\n");
    }
  }

  printf("\n");
  printf("----------\n%s\n----------\n", "Allocation List:");
  for (int i = 0; i < RENAME_TABLE_SIZE; i++) {
    printf("%d [%d] ", i, cpu->allocation_list[i]);
    if (i == (RENAME_TABLE_SIZE / 2) - 1) {
      printf("\n");
    }
  }
  printf("\n");
}

/**
 * Method to forward data from given stage to decode stage
 *
 * @param cpu pointer to current instance of cpu
 */
void forward_data_to_decode(APEX_CPU *cpu, CPU_Stage *stage) {
  if (!(stage->opcode == 0x8 || stage->opcode == 0x9 || stage->opcode == 0xf
      || stage->opcode == 0x10 || stage->opcode == 0x11 || stage->opcode == 0x12 || stage->opcode == 0xa
      || stage->opcode == 0xb || stage->opcode == 0xc)) {
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
        if (stage->rd == cpu->decode.rs1) {
          cpu->decode.rs1_value = stage->result_buffer;
          cpu->forwarded[cpu->decode.rs1] = 1;
        }

        if (stage->rd == cpu->decode.rs2) {
          cpu->decode.rs2_value = stage->result_buffer;
          cpu->forwarded[cpu->decode.rs2] = 1;
        }
        break;
      }

      case OPCODE_ADDL:
      case OPCODE_SUBL:
      case OPCODE_LOAD: {
        if (stage->rd == cpu->decode.rs1) {
          cpu->decode.rs1_value = stage->result_buffer;
          cpu->forwarded[cpu->decode.rs1] = 1;
        }
        break;
      }

      case OPCODE_STORE: {
        if (stage->rd == cpu->decode.rs2) {
          cpu->decode.rs2_value = stage->result_buffer;
          cpu->forwarded[cpu->decode.rs2] = 1;
        }
        break;
      }

      case OPCODE_STR: {
        if (stage->rd == cpu->decode.rs3) {
          cpu->decode.rs3_value = stage->result_buffer;
          cpu->forwarded[cpu->decode.rs3] = 1;
        }

        if (stage->rd == cpu->decode.rs2) {
          cpu->decode.rs2_value = stage->result_buffer;
          cpu->forwarded[stage->rd] = 1;
        }

        if (stage->rd == cpu->decode.rs1) {
          cpu->decode.rs1_value = stage->result_buffer;
          cpu->forwarded[stage->rd] = 1;
        }
        break;
      }
    }
  }
}

/**
 * Method to forward data from given stage to issue queue
 *
 * @param cpu pointer to current instance of cpu
 */
void forward_data_to_iq(APEX_CPU *cpu, CPU_Stage *stage) {

  for (int i = 0; i < IQ_SIZE; i++) {
    IQ_Entry *iq_entry = &cpu->issue_queue[i];
    if (cpu->iq_entry_used[i]) {
      if (!(stage->opcode == 0x8 || stage->opcode == 0x9 || stage->opcode == 0xf
          || stage->opcode == 0x10 || stage->opcode == 0x11 || stage->opcode == 0x12 || stage->opcode == 0xa
          || stage->opcode == 0xb || stage->opcode == 0xc)) {
        switch (iq_entry->opcode) {
          case OPCODE_ADD:
          case OPCODE_SUB:
          case OPCODE_MUL:
          case OPCODE_AND:
          case OPCODE_OR:
          case OPCODE_XOR:
          case OPCODE_LDR:
          case OPCODE_CMP:
          case OPCODE_MOVC: {
            if (stage->rd == iq_entry->rs1) {
              iq_entry->rs1_value = stage->result_buffer;
              cpu->forwarded[stage->rd] = 1;
            }

            if (stage->rd == iq_entry->rs2) {
              iq_entry->rs2_value = stage->result_buffer;
              cpu->forwarded[stage->rd] = 1;
            }
            break;
          }

          case OPCODE_ADDL:
          case OPCODE_SUBL:
          case OPCODE_LOAD: {
            if (stage->rd == iq_entry->rs1) {
              iq_entry->rs1_value = stage->result_buffer;
              cpu->forwarded[stage->rd] = 1;
            }
            break;
          }

          case OPCODE_STORE: {
            if (stage->rd == iq_entry->rs2) {
              iq_entry->rs2_value = stage->result_buffer;
              cpu->forwarded[stage->rd] = 1;
            }
            break;
          }

          case OPCODE_STR: {
            if (stage->rd == iq_entry->rs3) {
              iq_entry->rs3_value = stage->result_buffer;
              cpu->forwarded[stage->rd] = 1;
            }

            if (stage->rd == iq_entry->rs2) {
              iq_entry->rs2_value = stage->result_buffer;
              cpu->forwarded[stage->rd] = 1;
            }

            if (stage->rd == iq_entry->rs1) {
              iq_entry->rs1_value = stage->result_buffer;
              cpu->forwarded[stage->rd] = 1;
            }
            break;
          }
        }
        validate_iq_entry(cpu, iq_entry);
      }
    }
  }
}

void validate_iq_entry(APEX_CPU *cpu, IQ_Entry *iq_entry) {
  switch (iq_entry->opcode) {
    case OPCODE_ADD:
    case OPCODE_SUB:
    case OPCODE_MUL:
    case OPCODE_AND:
    case OPCODE_OR:
    case OPCODE_XOR:
    case OPCODE_LDR:
    case OPCODE_CMP:
    case OPCODE_STORE: {
      if ((cpu->status[iq_entry->rs1] == 1 || cpu->forwarded[iq_entry->rs1] == 1)
          && (cpu->status[iq_entry->rs2] == 1 || cpu->forwarded[iq_entry->rs2] == 1))
        iq_entry->valid = true;
      else
        iq_entry->valid = false;
      break;
    }

    case OPCODE_LOAD:
    case OPCODE_ADDL:
    case OPCODE_SUBL: {
      if (cpu->status[iq_entry->rs1] == 1 || cpu->forwarded[iq_entry->rs1] == 1)
        iq_entry->valid = true;
      else
        iq_entry->valid = false;
      break;
    }

    case OPCODE_STR: {
      if ((cpu->status[iq_entry->rs1] == 1 || cpu->forwarded[iq_entry->rs1] == 1)
          && (cpu->status[iq_entry->rs2] == 1 || cpu->forwarded[iq_entry->rs2] == 1)
          && (cpu->status[iq_entry->rs3] == 1 || cpu->forwarded[iq_entry->rs3] == 1))
        iq_entry->valid = true;
      else
        iq_entry->valid = false;
      break;
    }

    case OPCODE_MOVC: {
      iq_entry->valid = true;
      break;
    }
  }
}

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int get_code_memory_index_from_pc(const int pc) {
  return (pc - 4000) / 4;
}

static void print_instruction(const CPU_Stage *stage) {
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

static void print_instruction_p(const CPU_Stage *stage) {
  switch (stage->opcode) {

    case OPCODE_ADD:
    case OPCODE_SUB:
    case OPCODE_MUL:
    case OPCODE_AND:
    case OPCODE_OR:
    case OPCODE_XOR:
    case OPCODE_LDR: {
      printf("%s,P%d,P%d,P%d ", stage->opcode_str, stage->rd, stage->rs1,
             stage->rs2);
      break;
    }

    case OPCODE_LOAD:
    case OPCODE_SUBL:
    case OPCODE_ADDL: {
      printf("%s,P%d,P%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
             stage->imm);
      break;
    }

    case OPCODE_STORE: {
      printf("%s,P%d,P%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
             stage->imm);
      break;
    }

    case OPCODE_STR: {
      printf("%s,P%d,P%d,P%d ", stage->opcode_str, stage->rs1, stage->rs2,
             stage->rs3);
      break;
    }

    case OPCODE_MOVC: {
      printf("%s,P%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
      break;
    }

    case OPCODE_CMP: {
      printf("%s,P%d,P%d ", stage->opcode_str, stage->rs1, stage->rs2);
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
static void print_stage_content(const char *name, const CPU_Stage *stage) {
  printf("%-15s: pc(%d) ", name, stage->pc);
  if ((strcmp(name, "Fetch") == 0)) {
    print_instruction(stage);
  } else {
    print_instruction_p(stage);
  }
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
