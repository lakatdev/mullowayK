#include <interpreter/instructions.h>
#include <interpreter/interpreter.h>
#include <interpreter/instance.h>
#include <system/interrupts.h>
#include <system/interface.h>
#include <system/memory.h>
#include <system/storage.h>
#include <apps/runtime.h>
#include <drivers/serial.h>
#include <drivers/usb.h>

// Usually provided by the standard library.

typedef long long s64;
typedef unsigned long long u64;

s64 __divdi3(s64 num, s64 den)
{
    int neg = 0;
    if (num < 0) { num = -num; neg = !neg; }
    if (den < 0) { den = -den; neg = !neg; }
    u64 unum = (u64)num, uden = (u64)den, q = 0;
    while (unum >= uden) {
        u64 tmp = uden, multiple = 1;
        while ((tmp << 1) <= unum) { tmp <<= 1; multiple <<= 1; }
        unum -= tmp; q += multiple;
    }
    return neg ? -(s64)q : (s64)q;
}

s64 __moddi3(s64 num, s64 den)
{
    int neg = 0;
    if (num < 0) { num = -num; neg = 1; }
    if (den < 0) { den = -den; }
    u64 unum = (u64)num, uden = (u64)den;
    while (unum >= uden) {
        u64 tmp = uden;
        while ((tmp << 1) <= unum) { tmp <<= 1; }
        unum -= tmp;
    }
    return neg ? -(s64)unum : (s64)unum;
}

// End.

void interpreter_execute_add(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 5) {
        printf("Error: ADD instruction requires more tokens.\n");
        interpreter_halt();
        return;
    }

    const char* dest_var_name = tokens[0];
    const char* op1_token = tokens[2];
    const char* op2_token = tokens[4];

    Interpreter_Value val1 = interpreter_get_value_of_token(instance, op1_token);
    if (val1.type == -1) {
        printf("Error: Operand1 for ADD instruction is invalid or not found.\n");
        interpreter_halt();
        return;
    }

    Interpreter_Value val2 = interpreter_get_value_of_token(instance, op2_token);
    if (val2.type == -1) {
        printf("Error: Operand2 for ADD instruction is invalid or not found.\n");
        interpreter_halt();
        return;
    }

    Interpreter_Value result;
    memset(&result, 0, sizeof(Interpreter_Value));

    if ((val1.type == TYPE_INT || val1.type == TYPE_BYTE) && (val2.type == TYPE_INT || val2.type == TYPE_BYTE)) {
        result.type = TYPE_INT;
        long long v1 = (val1.type == TYPE_INT) ? val1.i : val1.b;
        long long v2 = (val2.type == TYPE_INT) ? val2.i : val2.b;
        result.i = v1 + v2;
    }
    else if ((val1.type == TYPE_FLOAT || val1.type == TYPE_INT || val1.type == TYPE_BYTE) &&
               (val2.type == TYPE_FLOAT || val2.type == TYPE_INT || val2.type == TYPE_BYTE)) {
        result.type = TYPE_FLOAT;
        float f1 = (val1.type == TYPE_FLOAT) ? val1.f : ((val1.type == TYPE_INT) ? (float)val1.i : (float)val1.b);
        float f2 = (val2.type == TYPE_FLOAT) ? val2.f : ((val2.type == TYPE_INT) ? (float)val2.i : (float)val2.b);
        result.f = f1 + f2;
    }
    else {
        printf("Error: Incompatible types for ADD operation. Only numeric types.\n");
        interpreter_halt();
        return;
    }
    interpreter_set_variable(instance, dest_var_name, result);
}

void interpreter_execute_add_assign(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 3) {
        printf("Error: ADD_ASSIGN instruction requires 3 tokens.\n");
        interpreter_halt();
        return;
    }

    const char* var_name = tokens[0];
    const char* value_token = tokens[2];

    Interpreter_Value* target_var_ptr = interpreter_get_variable(instance, var_name);
    if (target_var_ptr == (void*)0) {
         printf("Error: Variable not declared for ADD_ASSIGN operation.\n");
         interpreter_halt();
         return;
    }
    Interpreter_Value current_val = *target_var_ptr;

    Interpreter_Value val_to_add = interpreter_get_value_of_token(instance, value_token);
    if (val_to_add.type == -1) {
        printf("Error: Value for ADD_ASSIGN is invalid or not found.\n");
        interpreter_halt();
        return;
    }

    Interpreter_Value result;
    memset(&result, 0, sizeof(Interpreter_Value));

    if (current_val.type == TYPE_INT) {
        if (val_to_add.type == TYPE_INT) {
            result.type = TYPE_INT;
            result.i = current_val.i + val_to_add.i;
        }
        else if (val_to_add.type == TYPE_BYTE) {
            result.type = TYPE_INT;
            result.i = current_val.i + val_to_add.b;
        }
        else if (val_to_add.type == TYPE_FLOAT) {
            result.type = TYPE_FLOAT;
            result.f = (float)current_val.i + val_to_add.f;
        }
        else {
            printf("Error: Incompatible types for ADD_ASSIGN.\n");
            interpreter_halt();
            return;
        }
    }
    else if (current_val.type == TYPE_FLOAT) {
        if (val_to_add.type == TYPE_FLOAT) {
            result.type = TYPE_FLOAT;
            result.f = current_val.f + val_to_add.f;
        }
        else if (val_to_add.type == TYPE_INT) {
            result.type = TYPE_FLOAT;
            result.f = current_val.f + (float)val_to_add.i;
        }
        else if (val_to_add.type == TYPE_BYTE) {
            result.type = TYPE_FLOAT;
            result.f = current_val.f + (float)val_to_add.b;
        }
        else {
            printf("Error: Incompatible types for ADD_ASSIGN.\n");
            interpreter_halt();
            return;
        }
    }
    else if (current_val.type == TYPE_BYTE) {
        if (val_to_add.type == TYPE_BYTE) {
            result.type = TYPE_INT;
            result.i = (long long)current_val.b + val_to_add.b;
        }
        else if (val_to_add.type == TYPE_INT) {
            result.type = TYPE_INT;
            result.i = (long long)current_val.b + val_to_add.i;
        }
        else if (val_to_add.type == TYPE_FLOAT) {
            result.type = TYPE_FLOAT;
            result.f = (float)current_val.b + val_to_add.f;
        }
        else {
            printf("Error: Incompatible types for ADD_ASSIGN.\n");
            interpreter_halt();
            return;
        }
    } 
    else { 
        printf("Error: Incompatible types for ADD_ASSIGN.\n");
        interpreter_halt();
        return;
    }

    interpreter_set_variable(instance, var_name, result);
    return;
}

void interpreter_execute_array_set(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 5) {
        printf("Error: ARRAY_SET requires at least 5 tokens.\n");
        interpreter_halt();
        return;
    }
    const char* array_name = tokens[0];
    const char* index_token = tokens[2];
    const char* value_token = tokens[4];

    Interpreter_Value* array_var = interpreter_get_variable(instance, array_name);
    if (!array_var) {
        printf("Error: ARRAY_SET: Variable not found.\n");
        interpreter_halt();
        return;
    }
    Interpreter_Value index_val = interpreter_get_value_of_token(instance, index_token);
    int index = (index_val.type == TYPE_INT) ? index_val.i : (index_val.type == TYPE_BYTE ? index_val.b : -1);
    if (index < 0 || index >= INTERPRETER_MAX_ARRAY_SIZE) {
        printf("Error: ARRAY_SET: Index out of bounds.\n");
        interpreter_halt();
        return;
    }
    Interpreter_Value value = interpreter_get_value_of_token(instance, value_token);

    switch (array_var->type) {
        case TYPE_IARRAY:
            if (index >= array_var->iarray.size) array_var->iarray.size = index + 1;
            array_var->iarray.data[index] = value.i;
            break;
        case TYPE_FARRAY:
            if (index >= array_var->farray.size) array_var->farray.size = index + 1;
            array_var->farray.data[index] = value.f;
            break;
        case TYPE_STRING:
            if (index >= array_var->string.size) array_var->string.size = index + 1;
            array_var->string.data[index] = value.b;
            break;
        default:
            printf("Error: ARRAY_SET: Not an array type.\n");
            interpreter_halt();
            return;
    }
}

