#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <interpreter/instance.h>
#include <interpreter/config.h>

void interpreter_instance_init(Interpreter_Instance* instance);
int interpreter_load_code(Interpreter_Instance* instance, const char* code);
int interpreter_execute(Interpreter_Instance* instance);
void interpreter_dump_tokens(Interpreter_Instance* instance);
int interpreter_parse_functions(Interpreter_Instance* instance);
void interpreter_dump_functions(Interpreter_Instance* instance);
void interpreter_halt();
void interpreter_parse_const_escapes(const char* src, char* dest, unsigned long long int max_len);

int interpreter_call_function(Interpreter_Instance* instance, const char* function_name, char** arg_names, int arg_count);
void interpreter_declare_variable(Interpreter_Instance* instance, const char* name, Interpreter_VarType type);
Interpreter_Value* interpreter_get_variable(Interpreter_Instance* instance, const char* name);
Interpreter_Value interpreter_get_value_of_token(Interpreter_Instance* instance, const char* token);
Interpreter_VarType interpreter_get_type(const char* type_str);
void interpreter_set_variable(Interpreter_Instance* instance, const char* name, Interpreter_Value value);
void interpreter_set_reference_parameter(Interpreter_Instance* instance, const char* param_name, const char* var_name);
int interpreter_ci_strcmp(const char* str1, const char* str2);
int interpreter_compare(Interpreter_Value left, Interpreter_Value right, const char* op);
int interpreter_find_matching_end(Interpreter_Instance* instance, int start_line);

extern char interpreter_code_path[INTERPRETER_PATH_MAX];

#endif
