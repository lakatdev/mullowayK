#ifndef INTERPRETER_INSTANCE_H
#define INTERPRETER_INSTANCE_H

#include <interpreter/types.h>
#include <interpreter/config.h>
#include <interpreter/function.h>

typedef enum {
    INPUT_MODE_NONE,
    INPUT_MODE_STRING,
    INPUT_MODE_NUMERIC,
    INPUT_MODE_ASCII,
    INPUT_MODE_SERIAL
} Interpreter_InputMode;

typedef struct {
    int return_address;
    int local_var_count;
    Interpreter_Variable local_vars[INTERPRETER_MAX_VARIABLES];
    int ref_count;
    Interpreter_Reference refs[INTERPRETER_MAX_PARAMETERS];
    const char* return_var;
} Interpreter_CallFrame;

typedef struct {
    Interpreter_Function functions[INTERPRETER_MAX_FUNCTIONS];
    int func_count;
    int depth;
    char parsed_code[INTERPRETER_MAX_PARSED_LINES][INTERPRETER_MAX_TOKENS_PER_LINE][INTERPRETER_MAX_TOKEN_LENGTH];
    int line_token_counts[INTERPRETER_MAX_PARSED_LINES];
    int parsed_line_count;
    Interpreter_CallFrame call_stack[INTERPRETER_MAX_STACK];
    int stack_pointer;
    int execution_position;
    int is_running;
    int should_stop;
    int instruction_count;
    int is_sleeping;
    unsigned long long int sleep_until_tick;
    int waiting_for_input;
    char* input_variable_name;
    char input_buffer[INTERPRETER_MAX_INPUT_LENGTH];
    int input_ready;
    Interpreter_InputMode input_mode;
    int serial_bytes_to_read;
    int serial_bytes_read;
} Interpreter_Instance;

#endif