void interpreter_execute_array_get(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 5) {
        printf("Error: ARRAY_GET requires at least 5 tokens.\n");
        interpreter_halt();
        return;
    }
    const char* target_name = tokens[0];
    const char* array_name = tokens[2];
    const char* index_token = tokens[4];

    Interpreter_Value* array_var = interpreter_get_variable(instance, array_name);
    if (!array_var) {
        printf("Error: ARRAY_GET: Variable not found.\n");
        interpreter_halt();
        return;
    }
    Interpreter_Value index_val = interpreter_get_value_of_token(instance, index_token);
    int index = (index_val.type == TYPE_INT) ? index_val.i : (index_val.type == TYPE_BYTE ? index_val.b : -1);
    if (index < 0) {
        printf("Error: ARRAY_GET: Index out of bounds.\n");
        interpreter_halt();
        return;
    }

    Interpreter_Value result;
    memset(&result, 0, sizeof(Interpreter_Value));
    switch (array_var->type) {
        case TYPE_IARRAY:
            if (index >= array_var->iarray.size) {
                printf("Error: ARRAY_GET: Index out of bounds for iarray.\n");
                interpreter_halt();
                return;
            }
            result.type = TYPE_INT;
            result.i = array_var->iarray.data[index];
            break;
        case TYPE_FARRAY:
            if (index >= array_var->farray.size) {
                printf("Error: ARRAY_GET: Index out of bounds for farray.\n");
                interpreter_halt();
                return;
            }
            result.type = TYPE_FLOAT;
            result.f = array_var->farray.data[index];
            break;
        case TYPE_STRING:
            if (index >= array_var->string.size) {
                printf("Error: ARRAY_GET: Index out of bounds for string.\n");
                interpreter_halt();
                return;
            }
            result.type = TYPE_BYTE;
            result.b = array_var->string.data[index];
            break;
        default:
            printf("Error: ARRAY_GET: Not an array type.\n");
            interpreter_halt();
            return;
    }
    interpreter_set_variable(instance, target_name, result);
}

void interpreter_execute_free(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 2) {
        printf("Error: FREE requires at least 2 tokens.\n");
        interpreter_halt();
        return;
    }
    const char* var_name = tokens[1];
    Interpreter_Value* var = interpreter_get_variable(instance, var_name);
    if (!var) {
        printf("Error: FREE: Variable not found.\n");
        interpreter_halt();
        return;
    }
    switch (var->type) {
        case TYPE_IARRAY:
            memset(var->iarray.data, 0, sizeof(var->iarray.data));
            var->iarray.size = 0;
            break;
        case TYPE_FARRAY:
            memset(var->farray.data, 0, sizeof(var->farray.data));
            var->farray.size = 0;
            break;
        case TYPE_STRING:
            memset(var->string.data, 0, sizeof(var->string.data));
            var->string.size = 0;
            break;
        default:
            printf("Error: FREE can only be used on array types.\n");
            interpreter_halt();
            return;
    }
}

void interpreter_execute_assign(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 3) {
        printf("Error: Assign needs 3 tokens.\n");
        interpreter_halt();
        return;
    }

    const char* dest_var_name = tokens[0];
    const char* value_token = tokens[2];

    Interpreter_Value value_to_assign = interpreter_get_value_of_token(instance, value_token);

    if (value_to_assign.type == -1) {
        printf("Error: Value for assignment is invalid or not found.\n");
        interpreter_halt();
        return;
    }

    interpreter_set_variable(instance, dest_var_name, value_to_assign);
}

void interpreter_execute_call(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 2) {
        printf("Error: CALL instruction requires function name\n");
        interpreter_halt();
        return;
    }

    const char* function_name = tokens[1];
    int arg_count = token_count - 2;
    
    char** arg_names = (void*)0;
    if (arg_count > 0) {
        arg_names = &tokens[2];
    }

    int result = interpreter_call_function(instance, function_name, arg_names, arg_count);
    if (result == -1) {
        return;
    }
}

void interpreter_execute_call_assign(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 4) {
        printf("Error: CALL_ASSIGN instruction requires more tokens.\n");
        interpreter_halt();
        return;
    }

    const char* dest_var_name = tokens[0];
    const char* function_name = tokens[3];
    int arg_count = token_count - 4;

    char** arg_names = (void*)0;
    if (arg_count > 0) {
        arg_names = &tokens[4];
    }

    Interpreter_CallFrame* current_frame = &instance->call_stack[instance->stack_pointer];
    current_frame->return_var = dest_var_name;

    int result = interpreter_call_function(instance, function_name, arg_names, arg_count);
    if (result == -1) {
        return;
    }
}

void interpreter_execute_cat(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 4) {
        printf("Error: CAT instruction requires at least 4 tokens.\n");
        interpreter_halt();
        return;
    }

    const char* dest_var_name = tokens[1];
    const char* mode = tokens[2];

    Interpreter_Value* dest_var = interpreter_get_variable(instance, dest_var_name);
    if (!dest_var || dest_var->type != TYPE_STRING) {
        printf("Error: Destination variable is not a string.\n");
        interpreter_halt();
        return;
    }

    if (interpreter_ci_strcmp(mode, "const") == 0) {
        unsigned long long int dest_len = dest_var->string.size;
        unsigned long long int max_len = INTERPRETER_MAX_ARRAY_SIZE;
        char temp[INTERPRETER_MAX_ARRAY_SIZE];
        unsigned long long int appended = 0;

        for (int i = 3; i < token_count; ++i) {
            interpreter_parse_const_escapes(tokens[i], temp, sizeof(temp));
            unsigned long long int chunk = strlen(temp);
            if (dest_len + appended + chunk > max_len)
                chunk = max_len - dest_len - appended;
            memcpy(dest_var->string.data + dest_len + appended, temp, chunk);
            appended += chunk;
            if (i < token_count - 1 && dest_len + appended < max_len) {
                dest_var->string.data[dest_len + appended] = ' ';
                appended += 1;
            }
        }
        dest_var->string.size = dest_len + appended;
    }
    else if (interpreter_ci_strcmp(mode, "string") == 0) {
        if (token_count < 4) {
            printf("Error: CAT string mode requires a source variable.\n");
            interpreter_halt();
            return;
        }
        const char* src_var_name = tokens[3];
        Interpreter_Value* src_var = interpreter_get_variable(instance, src_var_name);
        if (!src_var || src_var->type != TYPE_STRING) {
            printf("Error: Source variable is not a string.\n");
            interpreter_halt();
            return;
        }
        unsigned long long int dest_len = dest_var->string.size;
        unsigned long long int src_len = src_var->string.size;
        unsigned long long int max_len = INTERPRETER_MAX_ARRAY_SIZE;

        if (dest_len + src_len > max_len) {
            src_len = max_len - dest_len;
        }

        memcpy(dest_var->string.data + dest_len, src_var->string.data, src_len);
        dest_var->string.size = dest_len + src_len;
    }
    else {
        printf("Error: Unknown CAT mode.\n");
        interpreter_halt();
        return;
    }
}

void interpreter_execute_clear(Interpreter_Instance* instance, char** tokens, int token_count)
{
    app_runtime_clear_buffer();
}

void interpreter_execute_declare(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 3) {
        printf("Error: DECLARE instruction requires variable name and type.\n");
        interpreter_halt();
        return;
    }

    const char* var_name = tokens[1];
    const char* type_str = tokens[2];

    Interpreter_VarType var_type = interpreter_get_type(type_str);

    if (var_type == -1) {
        printf("Error: Unknown variable type for variable.\n");
        interpreter_halt();
        return;
    }

    interpreter_declare_variable(instance, var_name, var_type);
}

