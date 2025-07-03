#ifndef INTERPRETER_TYPES_H
#define INTERPRETER_TYPES_H

#include <interpreter/config.h>

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BYTE,
    TYPE_IARRAY,
    TYPE_FARRAY,
    TYPE_STRING
} Interpreter_VarType;

typedef struct {
    Interpreter_VarType type;
    union {
        int i;
        float f;
        unsigned char b;
        struct { int data[INTERPRETER_MAX_ARRAY_SIZE]; int size; } iarray;
        struct { float data[INTERPRETER_MAX_ARRAY_SIZE]; int size; } farray;
        struct { unsigned char data[INTERPRETER_MAX_ARRAY_SIZE]; int size; } string;
    };
} Interpreter_Value;

typedef struct {
    char name[INTERPRETER_MAX_NAME];
    Interpreter_Value* value_ptr;
} Interpreter_Reference;

#endif
