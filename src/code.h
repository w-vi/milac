/***************************************************
* TM code generation functions for Mila compiler   *
***************************************************/
#ifndef CODE_H_
#define CODE_H_ 1

#include "common.h"

BEGIN_C_DECLS

#define MAX_JUMPS 128
#define REGISTER_COUNT 3
#define REGISTER_SPARE 3
#define NO_REGISTER -1

#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define R5 5

#define GP 6 /* global memory pointer register */
#define PC 7 /* program counter - use for JMP*/

typedef enum
{
    HALT,
    IN,
    OUT,
    ADD,
    INC,
    DEC,
    SHR,
    SHL,
    SUB,
    MUL,
    DIV,
    LD,
    ST,
    LDA,
    LDC,
    JLT,
    JLE,
    JGT,
    JGE,
    JEQ,
    JNE
} inop_t;

typedef enum
{
    RO,
    RM,
    RC /* comment*/
} tmins_type_t;

    
typedef struct tmins_s
{
    tmins_type_t type;
    uint32_t line;
    inop_t op;
    int r1;
    int r2;
    int r3;
    const char *comment;
}   tmins_t;

extern list(tmins_t *, tm_code);

/* Procedure emitComment prints a comment line 
 * with comment c in the code file
 */
void emit_comment(char *c);

/* Procedure emitRO emits a register-only
 * TM instruction
 * op = the opcode
 * r = target register
 * s = 1st source register
 * t = 2nd source register
 * c = a comment to be printed if TraceCode is TRUE
 */
void emit_RO(inop_t op, int r, int s, int t, char *c);

/* Procedure emitRM emits a register-to-memory
 * TM instruction
 * op = the opcode
 * r = target register
 * d = the offset
 * s = the base register
 * c = a comment to be printed if TraceCode is TRUE
 */
void emit_RM(inop_t op, int r, int d, int s, char *c);

/* Function emitSkip skips "howMany" code
 * locations for later backpatch. It also
 * returns the current code position
 */
int emit_skip(int how_many);

/* Procedure emitBackup backs up to 
 * loc = a previously skipped location
 */
void emit_backup(int loc);

/* Procedure emitRestore restores the current 
 * code position to the highest previously
 * unemitted position
 */
void emit_restore(void);

/* Procedure emitRM_Abs converts an absolute reference 
 * to a pc-relative reference when emitting a
 * register-to-memory TM instruction
 * op = the opcode
 * r = target register
 * a = the absolute location in memory
 * c = a comment to be printed if TraceCode is TRUE
 */
void emit_RM_abs(inop_t op, int r, int a, char *c);

END_C_DECLS

#endif /* CODE_H_ */
