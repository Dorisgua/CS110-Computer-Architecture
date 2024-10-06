/* This project is based on the MIPS Assembler of CS61C in UC Berkeley.
   The framework of this project is been modified to be suitable for RISC-V
   in CS110 course in ShanghaiTech University by Zhijie Yang in March 2019.
   Updated by Chibin Zhang and Zhe Ye in March 2021. Modified by Lei Jia 
   to be part of the project in Feb 2023.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*include*/
#include "tables.h"
#include "translate.h"
#include "translate_utils.h"

/* Writes data during the assembler's first pass to OUTPUT. 
   The input Byte* buf should point to a buffer with a size larger than 8 Bytes.
   Then this function takes the first 4 bytes as a word and write to output FILE.
   And move the second 4 bytes to the head and set second 4 bytes to 0.
   Think about how to use this function in the output data.

   Notes: You can leave this function out, but you need to make sure that 
   the form of the output data file remains the same.
 */

int write_static_data(FILE *output, Byte *buf) {
    /*write static data*/
    fprintf(output, "%08x\n", *((Word*)buf));
    memcpy(buf, buf+4, 4);
    memset(buf+4, 0, 4);
    return 0;
}

/* Writes original code during the assembler's first pass to OUTPUT. 
   
   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS.

   Returns the number of instructions written (so 0 if there were any errors).

   Notes: Think about what you should do with this return value.
 */

unsigned write_original_code(FILE *output, const char *name, char **args,
                             int num_args) {
    /*write original code*/
    long imm;
    unsigned num_basic_code=0;

    if (!output || !name || !args)
        return 0;

    /* if-else ladder for original code output */
    if (strcmp(name, "la") == 0)
    {
        /* check count of arguments */
        if (num_args != 2) 
            return 0;
        
        if (num_args == 2)
            num_basic_code = 2;
    }
    else if (strcmp(name, "lw") == 0)
    {
        /* lw can takes two or three parameters */
        if (num_args != 2 && num_args != 3) 
            return 0;
        
        /* lw x11, label */
        if (num_args == 2)
            num_basic_code = 2;

        /* lw x11, 0(x12) */
        if (num_args == 3)
            num_basic_code = 1;
    }
    else if (strcmp(name, "li") == 0)
    {
        /* check count of arguments */
        if (num_args != 2)
            return 0;
        /* translate and check input immediate */
        if (translate_num_32(&imm, args[1]) == -1)
            return 0;

        /* check how many lines are used */
        if (_INT12_MIN_ <= imm && imm <= _INT12_MAX_)
            num_basic_code = 1;
        else
            num_basic_code = 2;
    }
    /*else situation*/
    else
    {
        num_basic_code = 1;
    }

    /* perform write string to output here */
    write_inst_string(output, name, args, num_args);
    if(num_basic_code>1)
        fprintf(output, "-\n");

    return num_basic_code;
}

/* Translate instructions perform a pseudoinstruction extpansion and call 
   write_instruction() to write the corresponding instruction. The case for
   general instructions, beqz and la has already been completed, but you need to 
   write code to translate the bnez, j, jr, li, lw and mv pseudoinstructions. 
   Your pseudoinstruction expansions should not have any side effects.

   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS.

   Error checking for regular instructions are done in write_instruction(). 
   However, for pseudoinstructions, you must make sure that ARGS contains the 
   correct number of arguments. You do NOT need to check whether the registers 
   or label are valid, since that will be checked in write_instruction().

   Also for li:
    - make sure that the number is representable by 32 bits. (Hint: the number
        can be both signed or unsigned).
    - if the immediate can fit in the imm field of an addiu instruction, then
        expand li into a single addi instruction. Otherwise, expand it into
        a lui-addi pair.

   And for other pseudoinstruction:
    - your expansion should be as same as venus. Try them in venus and see what
        will happen. 

   venus has slightly different translation rules for li, and it allows numbers
   larger than the largest 32 bit number to be loaded with li. You should follow
   the above rules if venus behaves differently. 

   If writing multiple instructions, make sure that each instruction has correct 
   addrress.

   Returns the number of instructions written (so 0 if there were any errors).
 */


