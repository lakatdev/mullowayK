#ifndef INTERPRETER_VARIABLES_H
#define INTERPRETER_VARIABLES_H

#include <interpreter/types.h>
#include <interpreter/config.h>

typedef struct {
    char name[INTERPRETER_MAX_NAME];
    Interpreter_VarType type;
    Interpreter_Value value;
    int declaration_line;
} Interpreter_Variable;

typedef struct {
    char name[INTERPRETER_MAX_NAME];
    Interpreter_VarType type;
    int is_ref;
} Interpreter_Parameter;

#endif
