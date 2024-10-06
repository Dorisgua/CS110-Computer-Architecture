/* This project is based on the MIPS Assembler of CS61C in UC Berkeley.
   The framework of this project is been modified to be suitable for RISC-V
   in CS110 course in ShanghaiTech University by Zhijie Yang in March 2019.
   Updated by Chibin Zhang and Zhe Ye in March 2021. Modified by Lei Jia 
   to be part of the project in Feb 2023.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*CS61C in UC Berkeley*/
#include "assembler.h"
#include "tables.h"
/*CS61C in UC Berkeley*/
#include "translate.h"
#include "translate_utils.h"
/*CS61C in UC Berkeley*/
#define MAX_ARGS 3
#define BUF_SIZE 1024
const char *IGNORE_CHARS = " \f\n\r\t\v,()";
/*CS61C in UC Berkeley*/
/*******************************
 * Helper Functions
 *******************************/

/* you should not be calling this function yourself. */
static void raise_label_error(uint32_t input_line, const char *label)
{
    write_to_log("Error - invalid label at line %d: %s\n", input_line, label);
}

/* call this function if more than MAX_ARGS arguments are found while parsing
   arguments.

   INPUT_LINE is which line of the input file that the error occurred in. Note
   that the first line is line 1 and that empty lines are included in the count.

   EXTRA_ARG should contain the first extra argument encountered.
 */
static void raise_extra_argument_error(uint32_t input_line,
                                       const char *extra_arg)
{
    write_to_log("Error - extra argument at line %d: %s\n", input_line,
                 extra_arg);
}

/* You should call this function if write_original_code() or translate_inst()
   returns 0.

   INPUT_LINE is which line of the input file that the error occurred in. Note
   that the first line is line 1 and that empty lines are included in the count.
 */
static void raise_instruction_error(uint32_t input_line, const char *name,
                                    char **args, int num_args)
{

    write_to_log("Error - invalid instruction at line %d: ", input_line);
    log_inst(name, args, num_args);
}

/* Truncates the string at the first occurrence of the '#' character. */
static void skip_comments(char *str)
{
    char *comment_start = strchr(str, '#');
    if (comment_start)
    {
        *comment_start = '\0';
    }
}

/* Reads STR and determines whether it is a label (ends in ':'), and if so,
   whether it is a valid label, and then tries to add it to the symbol table.

   INPUT_LINE is which line of the input file we are currently processing. Note
   that the first line is line 1 and that empty lines are included in this
   count.

   BYTE_OFFSET is the offset of the NEXT instruction (should it exist).

   Four scenarios can happen:
    1. STR is not a label (does not end in ':'). Returns 0.
    2. STR ends in ':', but is not a valid label. Returns -1.
    3a. STR ends in ':' and is a valid label. Addition to symbol table fails.
        Returns -1.
    3b. STR ends in ':' and is a valid label. Addition to symbol table succeeds.
        Returns 1.
 */