int translate_inst(FILE *basic_code, FILE *machine_code, const char *name, char **args, 
                   size_t num_args, uint32_t addr, SymbolTable *symtbl)
{
    /* initialize */
    char *sub_args[3];
    char buf[100];
    long imm, head, uiSec, addSec;
    /* early error handling on invalid arguments */
    if (!basic_code || !machine_code || !name || !args)
        return 0;
    /* beqz */
    if (strcmp(name, "beqz") == 0) {
        /* check count of arguments */
        if (num_args != 2)
            return 0;
        /* Setup parameters */
        sub_args[0] = args[0];
        sub_args[1] = "x0";
        sub_args[2] = args[1];
        /*write instruction*/
        if(write_instruction(basic_code, machine_code, "beq", sub_args, 3, addr, symtbl)==-1)
            return 0;
        /* error if not successful */
        return 1;
    }
    /* bnez */
    else if (strcmp(name, "bnez") == 0) {
        /* YOUR CODE HERE */
        /* check count of arguments */
        if (num_args != 2)
            return 0;
        /* Setup parameters */
        sub_args[0] = args[0];
        sub_args[1] = "x0";
        sub_args[2] = args[1];
        /*write instruction*/
        if(write_instruction(basic_code, machine_code, "bne", sub_args, 3, addr, symtbl)==-1)
            return 0;
        /* error if not successful */
        return 1;
    }
    /* j */
    else if (strcmp(name, "j") == 0) {
        /* YOUR CODE HERE */
        /*check count of arguments*/
        if (num_args != 1)
            return 0;
        /* Setup parameters */
        sub_args[0] = "x0";
        sub_args[1] = args[0];
        /* write instruction */
        if(write_instruction(basic_code, machine_code, "jal", sub_args, 2, addr, symtbl)==-1)
            return 0;
        /* error if not successful */
        return 1;
    }
    /* jr */
    else if (strcmp(name, "jr") == 0)
    {
        /* YOUR CODE HERE */
        /*check count of arguments*/
        if (num_args != 1)
            return 0;
        /* Setup parameters */
        sub_args[0] = "x0";
        sub_args[1] = args[0];
        sub_args[2] = "0x0";
        /* write instruction */
        if(write_instruction(basic_code, machine_code, "jalr", sub_args, 3, addr, symtbl)==-1)
            return 0;
        /* error if not successful */
        return 1;
    }   /* jal and jalr can be explicitly specified or use default ra */    
    /* jal */
    else if (strcmp(name, "jal") == 0)
    {
        /* YOUR CODE HERE */
        /*check whether need to add elements*/
        if (num_args == 2) {
            /* write instruction */
            if (write_instruction(basic_code, machine_code, "jal", args, 2, addr, symtbl)==-1)
                return 0;
            return 1;
        }
        /*check count of arguments*/
        if (num_args != 1)
            return 0;
        sub_args[0] = "x1";
        sub_args[1] = args[0];
        if(write_instruction(basic_code, machine_code, "jal", sub_args, 2, addr, symtbl)==-1)
            return 0;
        /* error if not successful */
        return 1;

    }
    else if (strcmp(name, "jalr") == 0)
    {
        /* YOUR CODE HERE */
        /*check whether need to add elements*/
        if (num_args == 3) {
            /* write instruction */
            if (write_instruction(basic_code, machine_code, "jalr", args, 3, addr, symtbl)==-1)
                return 0;
            return 1;
        }
        /*check count of arguments*/
        if (num_args != 1)
            return 0;
        /* Setup parameters */
        sub_args[0] = "x1";
        sub_args[1] = args[0];
        sub_args[2] = "0x0";
        /* write instruction */
        if(write_instruction(basic_code, machine_code, "jalr", sub_args, 3, addr, symtbl)==-1)
            return 0;
        /* error if not successful */
        return 1;
    }
    else if (strcmp(name, "la") == 0) {
         /* check count of arguments */
        if (num_args != 2)
            return 0;
        /* Get the symbol address */
        imm = get_addr_for_symbol(symtbl, args[1]);
        if(imm==-1)
            return 0;
        /* Get the symbol offset from current PC */
        imm = (imm - addr);
        /* Setup parameters for auipc */
        head = (imm+0x800)>>12;
        sprintf(buf, "%d", STATIC_CAST(int,head));
        sub_args[0] = args[0];
        sub_args[1] = buf;
        /* auipc */
        if(write_instruction(basic_code, machine_code, "auipc", sub_args, 2, addr, symtbl)==-1)
            return 0;
        /* Setup parameters for addi */
        imm = imm - (head<<12);
        sprintf(buf, "%d", STATIC_CAST(int,imm));
        sub_args[0] = args[0];
        sub_args[1] = args[0];
        sub_args[2] = buf;
        /* addi */
        if(write_instruction(basic_code, machine_code, "addi", sub_args, 3, addr+4, symtbl)==-1)
            return 0;
        /* error if not successful */
        return 2;
    }
    else if (strcmp(name, "lw") == 0) {
        /* YOUR CODE HERE */
        /* check count of arguments */
        if (num_args == 3) {
            if (write_instruction(basic_code, machine_code, "lw", args, 3, addr, symtbl)==-1)
                return 0;
            return 1;
        }
        if (num_args != 2)
            return 0;
        /* Get the symbol address */
        imm = get_addr_for_symbol(symtbl, args[1]);
        if(imm==-1)
            return 0;
        /* Get the symbol offset from current PC */
        imm = (imm - addr);
        /* Setup parameters for auipc */
        head = (imm+0x800)>>12;
        sprintf(buf, "%d", STATIC_CAST(int,head));
        sub_args[0] = args[0];
        sub_args[1] = buf;
        /* auipc */
        if(write_instruction(basic_code, machine_code, "auipc", sub_args, 2, addr, symtbl)==-1)
            return 0;
        /* Setup parameters for addi */
        imm = imm - (head<<12);
        sprintf(buf, "%d", STATIC_CAST(int,imm));
        /* subargs */
        sub_args[0] = args[0];
        sub_args[1] = buf;
        sub_args[2] = args[0];
        /* lw */
        if(write_instruction(basic_code, machine_code, "lw", sub_args, 3, addr+4, symtbl)==-1)
            return 0;
        /* error if not successful */
        return 2;
    }
    else if (strcmp(name, "li") == 0)
    {
        /* YOUR CODE HERE */
        /* check immediate */
        if(translate_num_32(&imm, args[1]) == -1)
            return 0;
        /* check count of arguments */
        if(num_args != 2)
            return 0;
        /*small one*/
        if(_INT12_MIN_ <= imm && imm <= _INT12_MAX_) {
            sub_args[0] = args[0];
            sub_args[1] = "x0";
            sub_args[2] = args[1];
            /* addi */
            if(write_instruction(basic_code, machine_code, "addi", sub_args, 3, addr, symtbl)==-1)
                return 0;
            return 1;
        }
        else {
            /* the last 20 bits of immediate */
            uiSec = ((imm >> 12) & 0xFFFFF);
            /* the first 12 bits of immediate */
            addSec = (imm & 0xFFF);
            /* extension */
            if((addSec & (1UL << 11)) != 0)
                uiSec+=1, sign_extension(&addSec, addSec, 12);
            /* write as decimal format */
            sprintf(buf, "%u", (unsigned int)uiSec);
            sub_args[0] = args[0];
            sub_args[1] = buf;
            /* lui */
            if(write_instruction(basic_code, machine_code, "lui", sub_args, 2, addr, symtbl)==-1)
                return 0;
            /* write as decimal format */
            sprintf(buf, "%d", (int)addSec);
            sub_args[0] = args[0];
            sub_args[1] = args[0];
            sub_args[2] = buf;
            /* addi */
            if(write_instruction(basic_code, machine_code, "addi", sub_args, 3, addr, symtbl)==-1)
                return 0;
            return 2;
        }
    }
    else if (strcmp(name, "mv") == 0){
        /* YOUR CODE HERE */
        /* check count of arguments */
        if (num_args != 2)
            return 0;
        /* Setup parameters */
        sub_args[0] = args[0];
        sub_args[1] = args[1];
        sub_args[2] = "0x0";
        /* addi */
        if(write_instruction(basic_code, machine_code, "addi", sub_args, 3, addr, symtbl)==-1)
            return 0;
        /* error if not successful */
        return 1;
    }
    else {
        /* handle non-pseudo ones */
        if(write_instruction(basic_code, machine_code, name, args, num_args, addr, symtbl)==-1)
            return 0;
        /* error if not successful */
        return 1;
    }
    return 0; 
}