void interpreter_execute_subtract(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 5) {
        printf("Error: SUBTRACT instruction requires 5 tokens.\n");
        interpreter_halt();
        return;
    }

    const char* dest_var_name = tokens[0];
    const char* op1_token = tokens[2];
    const char* op2_token = tokens[4];

    Interpreter_Value val1 = interpreter_get_value_of_token(instance, op1_token);
    if (val1.type == -1) {
        printf("Error: Operand1 for SUBTRACT instruction is invalid or not found.\n");
        interpreter_halt();
        return;
    }

    Interpreter_Value val2 = interpreter_get_value_of_token(instance, op2_token);
    if (val2.type == -1) {
        printf("Error: Operand2 for SUBTRACT instruction is invalid or not found.\n");
        interpreter_halt();
        return;
    }

    Interpreter_Value result;
    memset(&result, 0, sizeof(Interpreter_Value));

    if ((val1.type == TYPE_INT || val1.type == TYPE_BYTE) && (val2.type == TYPE_INT || val2.type == TYPE_BYTE)) {
        result.type = TYPE_INT;
        long long v1 = (val1.type == TYPE_INT) ? val1.i : val1.b;
        long long v2 = (val2.type == TYPE_INT) ? val2.i : val2.b;
        result.i = v1 - v2;
    }
    else if ((val1.type == TYPE_FLOAT || val1.type == TYPE_INT || val1.type == TYPE_BYTE) &&
               (val2.type == TYPE_FLOAT || val2.type == TYPE_INT || val2.type == TYPE_BYTE)) {
        result.type = TYPE_FLOAT;
        float f1 = (val1.type == TYPE_FLOAT) ? val1.f : ((val1.type == TYPE_INT) ? (float)val1.i : (float)val1.b);
        float f2 = (val2.type == TYPE_FLOAT) ? val2.f : ((val2.type == TYPE_INT) ? (float)val2.i : (float)val2.b);
        result.f = f1 - f2;
    }
    else {
        printf("Error: Incompatible types for SUBTRACT operation. Only numeric types.\n");
        interpreter_halt();
        return;
    }
    interpreter_set_variable(instance, dest_var_name, result);
}

void interpreter_execute_subtract_assign(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 3) {
        printf("Error: SUBTRACT_ASSIGN instruction requires 3 tokens.\n");
        interpreter_halt();
        return;
    }

    const char* var_name = tokens[0];
    const char* value_token = tokens[2];

    Interpreter_Value* target_var_ptr = interpreter_get_variable(instance, var_name);
    if (target_var_ptr == (void*)0) {
         printf("Error: Variable not declared for SUBTRACT_ASSIGN operation.\n");
         interpreter_halt();
         return;
    }
    Interpreter_Value current_val = *target_var_ptr;
    Interpreter_Value val_to_subtract = interpreter_get_value_of_token(instance, value_token);

    if (val_to_subtract.type == -1) {
        printf("Error: Value for SUBTRACT_ASSIGN is invalid or not found.\n");
        interpreter_halt();
        return;
    }

    Interpreter_Value result;
    memset(&result, 0, sizeof(Interpreter_Value));

    if (current_val.type == TYPE_INT) {
        if (val_to_subtract.type == TYPE_INT) {
            result.type = TYPE_INT;
            result.i = current_val.i - val_to_subtract.i;
        }
        else if (val_to_subtract.type == TYPE_BYTE) {
            result.type = TYPE_INT;
            result.i = current_val.i - val_to_subtract.b;
        }
        else if (val_to_subtract.type == TYPE_FLOAT) {
            result.type = TYPE_FLOAT;
            result.f = (float)current_val.i - val_to_subtract.f;
        }
        else {
            printf("Error: Incompatible types for SUBTRACT_ASSIGN.\n");
            interpreter_halt();
            return;
        }
    }
    else if (current_val.type == TYPE_FLOAT) {
        if (val_to_subtract.type == TYPE_FLOAT) {
            result.type = TYPE_FLOAT;
            result.f = current_val.f - val_to_subtract.f;
        }
        else if (val_to_subtract.type == TYPE_INT) {
            result.type = TYPE_FLOAT;
            result.f = current_val.f - (float)val_to_subtract.i;
        }
        else if (val_to_subtract.type == TYPE_BYTE) {
            result.type = TYPE_FLOAT;
            result.f = current_val.f - (float)val_to_subtract.b;
        }
        else {
            printf("Error: Incompatible types for SUBTRACT_ASSIGN.\n");
            interpreter_halt();
            return;
        }
    }
    else if (current_val.type == TYPE_BYTE) {
        if (val_to_subtract.type == TYPE_BYTE) {
            result.type = TYPE_INT; 
            result.i = (long long)current_val.b - val_to_subtract.b;
        }
        else if (val_to_subtract.type == TYPE_INT) {
            result.type = TYPE_INT;
            result.i = (long long)current_val.b - val_to_subtract.i;
        }
        else if (val_to_subtract.type == TYPE_FLOAT) {
            result.type = TYPE_FLOAT;
            result.f = (float)current_val.b - val_to_subtract.f;
        }
        else {
            printf("Error: Incompatible types for SUBTRACT_ASSIGN.\n");
            interpreter_halt();
            return;
        }
    }
    else {
        printf("Error: Variable has an unsupported type for SUBTRACT_ASSIGN.\n");
        interpreter_halt();
        return;
    }
    interpreter_set_variable(instance, var_name, result);
}

void interpreter_execute_multiply(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 5) {
        printf("Error: MULTIPLY instruction requires 5 tokens.\n");
        interpreter_halt();
        return;
    }

    const char* dest_var_name = tokens[0];
    const char* op1_token = tokens[2];
    const char* op2_token = tokens[4];

    Interpreter_Value val1 = interpreter_get_value_of_token(instance, op1_token);
    if (val1.type == -1) {
        printf("Error: Operand1 for MULTIPLY instruction is invalid or not found.\n");
        interpreter_halt();
        return;
    }

    Interpreter_Value val2 = interpreter_get_value_of_token(instance, op2_token);
    if (val2.type == -1) {
        printf("Error: Operand2 for MULTIPLY instruction is invalid or not found.\n");
        interpreter_halt();
        return;
    }

    Interpreter_Value result;
    memset(&result, 0, sizeof(Interpreter_Value));

    if ((val1.type == TYPE_INT || val1.type == TYPE_BYTE) && (val2.type == TYPE_INT || val2.type == TYPE_BYTE)) {
        result.type = TYPE_INT;
        long long v1 = (val1.type == TYPE_INT) ? val1.i : val1.b;
        long long v2 = (val2.type == TYPE_INT) ? val2.i : val2.b;
        result.i = v1 * v2;
    }
    else if ((val1.type == TYPE_FLOAT || val1.type == TYPE_INT || val1.type == TYPE_BYTE) &&
               (val2.type == TYPE_FLOAT || val2.type == TYPE_INT || val2.type == TYPE_BYTE)) {
        result.type = TYPE_FLOAT;
        float f1 = (val1.type == TYPE_FLOAT) ? val1.f : ((val1.type == TYPE_INT) ? (float)val1.i : (float)val1.b);
        float f2 = (val2.type == TYPE_FLOAT) ? val2.f : ((val2.type == TYPE_INT) ? (float)val2.i : (float)val2.b);
        result.f = f1 * f2;
    }
    else {
        printf("Error: Incompatible types for MULTIPLY operation. Only numeric types.\n");
        interpreter_halt();
        return;
    }
    interpreter_set_variable(instance, dest_var_name, result);
}

