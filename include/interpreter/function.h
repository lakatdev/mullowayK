#ifndef INTERPRETER_FUNCTION_H
#define INTERPRETER_FUNCTION_H

#include <interpreter/config.h>
#include <interpreter/variables.h>

typedef struct {
    char name[INTERPRETER_MAX_NAME];
    int param_count;
    Interpreter_Parameter params[INTERPRETER_MAX_PARAMETERS];
    int start_line;
} Interpreter_Function;

#endif