/* Writes the instruction in basic and hexadecimal format to OUTPUT during 
   pass #2.

   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS.

   The address (addr) is given for current instruction PC address.

   The symbol table (SYMTBL) is given for any symbols that need to be resolved
   at this step.

   You must perform error checking on all instructions and make sure that their
   arguments are valid. If an instruction is invalid, you should not write
   anything to OUTPUT but simply return -1. venus may be a useful resource for
   this step.

   Note the use of helper functions. Consider writing your own! If the function
   definition comes afterwards, you must declare it first (see translate.h).

   Returns 0 on success and -1 on error.
 */


int write_instruction(FILE *basic_code, FILE *machine_code, const char *name, char **args, 
                      size_t num_args, uint32_t addr, SymbolTable *symtbl)
{   
    /** R-format */
    /* add */
    addr = addr;
    symtbl = symtbl;
    if (strcmp(name, "add") == 0)
        return write_rtype(basic_code, machine_code, name, 0x33, 0x00, 0x00, args, num_args);
    /* mul */
    else if (strcmp(name, "mul") == 0)
        return write_rtype(basic_code, machine_code, name, 0x33, 0x00, 0x01, args, num_args);
    /* sub */
    else if (strcmp(name, "sub") == 0)
        return write_rtype(basic_code, machine_code, name, 0x33, 0x00, 0x20, args, num_args);
    /* sll */
    else if (strcmp(name, "sll") == 0)
        return write_rtype(basic_code, machine_code, name, 0x33, 0x01, 0x00, args, num_args); 
    /* mulh */
    else if (strcmp(name, "mulh") == 0)
        return write_rtype(basic_code, machine_code, name, 0x33, 0x01, 0x01, args, num_args);
    /* slt */
    else if (strcmp(name, "slt") == 0)
        return write_rtype(basic_code, machine_code, name, 0x33, 0x02, 0x00, args, num_args);
    /* sltu */
    else if (strcmp(name, "sltu") == 0)
        return write_rtype(basic_code, machine_code, name, 0x33, 0x03, 0x00, args, num_args);
    /* xor */
    else if (strcmp(name, "xor") == 0)
        return write_rtype(basic_code, machine_code, name, 0x33, 0x04, 0x00, args, num_args);
    /* div */
    else if (strcmp(name, "div") == 0)
        return write_rtype(basic_code, machine_code, name, 0x33, 0x04, 0x01, args, num_args);
    /* srl */
    else if (strcmp(name, "srl") == 0)
        return write_rtype(basic_code, machine_code, name, 0x33, 0x05, 0x00, args, num_args);
    /* sra */
    else if (strcmp(name, "sra") == 0)
        return write_rtype(basic_code, machine_code, name, 0x33, 0x05, 0x20, args, num_args);
    /* or */
    else if (strcmp(name, "or") == 0)
        return write_rtype(basic_code, machine_code, name, 0x33, 0x06, 0x00, args, num_args);
    /* rem */
    else if (strcmp(name, "rem") == 0)
        return write_rtype(basic_code, machine_code, name, 0x33, 0x06, 0x01, args, num_args);
    /* and */
    else if (strcmp(name, "and") == 0)
        return write_rtype(basic_code, machine_code, name, 0x33, 0x07, 0x00, args, num_args);
    /* lb */
    else if (strcmp(name, "lb") == 0)
        return write_itype(basic_code, machine_code, name, 0x03, 0x00, args, num_args, symtbl);
    /* lh */
    else if (strcmp(name, "lh") == 0)
        return write_itype(basic_code, machine_code, name, 0x03, 0x01, args, num_args, symtbl);
    /* lw */
    else if (strcmp(name, "lw") == 0)
        return write_itype(basic_code, machine_code, name, 0x03, 0x02, args, num_args, symtbl);
    /* lbu */
    else if (strcmp(name, "lbu") == 0)
        return write_itype(basic_code, machine_code, name, 0x03, 0x04, args, num_args, symtbl);
    /* lhu */
    else if (strcmp(name, "lhu") == 0)
        return write_itype(basic_code, machine_code, name, 0x03, 0x05, args, num_args, symtbl);
    /* addi */
    else if (strcmp(name, "addi") == 0)
        return write_itype(basic_code, machine_code, name, 0x13, 0x00, args, num_args, symtbl);
    /* slli */
    else if (strcmp(name, "slli") == 0)
        return write_itype(basic_code, machine_code, name, 0x13, 0x01, args, num_args, symtbl);
    /* slti */
    else if (strcmp(name, "slti") == 0)
        return write_itype(basic_code, machine_code, name, 0x13, 0x02, args, num_args, symtbl);
    /* sltiu */
    else if (strcmp(name, "sltiu") == 0)
        return write_itype(basic_code, machine_code, name, 0x13, 0x03, args, num_args, symtbl);
    /* xori */
    else if (strcmp(name, "xori") == 0)
        return write_itype(basic_code, machine_code, name, 0x13, 0x04, args, num_args, symtbl);
    /* srli */
    else if (strcmp(name, "srli") == 0)
        return write_itype(basic_code, machine_code, name, 0x13, 0x05, args, num_args, symtbl);
    /* srai */
    else if (strcmp(name, "srai") == 0)
        return write_itype(basic_code, machine_code, name, 0x13, 0x05, args, num_args, symtbl);
    /* ori */
    else if (strcmp(name, "ori") == 0)
        return write_itype(basic_code, machine_code, name, 0x13, 0x06, args, num_args, symtbl);
    /* andi */
    else if (strcmp(name, "andi") == 0)
        return write_itype(basic_code, machine_code, name, 0x13, 0x07, args, num_args, symtbl);
    /* jalr */
    else if (strcmp(name, "jalr") == 0)
        return write_itype(basic_code, machine_code, name, 0x67, 0x00, args, num_args, symtbl);
    /* ecall */
    else if (strcmp(name, "ecall") == 0)
        return write_itype(basic_code, machine_code, name, 0x73, 0x00, args, num_args, symtbl);
    /* sb */
    else if (strcmp(name, "sb") == 0)
        return write_stype(basic_code, machine_code, name, 0x23, 0x00, args, num_args, symtbl);
    /* sh */
    else if (strcmp(name, "sh") == 0)
        return write_stype(basic_code, machine_code, name, 0x23, 0x01, args, num_args, symtbl);
    /* sw */
    else if (strcmp(name, "sw") == 0)
        return write_stype(basic_code, machine_code, name, 0x23, 0x02, args, num_args, symtbl);
    /* beq */
    else if (strcmp(name, "beq") == 0)
        return write_sbtype(basic_code, machine_code, name, 0x63, 0x00, args, num_args, addr, symtbl);
    /* bne */
    else if (strcmp(name, "bne") == 0)
        return write_sbtype(basic_code, machine_code, name, 0x63, 0x01, args, num_args, addr, symtbl);
    /* blt */
    else if (strcmp(name, "blt") == 0)
        return write_sbtype(basic_code, machine_code, name, 0x63, 0x04, args, num_args, addr, symtbl);
    /* bge */
    else if (strcmp(name, "bge") == 0)
        return write_sbtype(basic_code, machine_code, name, 0x63, 0x05, args, num_args, addr, symtbl);
    /* bltu */
    else if (strcmp(name, "bltu") == 0)
        return write_sbtype(basic_code, machine_code, name, 0x63, 0x06, args, num_args, addr, symtbl);
    /* bgeu */
    else if (strcmp(name, "bgeu") == 0)
        return write_sbtype(basic_code, machine_code, name, 0x63, 0x07, args, num_args, addr, symtbl);
    /* auipc */
    else if (strcmp(name, "auipc") == 0)
        return write_utype(basic_code, machine_code, name, 0x17, args, num_args, symtbl);
    /* lui */
    else if (strcmp(name, "lui") == 0)
        return write_utype(basic_code, machine_code, name, 0x37, args, num_args, symtbl);
    /* jal */
    else if (strcmp(name, "jal") == 0)
        return write_ujtype(basic_code, machine_code, name, 0x6f, args, num_args, addr, symtbl);
    /* YOUR CODE HERE */
    /* else condition */
    else
        return -1;
}