void interpreter_execute_multiply_assign(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 3) {
        printf("Error: MULTIPLY_ASSIGN instruction requires 3 tokens.\n");
        interpreter_halt();
        return;
    }

    const char* var_name = tokens[0];
    const char* value_token = tokens[2];

    Interpreter_Value* target_var_ptr = interpreter_get_variable(instance, var_name);
    if (target_var_ptr == (void*)0) {
         printf("Error: Variable not declared for MULTIPLY_ASSIGN operation.\n");
         interpreter_halt();
         return;
    }
    Interpreter_Value current_val = *target_var_ptr;
    Interpreter_Value val_to_multiply = interpreter_get_value_of_token(instance, value_token);

    if (val_to_multiply.type == -1) {
        printf("Error: Value for MULTIPLY_ASSIGN is invalid or not found.\n");
        interpreter_halt();
        return;
    }

    Interpreter_Value result;
    memset(&result, 0, sizeof(Interpreter_Value));

    if (current_val.type == TYPE_INT) {
        if (val_to_multiply.type == TYPE_INT) {
            result.type = TYPE_INT;
            result.i = current_val.i * val_to_multiply.i;
        }
        else if (val_to_multiply.type == TYPE_BYTE) {
            result.type = TYPE_INT;
            result.i = current_val.i * val_to_multiply.b;
        }
        else if (val_to_multiply.type == TYPE_FLOAT) {
            result.type = TYPE_FLOAT;
            result.f = (float)current_val.i * val_to_multiply.f;
        }
        else {
            printf("Error: Incompatible types for MULTIPLY_ASSIGN.\n");
            interpreter_halt();
            return;
        }
    }
    else if (current_val.type == TYPE_FLOAT) {
        if (val_to_multiply.type == TYPE_FLOAT) {
            result.type = TYPE_FLOAT;
            result.f = current_val.f * val_to_multiply.f;
        }
        else if (val_to_multiply.type == TYPE_INT) {
            result.type = TYPE_FLOAT;
            result.f = current_val.f * (float)val_to_multiply.i;
        }
        else if (val_to_multiply.type == TYPE_BYTE) {
            result.type = TYPE_FLOAT;
            result.f = current_val.f * (float)val_to_multiply.b;
        }
        else {
            printf("Error: Incompatible types for MULTIPLY_ASSIGN.\n");
            interpreter_halt();
            return;
        }
    }
    else if (current_val.type == TYPE_BYTE) {
        if (val_to_multiply.type == TYPE_BYTE) {
            result.type = TYPE_INT; 
            result.i = (long long)current_val.b * val_to_multiply.b;
        }
        else if (val_to_multiply.type == TYPE_INT) {
            result.type = TYPE_INT;
            result.i = (long long)current_val.b * val_to_multiply.i;
        }
        else if (val_to_multiply.type == TYPE_FLOAT) {
            result.type = TYPE_FLOAT;
            result.f = (float)current_val.b * val_to_multiply.f;
        }
        else {
            printf("Error: Incompatible types for MULTIPLY_ASSIGN.\n");
            interpreter_halt();
            return;
        }
    }
    else {
        printf("Error: Variable has an unsupported type for MULTIPLY_ASSIGN.\n");
        interpreter_halt();
        return;
    }
    interpreter_set_variable(instance, var_name, result);
}

void interpreter_execute_divide(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 5) {
        printf("Error: DIVIDE instruction requires 5 tokens.\n");
        interpreter_halt();
        return;
    }

    const char* dest_var_name = tokens[0];
    const char* op1_token = tokens[2];
    const char* op2_token = tokens[4];

    Interpreter_Value val1 = interpreter_get_value_of_token(instance, op1_token);
    if (val1.type == -1) {
        printf("Error: Operand1 for DIVIDE instruction is invalid or not found.\n");
        interpreter_halt();
        return;
    }

    Interpreter_Value val2 = interpreter_get_value_of_token(instance, op2_token);
    if (val2.type == -1) {
        printf("Error: Operand2 for DIVIDE instruction is invalid or not found.\n");
        interpreter_halt();
        return;
    }

    Interpreter_Value result;
    memset(&result, 0, sizeof(Interpreter_Value));

    if ((val1.type == TYPE_INT || val1.type == TYPE_BYTE) && (val2.type == TYPE_INT || val2.type == TYPE_BYTE)) {
        result.type = TYPE_INT; 
        long long v1 = (val1.type == TYPE_INT) ? val1.i : val1.b;
        long long v2 = (val2.type == TYPE_INT) ? val2.i : val2.b;
        if (v2 == 0) {
            printf("Error: Division by zero in DIVIDE operation.\n");
            interpreter_halt();
            return;
        }
        result.i = v1 / v2;
    }
    else if ((val1.type == TYPE_FLOAT || val1.type == TYPE_INT || val1.type == TYPE_BYTE) &&
               (val2.type == TYPE_FLOAT || val2.type == TYPE_INT || val2.type == TYPE_BYTE)) {
        result.type = TYPE_FLOAT;
        float f1 = (val1.type == TYPE_FLOAT) ? val1.f : ((val1.type == TYPE_INT) ? (float)val1.i : (float)val1.b);
        float f2 = (val2.type == TYPE_FLOAT) ? val2.f : ((val2.type == TYPE_INT) ? (float)val2.i : (float)val2.b);
        if (f2 == 0.0f) {
            printf("Error: Division by zero in DIVIDE operation.\n");
            interpreter_halt();
            return;
        }
        result.f = f1 / f2;
    }
    else {
        printf("Error: Incompatible types for DIVIDE operation. Only numeric types.\n");
        interpreter_halt();
        return;
    }
    interpreter_set_variable(instance, dest_var_name, result);
}

void interpreter_execute_divide_assign(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 3) {
        printf("Error: DIVIDE_ASSIGN instruction requires 3 tokens.\n");
        interpreter_halt();
        return;
    }

    const char* var_name = tokens[0];
    const char* value_token = tokens[2];

    Interpreter_Value* target_var_ptr = interpreter_get_variable(instance, var_name);
    if (target_var_ptr == (void*)0) {
        printf("Error: Variable not declared for DIVIDE_ASSIGN operation.\n");
        interpreter_halt();
        return;
    }
    Interpreter_Value current_val = *target_var_ptr;
    Interpreter_Value val_to_divide = interpreter_get_value_of_token(instance, value_token);

    if (val_to_divide.type == -1) {
        printf("Error: Value for DIVIDE_ASSIGN is invalid or not found.\n");
        interpreter_halt();
        return;
    }

    Interpreter_Value result;
    memset(&result, 0, sizeof(Interpreter_Value));

    if (current_val.type == TYPE_INT) {
        long long divisor_i = 0;
        if (val_to_divide.type == TYPE_INT) {
            divisor_i = val_to_divide.i;
        }
        else if (val_to_divide.type == TYPE_BYTE) {
            divisor_i = val_to_divide.b;
        }
        else if (val_to_divide.type == TYPE_FLOAT) { 
            if (val_to_divide.f == 0.0f) {
                printf("Error: Division by zero in DIVIDE_ASSIGN.\n");
                interpreter_halt(); return;
            }
            result.type = TYPE_FLOAT;
            result.f = (float)current_val.i / val_to_divide.f;
            interpreter_set_variable(instance, var_name, result);
            return;
        }
        else {
            printf("Error: Incompatible types for DIVIDE_ASSIGN.\n");
            interpreter_halt(); return;
        }
        
        if (divisor_i == 0) {
            printf("Error: Division by zero in DIVIDE_ASSIGN.\n");
            interpreter_halt(); return;
        }
        result.type = TYPE_INT; 
        result.i = current_val.i / divisor_i;

    }
    else if (current_val.type == TYPE_FLOAT) {
        float divisor_f = 0.0f;
        if (val_to_divide.type == TYPE_FLOAT) {
            divisor_f = val_to_divide.f;
        }
        else if (val_to_divide.type == TYPE_INT) {
            divisor_f = (float)val_to_divide.i;
        }
        else if (val_to_divide.type == TYPE_BYTE) {
            divisor_f = (float)val_to_divide.b;
        }
        else {
            printf("Error: Incompatible types for DIVIDE_ASSIGN.\n");
            interpreter_halt(); return;
        }

        if (divisor_f == 0.0f) {
            printf("Error: Division by zero in DIVIDE_ASSIGN.\n");
            interpreter_halt(); return;
        }
        result.type = TYPE_FLOAT;
        result.f = current_val.f / divisor_f;

    }
    else if (current_val.type == TYPE_BYTE) {
        long long divisor_b_as_i = 0;
        if (val_to_divide.type == TYPE_BYTE) {
        divisor_b_as_i = val_to_divide.b;
        }
        else if (val_to_divide.type == TYPE_INT) {
            divisor_b_as_i = val_to_divide.i;
        }
        else if (val_to_divide.type == TYPE_FLOAT) { 
            if (val_to_divide.f == 0.0f) {
                printf("Error: Division by zero in DIVIDE_ASSIGN.\n");
                interpreter_halt(); return;
            }
            result.type = TYPE_FLOAT;
            result.f = (float)current_val.b / val_to_divide.f;
            interpreter_set_variable(instance, var_name, result);
            return;
        }
        else {
            printf("Error: Incompatible types for DIVIDE_ASSIGN.\n");
            interpreter_halt(); return;
        }

        if (divisor_b_as_i == 0) {
            printf("Error: Division by zero in DIVIDE_ASSIGN.\n");
            interpreter_halt(); return;
        }
        result.type = TYPE_INT; 
        result.i = (long long)current_val.b / divisor_b_as_i;
    }
    else {
        printf("Error: Variable has an unsupported type for DIVIDE_ASSIGN.\n");
        interpreter_halt();
        return;
    }
    interpreter_set_variable(instance, var_name, result);
}

