#include <stdint.h>

#define MAX_STR_LEN 100
#define REG_OFFSET_PREDEFINED 0x81A

extern int32_t create_load_literal(int32_t signFlag, int32_t rt, int32_t literal);
extern int32_t create_register_offset(int32_t loadOrStore, int32_t signFlag, int32_t rt, int32_t xm, int32_t xn);
extern int32_t create_pre_post_index(int32_t loadOrStore, int32_t signFlag, int32_t rt, int32_t simm9, int32_t preOrPost, int32_t xn);
extern int32_t create_unsigned_offset(int32_t loadOrStore, int32_t signFlag, int32_t rt, int32_t imm12, int32_t xn);