/* A helper function for writing most R-type instructions. You should use
   translate_reg() to parse registers and write_inst_rtype(), write_inst_stype(),
   write_inst_sbtype(), write_inst_utype(), write_inst_ecall(), write_inst_hex() 
   to write to basic_code or machine_code. Both are defined in translate_utils.h.

   Function write_rtype() is complete for reference. Complete other functions
   implementation below. You will find bitwise operations to be the cleanest 
   way to complete this function.

   Returns 0 on success and -1 on failure
 */
int write_rtype(FILE *basic_code, FILE *machine_code, const char *name, uint8_t opcode, 
                uint8_t funct3, uint8_t funct7, char **args, size_t num_args)
{
    /*  Perhaps perform some error checking? */
    int rd=0, rs=0, rt=0;
    uint32_t instruction;
    /* R-format requires rd rs rt */
    if (num_args != 3) {
        return -1;
    }
    /* destination register */
    rd = translate_reg(args[0]);
    /* source register 1 */
    rs = translate_reg(args[1]);
    /* source register 2 */
    rt = translate_reg(args[2]);
    /* error checking for register ids */
    if (rd == -1 || rs == -1 || rt == -1)
    {
        return -1;
    }
    /* write basic code */
    write_inst_rtype(basic_code, name, rd, rs, rt);
    /* generate instruction */
    instruction = opcode + (rd << 7) + (funct3 << 12) + (rs << 15) +
                  (rt << 20) + (funct7 << 25);
    /* write machine code */
    write_inst_hex(machine_code, instruction);
    return 0;
}