static int add_if_label(uint32_t input_line, char *str, uint32_t byte_offset,
                        SymbolTable *symtbl, int check_align)
{
    
    size_t len = strlen(str);
    if (str[len - 1] == ':')
    {
        str[len - 1] = '\0';
        if (is_valid_label(str))
        {
            /* add to table and return number of tokens, otherwise return -1 */
            if (add_to_table(symtbl, str, byte_offset, check_align) == 0)
            {
                return 1;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            /* we raise error if no valid label is found */
            raise_label_error(input_line, str);
            return -1;
        }
    }
    else
    {
        return 0;
    }
}

/*******************************
 * Implement the Following
 *******************************/

/* First pass of the assembler. You should implement pass_two() first.

   This function should read each line, strip all comments, scan for labels,
   and pass instructions to write_original_code(), pass data to write_static_data(). 
   The symbol table and data segment should also been built and written to 
   specified file. The input file may or may not be valid. Here are some guidelines:

   1. Only one of ".data" or ".text" may be present per line. It must be the 
      first token present. Once you see a ".data" or ".text", the rest part
      will be treat as data or text segment until the next ".data" or ".text"
      Appears. Ignore the rest token of this line. Default to text segment 
      if not indicated.
    
   2. For data segment, we only considering ".word", ".half", ".Byte" types.
      These types must be the first token of a line. The rest of the tokens 
      in this line will be stored as variables of that type in the data segment.
      You can assume that these numbers are legal with their types.

   3. Only one label may be present per line. It must be the first token
      present. Once you see a label, regardless of whether it is a valid label
      or invalid label, You can treat the NEXT token in this line as the 
      beginning of an instruction or a static data.

   4. If the first token is not a label, treat it as the name of an
      instruction. DO NOT try to check it is a valid instruction in this pass.
   
   //5. Everything after the instruction name in the same line should be treated 
      as arguments to that instruction. If there are more than MAX_ARGS 
      arguments, call raise_extra_argument_error() and pass in the first extra 
      argument. Do not write that instruction to the original file (eg. don't call
      write_original_code())
   
   //6. Only one instruction should be present per line. You do not need to do
       anything extra to detect this - it should be handled by guideline 5.
   
   8. A line containing only a label is valid. The address of the label should
      be the address of the next instruction or static data regardless of 
      whether there is a next instruction or static data or not.
   
   9. If an instruction contains an immediate, you should output it AS IS.
   
   10. Comments should always be skipped before any further process.

   11. Note that the starting address of the text segment is defined as TEXT_BASE
   in "../types.", and the starting address of the data segment is defined as 
   DATA_BASE in "../types.".

   12. Think about how you should use write_original_code() and write_static_data().
   
   Just like in pass_two(), if the function encounters an error it should NOT
   exit, but process the entire file and return -1. If no errors were
   encountered, it should return 0.
 */

int pass_one(FILE *input, FILE *original, FILE *symtbl, FILE *data) {
    /* YOUR CODE HERE */
    char buf[BUF_SIZE];
    Byte buffer[BUF_SIZE]={0};
    /*initialize textoffset dataoffset*/
    uint32_t textoffset=TEXT_BASE;
    uint32_t dataoffset=DATA_BASE;
    /*initialize error*/
    int error=0;
    int indexline=0;
    uint32_t temp=0;
    /*initialize numargs*/
    int dataortext=1;
    int num_args;
    uint32_t index=0;
    /*CS61C in UC Berkeley*/
    char *args[MAX_ARGS];
    char *nextline=NULL;
    uint32_t idx=0;
    /*CS61C in UC Berkeley*/
    char *name=NULL;
    int written=0;
    /*CS61C in UC Berkeley*/
    SymbolTable* symboltbl=create_table(SYMBOLTBL_UNIQUE_NAME);
    while(fgets(buf,BUF_SIZE,input)){
        /*CS61C in UC Berkeley*/
        indexline+=1;
        skip_comments(buf);
        /*all comments*/
        if(strlen(buf)==0){
            continue;
        }
        num_args=0;
        /*CS61C in UC Berkeley*/
        nextline=strtok(buf,IGNORE_CHARS);
        if(nextline==NULL){
            continue;
        }
        /*data*/
        switch(strcmp(nextline,".data")){
            case 0:
                dataortext=0;
                continue;
            default:
                break;
        }
        /*text*/
         switch(strcmp(nextline,".text")){
            case 0:
                dataortext=1;
                continue;
            default:
                break;
        }
        /*data*/
        if(dataortext==0){
            switch(add_if_label(indexline,nextline,dataoffset,symboltbl,0)+1){
                /*case 2*/
                case 2:
                    nextline=strtok(NULL,IGNORE_CHARS);
                    break;
                /*case 1*/
                case 1:
                    error=-1;
                    break;
                /*case 0*/
                case 0:
                    error=-1;
                    continue;
            }
            /*if nextline is null*/
            if(nextline==NULL){
                continue;
            }
            /*.word*/
            if(strcmp(nextline,".word")==0){
                while((nextline=strtok(NULL,IGNORE_CHARS))!=NULL){
                    long tmp;
                    /*translate*/
                    translate_num_32(&tmp,nextline);
                    /*CS61C in UC Berkeley*/
                    memcpy(buffer+dataoffset-DATA_BASE,&tmp,4);
                    dataoffset=dataoffset+1;
                    dataoffset=dataoffset+1;
                    dataoffset=dataoffset+1;
                    dataoffset=dataoffset+1;
                }
            }
            /*.half*/
            else if(strcmp(nextline,".half")==0){
                while((nextline=strtok(NULL,IGNORE_CHARS))!=NULL){
                    long tmp;
                    /*translate*/
                    translate_num_32(&tmp,nextline);
                    /*CS61C in UC Berkeley*/
                    memcpy(buffer+dataoffset-DATA_BASE,&tmp,2);
                    dataoffset=dataoffset+1;
                    dataoffset=dataoffset+1;
                }
            }
            /*.byte*/
            else if(strcmp(nextline,".byte")==0){
                while((nextline=strtok(NULL,IGNORE_CHARS))!=NULL){
                    long tmp;
                    /*translate*/
                    translate_num_32(&tmp,nextline);
                    /*CS61C in UC Berkeley*/
                    memcpy(buffer+dataoffset-DATA_BASE,&tmp,1);
                    dataoffset=dataoffset+1;
                }
            }
            /*else*/
            else{
                error=-1;
                continue;
            } 
        }
        /*text*/
        if(dataortext==1){
            switch(add_if_label(indexline,nextline,textoffset,symboltbl,0)+1){
                /*case 2*/
                case 2:
                    nextline=strtok(NULL,IGNORE_CHARS);
                    break;
                /*case 1*/
                case 1:
                    error=-1;
                    break;
                /*case 0*/
                case 0:
                    error=-1;
                    continue;
            }
            /*if nextline is null*/
            if(nextline==NULL){
                continue;
            }
            name=nextline;
            while((nextline=strtok(NULL,IGNORE_CHARS))!=NULL){
                /*if segmentation fault*/
                if(num_args+1>MAX_ARGS){
                    error=-1;
                    raise_extra_argument_error(indexline,nextline);
                    break;
                }
                /*value*/
                args[num_args]=nextline;
                num_args+=1;
            }
            /*written*/
            written=write_original_code(original,name,args,num_args);
            if(written==0){
                error=-1;
                /*instruction error*/
                raise_instruction_error(indexline,name,args,num_args);
                continue;
            }
            /*textoffset addition*/
            textoffset+=4*written;
        }
    }
    /*write static data from buffer*/
    temp=dataoffset-DATA_BASE+2;
    temp=(temp+1)/4;
    while(index<temp){
        /*bufferrrr*/
        Byte bufferrrr[8];
        idx=index*4;
        memcpy(bufferrrr,buffer+idx,4);
        /*write static data*/
        write_static_data(data,bufferrrr);
        index++;
    }
    /*write table*/
    write_table(symboltbl,symtbl);
    free_table(symboltbl);
    /*return error*/
    if(error<0){
        /*return error*/
        return error;
    }
    else{
        /*return error+1*/
        return error;
    }
}

/* Second pass of the assembler.

   This function should read an original file and the corresponding symbol table
   file, translates it into basic code and machine code. You may assume:
    1. The input file contains no comments
    2. The input file contains no labels
    3. The input file contains at maximum one instruction per line
    4. All instructions have at maximum MAX_ARGS arguments
    5. The symbol table file is well formatted
    6. The symbol table file contains all the symbol required for translation
    7. The line may contain only one "-" which is placeholder you can ignore.
   If an error is reached, DO NOT EXIT the function. Keep translating the rest of
   the document, and at the end, return -1. Return 0 if no errors were encountered. */


int pass_two(FILE *original, FILE *symtbl, FILE *basic, FILE *machine) {
    /* YOUR CODE HERE */
    char bufori[BUF_SIZE];
    int error=0;
    /*initialize line index*/
    int indexline=0;
    uint32_t byteoffset=0;
    /*CS61C in UC Berkeley*/
    char* args[MAX_ARGS];
    char* name;
    /*nextline use for strtok*/
    char *nextline;
    int num_args,written;
    /*create table*/
    SymbolTable* symboltable=create_table_from_file(SYMBOLTBL_UNIQUE_NAME,symtbl);
    /*read file*/
    while(fgets(bufori,BUF_SIZE,original)!=NULL){
        num_args=0;
        indexline+=1;
        /*CS61C in UC Berkeley*/
        name=strtok(bufori,IGNORE_CHARS);
        nextline=strtok(NULL,IGNORE_CHARS);
        /*CS61C in UC Berkeley*/
        while(nextline!=NULL){
            /*if segmentation fault*/
            if(num_args+1>MAX_ARGS){
                error=-1;
                raise_extra_argument_error(indexline,nextline);
            }
            /*CS61C in UC Berkeley*/
            args[num_args]=nextline;
            num_args+=1;
            nextline=strtok(NULL,IGNORE_CHARS);
        }
        /*CS61C in UC Berkeley*/
        written=translate_inst(basic,machine,name,args,num_args,byteoffset,symboltable);
        if(written==0){
            error=-1;
            raise_instruction_error(indexline,name,args,num_args);
        }
        /*CS61C in UC Berkeley*/
        byteoffset+=4*written;
    }
    free_table(symboltable);
    /*return error*/
    if(error<0){
        /*return error*/
        return error;
    }
    else{
        /*return error+1*/
        return error;
    }
}


/*******************************
 * Do Not Modify Code Below
 *******************************/


/* Runs the two-pass assembler. Most of the actual work is done in pass_one()
   and pass_two().
 */
int assemble(int mode, FILE *input, FILE *data, FILE *symtbl, FILE *orgin, FILE *basic, FILE *text)
{
    int err = 0;

    /* Execute pass one if mode two is not specified */
    if (mode != 2)
    {
        rewind(input);
        if (pass_one(input, orgin, symtbl, data) != 0)
        {
            err = 1;
        }
    }

    /* Execute pass two if mode one is not specified */
    if (mode != 1)
    {
        rewind(orgin);
        rewind(symtbl);
        if (pass_two(orgin, symtbl, basic, text) != 0)
        {
            err = 1;
        }
    }

    /* Error handling */
    if (err) {
        write_to_log("One or more errors encountered during assembly operation.\n");
    } else {
        write_to_log("Assembly operation completed successfully!\n");
    }

    return err;
}