void interpreter_execute_mod(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 5) {
        printf("Error: MOD instruction requires 5 tokens.\n");
        interpreter_halt();
        return;
    }

    const char* dest_var_name = tokens[0];
    const char* op1_token = tokens[2];
    const char* op2_token = tokens[4];

    Interpreter_Value val1 = interpreter_get_value_of_token(instance, op1_token);
    if (val1.type == -1) {
        printf("Error: Operand1 for MOD instruction is invalid or not found.\n");
        interpreter_halt();
        return;
    }

    Interpreter_Value val2 = interpreter_get_value_of_token(instance, op2_token);
    if (val2.type == -1) {
        printf("Error: Operand2 for MOD instruction is invalid or not found.\n");
        interpreter_halt();
        return;
    }

    Interpreter_Value result;
    memset(&result, 0, sizeof(Interpreter_Value));

    if ((val1.type == TYPE_INT || val1.type == TYPE_BYTE) && (val2.type == TYPE_INT || val2.type == TYPE_BYTE)) {
        result.type = TYPE_INT;
        long long v1 = (val1.type == TYPE_INT) ? val1.i : val1.b;
        long long v2 = (val2.type == TYPE_INT) ? val2.i : val2.b;
        if (v2 == 0) {
            printf("Error: Modulo by zero in MOD operation.\n");
            interpreter_halt();
            return;
        }
        result.i = v1 % v2;
    }
    else {
        printf("Error: Incompatible types for MOD operation. Only integer types.\n");
        interpreter_halt();
        return;
    }
    interpreter_set_variable(instance, dest_var_name, result);
}

void interpreter_execute_mod_assign(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 3) {
        printf("Error: MOD_ASSIGN instruction requires 3 tokens.\n");
        interpreter_halt();
        return;
    }

    const char* var_name = tokens[0];
    const char* value_token = tokens[2];

    Interpreter_Value* target_var_ptr = interpreter_get_variable(instance, var_name);
    if (target_var_ptr == (void*)0) {
         printf("Error: Variable not declared for MOD_ASSIGN operation.\n");
         interpreter_halt();
         return;
    }
    Interpreter_Value current_val = *target_var_ptr;
    Interpreter_Value val_to_mod = interpreter_get_value_of_token(instance, value_token);

    if (val_to_mod.type == -1) {
        printf("Error: Value for MOD_ASSIGN is invalid or not found.\n");
        interpreter_halt();
        return;
    }

    Interpreter_Value result;
    memset(&result, 0, sizeof(Interpreter_Value));

    if ((current_val.type == TYPE_INT || current_val.type == TYPE_BYTE) &&
        (val_to_mod.type == TYPE_INT || val_to_mod.type == TYPE_BYTE)) {
        
        long long current_v = (current_val.type == TYPE_INT) ? current_val.i : current_val.b;
        long long val_v = (val_to_mod.type == TYPE_INT) ? val_to_mod.i : val_to_mod.b;

        if (val_v == 0) {
            printf("Error: Modulo by zero in MOD_ASSIGN.\n");
            interpreter_halt();
            return;
        }
        result.type = TYPE_INT; 
        result.i = current_v % val_v;
    }
    else {
        printf("Error: Incompatible types for MOD_ASSIGN. Both variable and value must be integer types.\n");
        interpreter_halt();
        return;
    }
    interpreter_set_variable(instance, var_name, result);
}

void interpreter_execute_exec(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 3) {
        printf("Error: EXEC requires a mode and argument.\n");
        interpreter_halt();
        return;
    }

    const char* mode = tokens[1];

    if (interpreter_ci_strcmp(mode, "const") == 0) {
        char command[1024] = {0};
        unsigned long long int pos = 0;
        for (int i = 2; i < token_count; ++i) {
            unsigned long long int len = strlen(tokens[i]);
            if (pos + len + 2 >= sizeof(command)) break;
            memcpy(command + pos, tokens[i], len);
            pos += len;
            if (i < token_count - 1) command[pos++] = ' ';
        }
        command[pos] = '\0';
        
        if (app_runtime_push_instance_from_file(command) != 0) {
            printf("Error: EXEC: Failed to execute file.\n");
            interpreter_halt();
            return;
        }
    }
    else if (interpreter_ci_strcmp(mode, "string") == 0) {
        Interpreter_Value* str_var = interpreter_get_variable(instance, tokens[2]);
        if (!str_var || str_var->type != TYPE_STRING) {
            printf("Error: EXEC string: variable not found or not a string.\n");
            interpreter_halt();
            return;
        }
        char command[INTERPRETER_MAX_ARRAY_SIZE + 1] = {0};
        memcpy(command, str_var->string.data, str_var->string.size);
        command[str_var->string.size] = '\0';
        
        if (app_runtime_push_instance_from_file(command) != 0) {
            printf("Error: EXEC: Failed to execute file.\n");
            interpreter_halt();
            return;
        }
    }
    else {
        printf("Error: Unknown EXEC mode.\n");
        interpreter_halt();
    }
}

void interpreter_execute_if(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 4) {
        printf("Error: IF requires at least 4 tokens (if X > Y).\n");
        interpreter_halt();
        return;
    }
    Interpreter_Value left = interpreter_get_value_of_token(instance, tokens[1]);
    Interpreter_Value right = interpreter_get_value_of_token(instance, tokens[3]);
    const char* op = tokens[2];

    if (!interpreter_compare(left, right, op)) {
        int end_line = interpreter_find_matching_end(instance, instance->execution_position);
        if (end_line == -1) {
            printf("Error: No matching end for if.\n");
            interpreter_halt();
            return;
        }
        instance->execution_position = end_line + 1;
    }
}

void interpreter_execute_while(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 4) {
        printf("Error: WHILE requires at least 4 tokens (while X > Y).\n");
        interpreter_halt();
        return;
    }
    Interpreter_Value left = interpreter_get_value_of_token(instance, tokens[1]);
    Interpreter_Value right = interpreter_get_value_of_token(instance, tokens[3]);
    const char* op = tokens[2];

    if (!interpreter_compare(left, right, op)) {
        int end_line = interpreter_find_matching_end(instance, instance->execution_position);
        if (end_line == -1) {
            printf("Error: No matching end for while.\n");
            interpreter_halt();
            return;
        }
        instance->execution_position = end_line + 1;
    }
}

void interpreter_execute_end(Interpreter_Instance* instance, char** tokens, int token_count)
{
    int depth = 0;
    for (int i = instance->execution_position - 1; i >= 0; --i) {
        char* first_token = instance->parsed_code[i][0];
        if (interpreter_ci_strcmp(first_token, "end") == 0) {
            depth++;
        }
        else if (interpreter_ci_strcmp(first_token, "while") == 0) {
            if (depth == 0) {
                instance->execution_position = i;
                return;
            }
            depth--;
        }
        else if (interpreter_ci_strcmp(first_token, "if") == 0) {
            if (depth == 0) {
                return;
            }
            depth--;
        }
    }
}

