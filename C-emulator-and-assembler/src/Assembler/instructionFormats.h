#ifndef INSTRUCTION_FORMATS_H
#define INSTRUCTION_FORMATS_H

#include <stdint.h>
#include <stdbool.h>

extern int process_branch_instruction(int argc, char** argv) ;
extern int32_t process_sdp(int argc, char **argv);
extern int32_t special_instruction_decoder(int argc, char **argv);
extern int32_t data_processing(int argc, char ** argv);

#define DPI_OP0 0x4
#define DPR_OP0 0x5
#define WIDE_MOVE_OPI 0x5;

typedef struct InstructionGroup {
  char** instructions;
  int instruction_count;
} InstructionGroup;

extern bool is_in_instruction_group(const char* mnemonic, struct InstructionGroup* group);

/*
These structs represent each of the 5 instruction types
The union for instructions allows us to read each of these structs as integers directly
init___() functions initialise the values of predefined bits and MUST be used every time we create a struct, unless done separately (however, this would lead to redundancy)
*/
typedef struct data_proc_imm {
  uint32_t rd : 5;
  uint32_t rn : 5;
  uint32_t imm12 : 12;
  uint32_t sh : 1;
  uint32_t opi : 3;
  uint32_t op0 : 3;
  uint32_t opc : 2;
  uint32_t sf : 1;
} dpi;

typedef struct wide_move {
  uint32_t rd : 5;
  uint32_t imm16 : 16;
  uint32_t hw : 2;
  uint32_t opi : 3;
  uint32_t op0 : 3;
  uint32_t opc : 2;
  uint32_t sf : 1;
} wide_move;

dpi init_dpi(void);

typedef struct data_proc_reg {
  uint32_t rd : 5;
  uint32_t rn : 5;
  uint32_t operand : 6;
  uint32_t rm : 5;
  uint32_t opr : 4;
  uint32_t op0 : 3;
  uint32_t m : 1;
  uint32_t opc : 2;
  uint32_t sf : 1;
} dpr;

dpr init_dpr(void);

typedef struct single_data_trans {
  uint32_t rt : 5;
  uint32_t xn : 5;
  int32_t offset : 12;
  uint32_t l : 1;
  uint32_t predefined1 : 1;
  uint32_t u : 1;
  uint32_t predefined2 : 5;
  uint32_t sf : 1;
  uint32_t msb : 1;
} sdt;

sdt init_sdt(void);

typedef struct load_literal {
  uint32_t rt : 5;
  uint32_t simm19 : 19;
  uint32_t predefined : 6;
  uint32_t sf : 1;
  uint32_t msb : 1;
} ll;

ll init_load_literal(void);

typedef struct branch {
  uint32_t operand : 26;
  uint32_t predefined : 4;
  uint32_t dontcares : 2;
} branch;

branch init_branch(void);

typedef union instrUnion {
  int32_t value;
  dpr dprBits;
  dpi dpiBits;
  wide_move wm;
  sdt sdtBits;
  ll llBits;
  branch branchBits;
} instrUnion;

// Test function to check instructions can returned in the correct integer representation
bool test_instruction_format(void);

#endif // INSTRUCTION_FORMATS_H