int write_itype(FILE *basic_code, FILE *machine_code, const char *name, uint8_t opcode, 
                uint8_t funct3, char **args, size_t num_args, SymbolTable *symbol)
{
    /* YOUR CODE HERE */
    int rd=0, rs=0;
    uint32_t instruction;
    long int imm=0;
    int jud;
    symbol = symbol;
    /* i-format requires rd rs imm*/
    /* check count of argument */
    if (opcode != 0x73&&num_args != 3)
        return -1;
    if (opcode == 0x73&&num_args != 0)
        return -1;
    /* rd imm rs */
    if (opcode == 0x03) {
        /* destination register */
        rd = translate_reg(args[0]);
        /* imm */
        jud = translate_num_12(&imm, args[1]);
        if(jud == -1)
            return -1;
        /* destination register */
        rs = translate_reg(args[2]);
    }
    /* rd rs imm */
    else if (opcode == 0x13|| opcode == 0x67) {
        /* destination register */
        rd = translate_reg(args[0]);
        /* source register 1 */
        rs = translate_reg(args[1]);
        /* imm */
        jud = translate_num_12(&imm, args[2]);
        if(jud == -1)
            return -1;
    }
    /* error checking for register ids and imm */
    if(opcode != 0x73)
    if (rd == -1 || rs == -1) {
        return -1;
    }
    /* check range */
    if ((strcmp(name, "srai") == 0|| strcmp(name, "srli") == 0 || strcmp(name, "slli") == 0)&&(imm > 31 || imm < 0))
        return -1;
    /* write basic code */
    if (opcode == 0x03)
        write_inst_stype(basic_code, name, rd, rs, imm);
    /* different situation */
    else if(opcode == 0x73)
        write_inst_ecall(basic_code, name);
    else if(opcode != 0x73)
        write_inst_sbtype(basic_code, name, rd, rs, imm);
    /* generate instruction */
    instruction = opcode + (rd << 7) + (funct3 << 12) + (rs << 15) +(imm << 20);
    /* special situation of "srai" */
    if (strcmp(name, "srai")==0)
        instruction = instruction+(0x4 << 28);
    /* write machine code */
    write_inst_hex(machine_code, instruction);
    return 0;
}