void interpreter_execute_input(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 3) {
        printf("Error: INPUT instruction requires a mode and a variable name.\n");
        interpreter_halt();
        return;
    }

    const char* mode = tokens[1];
    const char* var_name = tokens[2];

    if (interpreter_ci_strcmp(mode, "$") == 0) {
        Interpreter_Value* var = interpreter_get_variable(instance, var_name);
        if (!var) {
            printf("Error: INPUT: Variable not found or not declared.\n");
            interpreter_halt();
            return;
        }
        
        instance->waiting_for_input = 1;
        instance->input_variable_name = (char*)var_name;
        instance->input_ready = 0;
        instance->input_buffer[0] = '\0';
        
        app_runtime_request_input();
        
        instance->input_mode = INPUT_MODE_NUMERIC;
    }
    else if (interpreter_ci_strcmp(mode, "ascii") == 0) {
        instance->waiting_for_input = 1;
        instance->input_variable_name = (char*)var_name;
        instance->input_ready = 0;
        instance->input_buffer[0] = '\0';
        
        app_runtime_request_input();
        
        instance->input_mode = INPUT_MODE_ASCII;
    }
    else if (interpreter_ci_strcmp(mode, "string") == 0) {
        Interpreter_Value* str_var = interpreter_get_variable(instance, var_name);
        if (!str_var || str_var->type != TYPE_STRING) {
            printf("Error: Variable is not a string.\n");
            interpreter_halt();
            return;
        }
        
        instance->waiting_for_input = 1;
        instance->input_variable_name = (char*)var_name;
        instance->input_ready = 0;
        instance->input_buffer[0] = '\0';
        
        app_runtime_request_input();
    
        instance->input_mode = INPUT_MODE_STRING;
    }
    else if (interpreter_ci_strcmp(mode, "serial") == 0) {
        if (token_count < 4) {
            printf("Error: INPUT serial requires variable name and byte count.\n");
            interpreter_halt();
            return;
        }
        
        Interpreter_Value* str_var = interpreter_get_variable(instance, var_name);
        if (!str_var || str_var->type != TYPE_STRING) {
            printf("Error: INPUT serial: Variable is not a string.\n");
            interpreter_halt();
            return;
        }
        
        Interpreter_Value byte_count_val = interpreter_get_value_of_token(instance, tokens[3]);
        if (byte_count_val.type != TYPE_INT && byte_count_val.type != TYPE_BYTE) {
            printf("Error: INPUT serial: Byte count must be integer or byte.\n");
            interpreter_halt();
            return;
        }
        
        int byte_count = (byte_count_val.type == TYPE_INT) ? byte_count_val.i : byte_count_val.b;
        if (byte_count < 0 || byte_count > INTERPRETER_MAX_ARRAY_SIZE) {
            printf("Error: INPUT serial: Invalid byte count.\n");
            interpreter_halt();
            return;
        }
        
        instance->waiting_for_input = 1;
        instance->input_variable_name = (char*)var_name;
        instance->input_ready = 0;
        instance->input_mode = INPUT_MODE_SERIAL;
        instance->serial_bytes_to_read = byte_count;
        instance->serial_bytes_read = 0;
    }
    else if (interpreter_ci_strcmp(mode, "usb") == 0) {
        if (token_count < 3) {
            printf("Error: INPUT usb requires a variable name.\n");
            interpreter_halt();
            return;
        }
        
        Interpreter_Value* str_var = interpreter_get_variable(instance, var_name);
        if (!str_var || str_var->type != TYPE_STRING) {
            printf("Error: INPUT usb: Variable is not a string.\n");
            interpreter_halt();
            return;
        }
        
        int byte_count = -1;
        if (token_count >= 4) {
            Interpreter_Value byte_count_val = interpreter_get_value_of_token(instance, tokens[3]);
            if (byte_count_val.type != TYPE_INT && byte_count_val.type != TYPE_BYTE) {
                printf("Error: INPUT usb: Byte count must be integer or byte.\n");
                interpreter_halt();
                return;
            }
            byte_count = (byte_count_val.type == TYPE_INT) ? byte_count_val.i : byte_count_val.b;
            if (byte_count < 0 || byte_count > INTERPRETER_MAX_ARRAY_SIZE) {
                printf("Error: INPUT usb: Invalid byte count.\n");
                interpreter_halt();
                return;
            }
        }
        
        instance->waiting_for_input = 1;
        instance->input_variable_name = (char*)var_name;
        instance->input_ready = 0;
        instance->input_mode = INPUT_MODE_USB_SERIAL;
        instance->serial_bytes_to_read = byte_count;
        instance->serial_bytes_read = 0;
    }
    else {
        printf("Error: Unknown INPUT mode.\n");
        interpreter_halt();
        return;
    }
}

void interpreter_execute_save(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 4) {
        printf("Error: SAVE requires type, variable, and path.\n");
        interpreter_halt();
        return;
    }
    
    const char* type = tokens[1];
    const char* var_name = tokens[2];
    const char* path = tokens[3];

    Interpreter_Value* var = interpreter_get_variable(instance, var_name);
    if (!var) {
        printf("Error: SAVE: Variable not found.\n");
        interpreter_halt();
        return;
    }

    unsigned int buffer_pos = 0;
    
    if (interpreter_ci_strcmp(type, "iarray") == 0 && var->type == TYPE_IARRAY) {
        unsigned int required_size = var->iarray.size * sizeof(int);
        if (required_size > STORAGE_RECORD_SIZE) {
            printf("Error: SAVE: Data too large.\n");
            interpreter_halt();
            return;
        }
        memcpy(interpreter_public_buffer + buffer_pos, var->iarray.data, var->iarray.size * sizeof(int));
        buffer_pos += var->iarray.size * sizeof(int);
    }
    else if (interpreter_ci_strcmp(type, "farray") == 0 && var->type == TYPE_FARRAY) {
        unsigned int required_size = var->farray.size * sizeof(float);
        if (required_size > STORAGE_RECORD_SIZE) {
            printf("Error: SAVE: Data too large.\n");
            interpreter_halt();
            return;
        }
        memcpy(interpreter_public_buffer + buffer_pos, var->farray.data, var->farray.size * sizeof(float));
        buffer_pos += var->farray.size * sizeof(float);
    }
    else if (interpreter_ci_strcmp(type, "string") == 0 && var->type == TYPE_STRING) {
        unsigned int required_size = var->string.size * sizeof(unsigned char);
        if (required_size > STORAGE_RECORD_SIZE) {
            printf("Error: SAVE: Data too large.\n");
            interpreter_halt();
            return;
        }
        memcpy(interpreter_public_buffer + buffer_pos, var->string.data, var->string.size * sizeof(unsigned char));
        buffer_pos += var->string.size * sizeof(unsigned char);
    }
    else {
        printf("Error: SAVE: Type mismatch or unsupported type.\n");
        interpreter_halt();
        return;
    }

    write_to_storage(path, interpreter_public_buffer, buffer_pos);
}

void interpreter_execute_load(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 4) {
        printf("Error: LOAD requires type, variable, and path.\n");
        interpreter_halt();
        return;
    }
    
    const char* type = tokens[1];
    const char* var_name = tokens[2];
    const char* path = tokens[3];

    Interpreter_Value* var = interpreter_get_variable(instance, var_name);
    if (!var) {
        printf("Error: LOAD: Variable not found.\n");
        interpreter_halt();
        return;
    }

    if (!files_exists(path)) {
        printf("Error: Could not find file for reading.\n");
        interpreter_halt();
        return;
    }

    unsigned int buffer_size = 0;
    read_from_storage(path, interpreter_public_buffer, &buffer_size);

    if (buffer_size == 0) {
        printf("Error: File is empty or could not be read.\n");
        interpreter_halt();
        return;
    }
    
    if (interpreter_ci_strcmp(type, "iarray") == 0 && var->type == TYPE_IARRAY) {
        int size = buffer_size / sizeof(int);
        if (size < 0 || size > INTERPRETER_MAX_ARRAY_SIZE) {
            printf("Error: LOAD: Invalid array size in file.\n");
            interpreter_halt();
            return;
        }
        memcpy(var->iarray.data, interpreter_public_buffer, buffer_size);
        var->iarray.size = size;
    }
    else if (interpreter_ci_strcmp(type, "farray") == 0 && var->type == TYPE_FARRAY) {
        int size = buffer_size / sizeof(float);
        if (size < 0 || size > INTERPRETER_MAX_ARRAY_SIZE) {
            printf("Error: LOAD: Invalid array size in file.\n");
            interpreter_halt();
            return;
        }
        memcpy(var->farray.data, interpreter_public_buffer, buffer_size);
        var->farray.size = size;
    }
    else if (interpreter_ci_strcmp(type, "string") == 0 && var->type == TYPE_STRING) {
        if (buffer_size > INTERPRETER_MAX_ARRAY_SIZE) {
            printf("Error: LOAD: String too large.\n");
            interpreter_halt();
            return;
        }
        memcpy(var->string.data, interpreter_public_buffer, buffer_size);
        var->string.size = buffer_size;
    }
    else {
        printf("Error: LOAD: Type mismatch or unsupported type.\n");
        interpreter_halt();
        return;
    }
}

