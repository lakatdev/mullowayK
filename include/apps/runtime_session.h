#ifndef APP_RUNTIME_SESSION_H
#define APP_RUNTIME_SESSION_H

#include <interpreter/interpreter.h>
#include <interpreter/config.h>

#define APP_RUNTIME_WIDTH 80
#define APP_RUNTIME_HEIGHT 25
#define MAX_RUNTIME_SESSIONS 9

typedef struct RuntimeSession {
    Interpreter_Instance instances[2];
    int instance_stack_top;
    
    int active;
    int loaded;
    int error_code;
    int execution_requested;
    int executing;
    int input_requested;
    
    char code[INTERPRETER_MAX_CODE];
    char video[APP_RUNTIME_WIDTH * APP_RUNTIME_HEIGHT];
    int cursor_x;
    
    char input_buffer[INTERPRETER_MAX_INPUT_LENGTH];
    int input_buffer_length;
    
    char load_year;
    char load_month;
    char load_day;
    char load_hour;
    char load_minute;
    char load_second;
    
    int window_id;
} RuntimeSession;

void runtime_session_init(RuntimeSession* session);
void runtime_session_cleanup(RuntimeSession* session);
RuntimeSession* runtime_session_allocate(int window_id);
void runtime_session_deallocate(RuntimeSession* session);
RuntimeSession* runtime_session_get_by_window(int window_id);

void runtime_session_load_code(RuntimeSession* session, const char* code);
void runtime_session_clear_code(RuntimeSession* session);
void runtime_session_execute(RuntimeSession* session);
void runtime_session_stop_execute(RuntimeSession* session);
void runtime_session_continue_execution(RuntimeSession* session);
void runtime_session_request_execute(RuntimeSession* session);
void runtime_session_process(RuntimeSession* session);

void runtime_session_print_char(RuntimeSession* session, char c);
void runtime_session_print(RuntimeSession* session, const char* str);
void runtime_session_print_int(RuntimeSession* session, int value);
void runtime_session_print_float(RuntimeSession* session, float value);
void runtime_session_clear_buffer(RuntimeSession* session);
void runtime_session_request_input(RuntimeSession* session);
void runtime_session_send_io(RuntimeSession* session);
int runtime_session_push_instance_from_file(RuntimeSession* session, const char* filename);

Interpreter_Instance* runtime_session_get_current_instance(RuntimeSession* session);
int runtime_session_get_active_count(void);
int runtime_session_get_max_count(void);

#endif