int write_stype(FILE *basic_code, FILE *machine_code, const char *name, uint8_t opcode, 
                uint8_t funct3, char **args, size_t num_args, SymbolTable *symbol)
{
    /* YOUR CODE HERE */
    /* initialize */
    int rs=0, rt=0, jud;
    uint32_t instruction;
    long int imm=0;
    symbol = symbol;
    /* R-format requires rs rt imm */
    /* check count of argument */
    if (num_args != 3) {
        return -1;
    }
    /* destination register */
    rt = translate_reg(args[0]);
    /* source register 1 */
    jud = translate_num_12(&imm, args[1]);
    /* check imm */
    if(jud == -1)
        return -1;
    rs = translate_reg(args[2]);
    /* error checking for register ids */
    if (rs == -1 || rt == -1) {
        return -1;
    }
    /* write basic code */
    write_inst_stype(basic_code, name, rt, rs, imm);
    /* generate instruction */
    instruction = opcode + ((imm&0x1f) << 7) + (funct3 << 12) + (rs << 15) + (rt << 20) + (((imm&0xfe0)>>5) << 25);
    /* write machine code */
    write_inst_hex(machine_code, instruction);
    return 0;
}

/* Hint: the way for branch to calculate relative address. 
   e.g. bne bne rs rt label
   assume the byte_offset(addr) of label is L,
   current instruction byte_offset(addr) is A
   the relative address I for label satisfy:
     L = A + I
   so the relative addres is
     I = L - A              */