void interpreter_execute_print(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 2) {
        printf("Error: PRINT instruction requires at least 2 tokens.\n");
        interpreter_halt();
        return;
    }

    const char* mode = tokens[1];

    if (interpreter_ci_strcmp(mode, "string") == 0) {
        if (token_count < 3) {
            printf("Error: PRINT string requires a variable name.\n");
            interpreter_halt();
            return;
        }
        Interpreter_Value* val = interpreter_get_variable(instance, tokens[2]);
        if (!val || val->type != TYPE_STRING) {
            printf("Error: Variable is not a string.\n");
            interpreter_halt();
            return;
        }
        for (int i = 0; i < val->string.size; i++) {
            app_runtime_print_char(val->string.data[i]);
        }
    }
    else if (interpreter_ci_strcmp(mode, "const") == 0) {
        char temp[INTERPRETER_MAX_ARRAY_SIZE];
        for (int i = 2; i < token_count; ++i) {
            interpreter_parse_const_escapes(tokens[i], temp, sizeof(temp));
            for (int j = 0; temp[j] != '\0'; j++) {
                app_runtime_print_char(temp[j]);
            }
            if (i < token_count - 1) {
                app_runtime_print_char(' ');
            }
        }
    }
    else if (interpreter_ci_strcmp(mode, "$") == 0) {
        if (token_count < 3) {
            printf("Error: PRINT $ requires a variable or value.\n");
            interpreter_halt();
            return;
        }
        Interpreter_Value val = interpreter_get_value_of_token(instance, tokens[2]);
        switch (val.type) {
            case TYPE_INT: {
                app_runtime_print_int(val.i);
                break;
            }
            case TYPE_FLOAT: {
                app_runtime_print_float(val.f);
                break;
            }
            case TYPE_BYTE: {
                app_runtime_print_int((int)val.b);
                break;
            }
            default: {
                printf("Error: Unsupported type for PRINT $.\n");
                interpreter_halt();
                return;
            }
        }
    }
    else if (interpreter_ci_strcmp(mode, "ascii") == 0) {
        if (token_count < 3) {
            printf("Error: PRINT ascii requires a variable or value.\n");
            interpreter_halt();
            return;
        }
        Interpreter_Value val = interpreter_get_value_of_token(instance, tokens[2]);
        int ch = 0;
        if (val.type == TYPE_INT) ch = val.i;
        else if (val.type == TYPE_BYTE) ch = val.b;
        else {
            printf("Error: PRINT ascii requires int or byte.\n");
            interpreter_halt();
            return;
        }
        app_runtime_print_char(ch);
    }
    else if (interpreter_ci_strcmp(mode, "serial") == 0) {
        if (token_count < 3) {
            printf("Error: PRINT serial requires a variable name.\n");
            interpreter_halt();
            return;
        }
        Interpreter_Value* val = interpreter_get_variable(instance, tokens[2]);
        if (!val || val->type != TYPE_STRING) {
            printf("Error: PRINT serial: Variable is not a string.\n");
            interpreter_halt();
            return;
        }
        com1_write(val->string.data, val->string.size);
    }
    else if (interpreter_ci_strcmp(mode, "usb") == 0) {
        if (token_count < 3) {
            printf("Error: PRINT usb requires a variable name.\n");
            interpreter_halt();
            return;
        }
        Interpreter_Value* val = interpreter_get_variable(instance, tokens[2]);
        if (!val || val->type != TYPE_STRING) {
            printf("Error: PRINT usb: Variable is not a string.\n");
            interpreter_halt();
            return;
        }
        usb_serial_write(val->string.data, val->string.size);
    }
    else {
        printf("Error: Unknown PRINT mode.\n");
        interpreter_halt();
        return;
    }
}

void interpreter_execute_println(Interpreter_Instance* instance, char** tokens, int token_count)
{
    interpreter_execute_print(instance, tokens, token_count);
    app_runtime_print_char('\n');
}

static unsigned int interpreter_rand_seed = 0;
static int interpreter_seeded = 0;

void interpreter_execute_random(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 3) {
        printf("RANDOM instruction requires variable and upper bound.\n");
        interpreter_halt();
        return;
    }

    const char* var_name = tokens[1];
    Interpreter_Value upper_val = interpreter_get_value_of_token(instance, tokens[2]);
    if (upper_val.type != TYPE_INT && upper_val.type != TYPE_BYTE) {
        printf("RANDOM upper bound must be integer or byte.\n");
        interpreter_halt();
        return;
    }

    int upper = (upper_val.type == TYPE_INT) ? upper_val.i : upper_val.b;
    if (upper <= 0) {
        printf("RANDOM upper bound must be > 0.\n");
        interpreter_halt();
        return;
    }

    if (!interpreter_seeded) {
        interpreter_rand_seed = system_uptime() & 0xFFFFFFFF;
        interpreter_seeded = 1;
    }

    interpreter_rand_seed = (1103515245 * interpreter_rand_seed + 12345);
    unsigned int random_value = (interpreter_rand_seed >> 16) & 0x7FFF;
    int result_int = random_value % upper;

    Interpreter_Value result;
    result.type = TYPE_INT;
    result.i = result_int;
    interpreter_set_variable(instance, var_name, result);
}

void interpreter_execute_return(Interpreter_Instance* instance, char** tokens, int token_count)
{
    Interpreter_Value return_value;
    memset(&return_value, 0, sizeof(Interpreter_Value));
    if (token_count > 1) {
        return_value = interpreter_get_value_of_token(instance, tokens[1]);
        if (return_value.type == -1) {
            printf("Error: Invalid return value.\n");
            interpreter_halt();
            return;
        }
    }

    if (instance->stack_pointer <= 0) {
        instance->stack_pointer = -1;
        instance->execution_position = -1;
    }
    else {
        instance->execution_position = instance->call_stack[instance->stack_pointer].return_address;
        instance->stack_pointer--;

        if (instance->stack_pointer >= 0) {
            Interpreter_CallFrame* caller_frame = &instance->call_stack[instance->stack_pointer];
            if (caller_frame->return_var != (void*)0) {
                interpreter_set_variable(instance, caller_frame->return_var, return_value);
            }
        }
    }
}

void interpreter_execute_sizeof(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 3) {
        printf("Error: SIZEOF instruction requires a destination variable and a source array/string.\n");
        interpreter_halt();
        return;
    }

    const char* dest_var_name = tokens[0];
    const char* src_var_name = tokens[2];

    Interpreter_Value* src_val = interpreter_get_variable(instance, src_var_name);
    if (!src_val) {
        printf("Error: Variable not found for SIZEOF.\n");
        interpreter_halt();
        return;
    }

    int size = 0;
    switch (src_val->type) {
        case TYPE_IARRAY:
            size = src_val->iarray.size;
            break;
        case TYPE_FARRAY:
            size = src_val->farray.size;
            break;
        case TYPE_STRING:
            size = src_val->string.size;
            break;
        default:
            printf("Error: SIZEOF only supports arrays and strings.\n");
            interpreter_halt();
            return;
    }

    Interpreter_Value result;
    memset(&result, 0, sizeof(Interpreter_Value));
    result.type = TYPE_INT;
    result.i = size;
    interpreter_set_variable(instance, dest_var_name, result);
}

