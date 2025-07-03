#ifndef INTERPRETER_INSTRUCTIONS_H
#define INTERPRETER_INSTRUCTIONS_H

#include <interpreter/instance.h>

typedef struct {
    const char* token;
    int token_index;
} Interpreter_Instruction_AttributeMatcher;

typedef void (*Interpreter_Instruction_ExecuteFunction)(Interpreter_Instance* instance, char** tokens, int token_count);

typedef struct {
    const char* name;
    Interpreter_Instruction_AttributeMatcher* matchers;
    int matcher_count;
    int min_token_count;
    Interpreter_Instruction_ExecuteFunction execute;
} Interpreter_Instruction_Definition;

extern const Interpreter_Instruction_Definition g_instruction_definitions[];
extern const int g_instruction_definition_count;

int instruction_matches(const Interpreter_Instruction_Definition* def, char** tokens, int token_count);

void interpreter_execute_add(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_add_assign(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_array_get(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_array_set(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_assign(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_call(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_call_assign(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_cat(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_clear(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_declare(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_divide(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_divide_assign(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_end(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_exec(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_free(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_if(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_input(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_load(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_mod(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_mod_assign(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_multiply(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_multiply_assign(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_print(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_println(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_random(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_return(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_save(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_sizeof(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_sleep(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_subtract(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_subtract_assign(Interpreter_Instance* instance, char** tokens, int token_count);
void interpreter_execute_while(Interpreter_Instance* instance, char** tokens, int token_count);

#endif