int write_sbtype(FILE *basic_code, FILE *machine_code, const char *name, uint8_t opcode, 
                 uint8_t funct3, char **args, size_t num_args, uint32_t addr, SymbolTable *symbol)
{
    /* YOUR CODE HERE */
    int rs=0, rt=0;
    uint32_t instruction;
    /* initialize */
    long int imm=0;
    addr = addr;
    /* R-format requires imm rs rt */
    /* check count of arguments */
    if (num_args != 3) {
        return -1;
    }
    /* destination register */
    rs = translate_reg(args[0]);
    /* source register 1 */
    rt = translate_reg(args[1]);
    /* error checking for register ids */
    if (rs == -1 || rt == -1 || !args[2] || is_valid_label(args[2])==0) {
        return -1;
    }
    /* get imm */
    imm = get_addr_for_symbol(symbol, args[2]);
    /* check imm */
    if(imm == -1)
        return -1;
    imm = imm - addr;
    if (imm < _INT12_MIN_ || imm > _INT12_MAX_)
        return -1;
    /* write basic code */
    write_inst_sbtype(basic_code, name, rs, rt, imm);
    /* generate instruction */
    instruction = opcode + (((imm&0x800)>>11) << 7) + (((imm&0x1e)>>1) << 8) + (funct3 << 12) + (rs << 15) + (rt << 20) + (((imm&0x7e0)>>5) << 25) + (((imm&0x1000)>>12) << 31);
    /* write machine code */
    write_inst_hex(machine_code, instruction);
    return 0;
}

int write_utype(FILE *basic_code, FILE *machine_code, const char *name, uint8_t opcode,  
                char **args, size_t num_args, SymbolTable *symbol)
{
    /* YOUR CODE HERE */
    int rd=0, jud;
    uint32_t instruction;
    long int imm=0;
    symbol = symbol;
    /* R-format requires rd rs rt */
    /* check count of arguments */
    if (num_args != 2) {
        return -1;
    }
    /* destination register */
    rd = translate_reg(args[0]);
    jud = translate_num(&imm, args[1], 0, _UINT20_MAX_);
    /* check imm 1 */
    if(jud == -1)
        return -1;
    /* error checking for register ids */
    if (rd == -1) {
        return -1;
    }
    /* write basic code */
    write_inst_utype(basic_code, name, rd, imm);
    /* generate instruction */
    instruction = opcode + (rd << 7) + (imm << 12);
    /* write machine code */
    write_inst_hex(machine_code, instruction);
    return 0;
}

/* In this project there is no need to relocate labels,
   you may think about the reasons. */
int write_ujtype(FILE *basic_code, FILE *machine_code, const char *name, uint8_t opcode,
                 char **args, size_t num_args, uint32_t addr, SymbolTable *symbol)
{
    /* YOUR CODE HERE */
    /* initialize */
    int rd=0;
    uint32_t instruction;
    long int imm=0;
    /* R-format requires rd rs rt */
    /* check count of argument */
    if (num_args != 2) {
        return -1;
    }
    /* destination register */
    rd = translate_reg(args[0]);
    /* source register 1 */
    /* error checking for register ids */
    if (rd == -1 || !args[1] || is_valid_label(args[1])==0) {
        return -1;
    }
    /* get imm */
    imm = get_addr_for_symbol(symbol, args[1]);
    /* check imm */
    if(imm == -1)
        return -1;
    imm = imm - addr;
    if (imm < _INT12_MIN_ || imm > _INT12_MAX_)
        return -1;
    /* write basic code */
    write_inst_utype(basic_code, name, rd, imm);
    /* generate instruction */
    instruction = opcode + (rd << 7) + (((imm&0xff000)>>12) << 12) + (((imm&0x800)>>11) << 20) + (((imm&0x7fe)>>1) << 21) + (((imm&0x100000)>>20) << 31);
    /* write machine code */
    write_inst_hex(machine_code, instruction);
    return 0;
}