void interpreter_execute_sleep(Interpreter_Instance* instance, char** tokens, int token_count)
{
    if (token_count < 2) {
        printf("Error: SLEEP instruction requires a millisecond constant.\n");
        interpreter_halt();
        return;
    }

    Interpreter_Value ms_val = interpreter_get_value_of_token(instance, tokens[1]);
    if (ms_val.type != TYPE_INT && ms_val.type != TYPE_BYTE) {
        printf("Error: SLEEP time must be an integer or byte.\n");
        interpreter_halt();
        return;
    }
    int ms = (ms_val.type == TYPE_INT) ? ms_val.i : ms_val.b;

    if (ms < 0) {
        printf("Error: SLEEP time must be non-negative.\n");
        interpreter_halt();
        return;
    }
    
    if (ms > 0) {
        unsigned long long int ticks_to_sleep = (unsigned long long int)ms;
        instance->sleep_until_tick = system_uptime() + ticks_to_sleep;
        instance->is_sleeping = 1;
    }
}

Interpreter_Instruction_AttributeMatcher declare_matchers[] = { {"@", 0} };
Interpreter_Instruction_AttributeMatcher assign_matchers[] = { {"=", 1} };
Interpreter_Instruction_AttributeMatcher add_matchers[] = { {"=", 1}, {"+", 3} };
Interpreter_Instruction_AttributeMatcher add_assign_matchers[] = { {"+=", 1} };
Interpreter_Instruction_AttributeMatcher array_get_matchers[] = { {"<-", 1}, {":", 3} };
Interpreter_Instruction_AttributeMatcher array_set_matchers[] = { {":", 1}, {"<-", 3} };
Interpreter_Instruction_AttributeMatcher call_matchers[] = { {"call", 0} };
Interpreter_Instruction_AttributeMatcher call_assign_matchers[] = { {"<-", 1}, {"call", 2} };
Interpreter_Instruction_AttributeMatcher cat_matchers[] = { {"cat", 0} };
Interpreter_Instruction_AttributeMatcher clear_matchers[] = { {"clear", 0} };
Interpreter_Instruction_AttributeMatcher divide_matchers[] = { {"=", 1}, {"/", 3} };
Interpreter_Instruction_AttributeMatcher divide_assign_matchers[] = { {"/=", 1} };
Interpreter_Instruction_AttributeMatcher end_matchers[] = { {"end", 0} };
Interpreter_Instruction_AttributeMatcher exec_matchers[] = { {"exec", 0} };
Interpreter_Instruction_AttributeMatcher free_matchers[] = { {"free", 0} };
Interpreter_Instruction_AttributeMatcher if_matchers[] = { {"if", 0} };
Interpreter_Instruction_AttributeMatcher input_matchers[] = { {"input", 0} };
Interpreter_Instruction_AttributeMatcher load_matchers[] = { {"load", 0} };
Interpreter_Instruction_AttributeMatcher mod_matchers[] = { {"=", 1}, {"%", 3} };
Interpreter_Instruction_AttributeMatcher mod_assign_matchers[] = { {"%=", 1} };
Interpreter_Instruction_AttributeMatcher multiply_matchers[] = { {"=", 1}, {"*", 3} };
Interpreter_Instruction_AttributeMatcher multiply_assign_matchers[] = { {"*=", 1} };
Interpreter_Instruction_AttributeMatcher print_matchers[] = { {"print", 0} };
Interpreter_Instruction_AttributeMatcher println_matchers[] = { {"println", 0} };
Interpreter_Instruction_AttributeMatcher random_matchers[] = { {"random", 0} };
Interpreter_Instruction_AttributeMatcher return_matchers[] = { {"return", 0} };
Interpreter_Instruction_AttributeMatcher save_matchers[] = { {"save", 0} };
Interpreter_Instruction_AttributeMatcher sizeof_matchers[] = { {"sizeof", 1} };
Interpreter_Instruction_AttributeMatcher sleep_matchers[] = { {"sleep", 0} };
Interpreter_Instruction_AttributeMatcher subtract_matchers[] = { {"=", 1}, {"-", 3} };
Interpreter_Instruction_AttributeMatcher subtract_assign_matchers[] = { {"-=", 1} };
Interpreter_Instruction_AttributeMatcher while_matchers[] = { {"while", 0} };

const Interpreter_Instruction_Definition g_instruction_definitions[] = {
    {"ADD", add_matchers, sizeof(add_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 5, interpreter_execute_add},
    {"ARRAY_GET", array_get_matchers, sizeof(array_get_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 5, interpreter_execute_array_get},
    {"ARRAY_SET", array_set_matchers, sizeof(array_set_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 5, interpreter_execute_array_set},
    {"CALL_ASSIGN", call_assign_matchers, sizeof(call_assign_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 4, interpreter_execute_call_assign},
    {"DIVIDE", divide_matchers, sizeof(divide_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 5, interpreter_execute_divide},
    {"MOD", mod_matchers, sizeof(mod_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 5, interpreter_execute_mod},
    {"MULTIPLY", multiply_matchers, sizeof(multiply_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 5, interpreter_execute_multiply},
    {"SUBTRACT", subtract_matchers, sizeof(subtract_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 5, interpreter_execute_subtract},
    {"DECLARE", declare_matchers, sizeof(declare_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 3, interpreter_execute_declare},
    {"ASSIGN", assign_matchers, sizeof(assign_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 3, interpreter_execute_assign},
    {"ADD_ASSIGN", add_assign_matchers, sizeof(add_assign_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 3, interpreter_execute_add_assign},
    {"CALL", call_matchers, sizeof(call_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 2, interpreter_execute_call},
    {"CAT", cat_matchers, sizeof(cat_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 4, interpreter_execute_cat},
    {"CLEAR", clear_matchers, sizeof(clear_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 1, interpreter_execute_clear},
    {"DIVIDE_ASSIGN", divide_assign_matchers, sizeof(divide_assign_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 3, interpreter_execute_divide_assign},
    {"END", end_matchers, sizeof(end_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 1, interpreter_execute_end},
    {"EXEC", exec_matchers, sizeof(exec_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 3, interpreter_execute_exec},
    {"FREE", free_matchers, sizeof(free_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 2, interpreter_execute_free},
    {"IF", if_matchers, sizeof(if_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 4, interpreter_execute_if},
    {"INPUT", input_matchers, sizeof(input_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 3, interpreter_execute_input},
    {"LOAD", load_matchers, sizeof(load_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 4, interpreter_execute_load},
    {"MOD_ASSIGN", mod_assign_matchers, sizeof(mod_assign_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 3, interpreter_execute_mod_assign},
    {"MULTIPLY_ASSIGN", multiply_assign_matchers, sizeof(multiply_assign_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 3, interpreter_execute_multiply_assign},
    {"PRINT", print_matchers, sizeof(print_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 2, interpreter_execute_print},
    {"PRINTLN", println_matchers, sizeof(println_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 2, interpreter_execute_println},
    {"RANDOM", random_matchers, sizeof(random_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 3, interpreter_execute_random},
    {"RETURN", return_matchers, sizeof(return_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 1, interpreter_execute_return},
    {"SAVE", save_matchers, sizeof(save_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 4, interpreter_execute_save},
    {"SIZEOF", sizeof_matchers, sizeof(sizeof_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 3, interpreter_execute_sizeof},
    {"SLEEP", sleep_matchers, sizeof(sleep_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 2, interpreter_execute_sleep},
    {"SUBTRACT_ASSIGN", subtract_assign_matchers, sizeof(subtract_assign_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 3, interpreter_execute_subtract_assign},
    {"WHILE", while_matchers, sizeof(while_matchers)/sizeof(Interpreter_Instruction_AttributeMatcher), 4, interpreter_execute_while}
};
const int g_instruction_definition_count = sizeof(g_instruction_definitions) / sizeof(Interpreter_Instruction_Definition);
