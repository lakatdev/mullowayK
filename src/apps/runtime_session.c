#include <apps/runtime_session.h>
#include <userlib.h>
#include <rtc.h>
#include <storage.h>
#include <interface.h>
#include <memory.h>
#include <desktop.h>

static RuntimeSession* session_slots[MAX_RUNTIME_SESSIONS] = {0};
static int session_pool_initialized = 0;

static unsigned char* session_memory_base = (void*)0;
static unsigned int session_memory_offset = 0;
static unsigned int session_memory_size = 0;

static RuntimeSession* allocate_session_memory()
{
    if (session_memory_offset + sizeof(RuntimeSession) > session_memory_size) {
        printf("Runtime: Session memory exhausted!\n");
        return (void*)0;
    }
    
    unsigned int allocation_address = (unsigned int)session_memory_base + session_memory_offset;
    unsigned int allocation_end = allocation_address + sizeof(RuntimeSession);
    unsigned int available_mb = get_memory_mb();
    unsigned int available_bytes = available_mb * 1024 * 1024;
    
    if (allocation_end > available_bytes) {
        printf("Runtime: Allocation would exceed available memory\n");
        return (void*)0;
    }
    
    RuntimeSession* session = (RuntimeSession*)(session_memory_base + session_memory_offset);
    session_memory_offset += sizeof(RuntimeSession);
    
    return session;
}

static void free_session_memory(RuntimeSession* session)
{
    if (session) {
        memset(session, 0, sizeof(RuntimeSession));
    }
}

extern unsigned int _kernel_end;

void runtime_session_pool_init()
{
    if (session_pool_initialized) return;
    
    for (int i = 0; i < MAX_RUNTIME_SESSIONS; i++) {
        session_slots[i] = (void*)0;
    }
    
    unsigned int memory_mb = get_memory_mb();
    unsigned int available_bytes = memory_mb * 1024 * 1024;
    
    session_memory_size = sizeof(RuntimeSession) * MAX_RUNTIME_SESSIONS;
    
    unsigned int base_offset = (available_bytes * 25) / 100;
    
    unsigned int kernel_end_addr = (unsigned int)&_kernel_end;
    unsigned int safe_start = kernel_end_addr + (1024 * 1024); // 1MB padding
    print_hex(kernel_end_addr);

    if (base_offset < safe_start) {
        base_offset = safe_start;
    }

    if (base_offset < 20 * 1024 * 1024) {
        base_offset = 20 * 1024 * 1024;
    }
    
    if (base_offset + session_memory_size + (5 * 1024 * 1024) > available_bytes) {
        if (session_memory_size + (5 * 1024 * 1024) < available_bytes) {
            unsigned int alternative = available_bytes - session_memory_size - (5 * 1024 * 1024);
            if (alternative > safe_start) {
                base_offset = alternative;
            }
        }
    }
    
    session_memory_base = (unsigned char*)base_offset;
    session_memory_offset = 0;
    
    session_pool_initialized = 1;
}

void runtime_session_init(RuntimeSession* session)
{
    session->instance_stack_top = -1;
    session->loaded = 0;
    session->error_code = 0;
    session->execution_requested = 0;
    session->executing = 0;
    session->input_requested = 0;
    session->cursor_x = 0;
    session->input_buffer[0] = '\0';
    session->input_buffer_length = 0;
    for (int i = 0; i < APP_RUNTIME_WIDTH * APP_RUNTIME_HEIGHT; i++) {
        session->video[i] = 0;
    }
    session->code[0] = '\0';
}

RuntimeSession* runtime_session_allocate(int window_id)
{
    runtime_session_pool_init();
    
    for (int i = 0; i < MAX_RUNTIME_SESSIONS; i++) {
        if (session_slots[i] && session_slots[i]->active && session_slots[i]->window_id == window_id) {
            return session_slots[i];
        }
    }
    
    for (int i = 0; i < MAX_RUNTIME_SESSIONS; i++) {
        if (!session_slots[i] || !session_slots[i]->active) {
            if (!session_slots[i]) {
                session_slots[i] = allocate_session_memory();
                if (!session_slots[i]) {
                    printf("OUT OF MEMORY: Cannot create new runtime instance\n");
                    return (void*)0;
                }
                session_slots[i]->active = 0;
                session_slots[i]->loaded = 0;
                session_slots[i]->executing = 0;
                session_slots[i]->instance_stack_top = -1;
            }
            
            session_slots[i]->active = 1;
            session_slots[i]->window_id = window_id;
            runtime_session_init(session_slots[i]);
            return session_slots[i];
        }
    }
    
    printf("OUT OF MEMORY: Maximum runtime instances reached\n");
    return (void*)0;
}

void runtime_session_deallocate(RuntimeSession* session)
{
    if (!session) return;
    
    runtime_session_stop_execute(session);
    runtime_session_init(session);
    session->active = 0;
    session->window_id = -1;
}

void runtime_session_cleanup(RuntimeSession* session)
{
    runtime_session_deallocate(session);
}

RuntimeSession* runtime_session_get_by_window(int window_id)
{
    runtime_session_pool_init();
    
    if (window_id < 0) {
        return (void*)0;
    }
    
    for (int i = 0; i < MAX_RUNTIME_SESSIONS; i++) {
        if (session_slots[i] && session_slots[i]->active && session_slots[i]->window_id == window_id) {
            return session_slots[i];
        }
    }
    return (void*)0;
}

Interpreter_Instance* runtime_session_get_current_instance(RuntimeSession* session)
{
    if (!session || session->instance_stack_top < 0) {
        return (void*)0;
    }
    return &session->instances[session->instance_stack_top];
}

int runtime_session_get_active_count(void)
{
    int count = 0;
    for (int i = 0; i < MAX_RUNTIME_SESSIONS; i++) {
        if (session_slots[i] && session_slots[i]->active) {
            count++;
        }
    }
    return count;
}

int runtime_session_get_max_count(void)
{
    unsigned int memory_mb = get_memory_mb();
    unsigned int available_bytes = memory_mb * 1024 * 1024;
    unsigned int session_size = sizeof(RuntimeSession);
    
    unsigned int base_offset = (available_bytes * 25) / 100;
    if (base_offset < 20 * 1024 * 1024) {
        base_offset = 20 * 1024 * 1024;
    }
    
    unsigned int available_for_sessions = 0;
    if (available_bytes > base_offset + (5 * 1024 * 1024)) {
        available_for_sessions = available_bytes - base_offset - (5 * 1024 * 1024);
    }
    
    int max_instances = available_for_sessions / session_size;
    
    if (max_instances > MAX_RUNTIME_SESSIONS) {
        max_instances = MAX_RUNTIME_SESSIONS;
    }
    
    return max_instances;
}

void runtime_session_scroll_video(RuntimeSession* session)
{
    for (int i = 0; i < (APP_RUNTIME_HEIGHT - 1) * APP_RUNTIME_WIDTH; i++) {
        session->video[i] = session->video[i + APP_RUNTIME_WIDTH];
    }
    for (int i = (APP_RUNTIME_HEIGHT - 1) * APP_RUNTIME_WIDTH; i < APP_RUNTIME_HEIGHT * APP_RUNTIME_WIDTH; i++) {
        session->video[i] = 0;
    }
}

void runtime_session_print_char(RuntimeSession* session, char c)
{
    if (!session) return;
    
    switch (c) {
        case '\n': {
            session->cursor_x = 0;
            runtime_session_scroll_video(session);
            break;
        }
        case '\t': {
            session->cursor_x = (session->cursor_x + 4) & ~3; 
            break;
        }
        default: {
            if (c >= 32 && c <= 126) { 
                if (session->cursor_x < APP_RUNTIME_WIDTH) {
                    session->video[(APP_RUNTIME_HEIGHT - 1) * APP_RUNTIME_WIDTH + session->cursor_x] = c;
                    session->cursor_x++;
                }
                else {
                    session->cursor_x = 0;
                    runtime_session_scroll_video(session);
                    session->video[(APP_RUNTIME_HEIGHT - 1) * APP_RUNTIME_WIDTH + session->cursor_x] = c;
                }
            }
            break;
        }
    }
    invalidate();
}

void runtime_session_print(RuntimeSession* session, const char* str)
{
    if (!session) return;
    for (int i = 0; str[i] != '\0'; i++) {
        runtime_session_print_char(session, str[i]);
    }
    invalidate();
}

void runtime_session_print_int(RuntimeSession* session, int value)
{
    if (!session) return;
    
    char buffer[16]; 
    int i = 0;
    int is_negative = 0;
    
    if (value < 0) {
        is_negative = 1;
        value = -value;
    }
    
    if (value == 0) {
        runtime_session_print_char(session, '0');
        return;
    }
    
    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    if (is_negative) {
        runtime_session_print_char(session, '-');
    }
    
    for (int j = i - 1; j >= 0; j--) {
        runtime_session_print_char(session, buffer[j]);
    }
    invalidate();
}

void runtime_session_print_float(RuntimeSession* session, float value)
{
    if (!session) return;
    
    if (value != value) {
        runtime_session_print_char(session, 'N');
        runtime_session_print_char(session, 'a');
        runtime_session_print_char(session, 'N');
        return;
    }
    
    if (value < 0) {
        runtime_session_print_char(session, '-');
        value = -value;
    }
    
    if (value > 3.4028235e+38f) {
        runtime_session_print_char(session, 'I');
        runtime_session_print_char(session, 'n');
        runtime_session_print_char(session, 'f');
        return;
    }
    
    int int_part = (int)value;
    float frac_part = value - int_part;
    runtime_session_print_int(session, int_part);
    runtime_session_print_char(session, '.');
    
    for (int i = 0; i < 6; i++) {
        frac_part *= 10;
        int digit = (int)frac_part;
        runtime_session_print_char(session, '0' + digit);
        frac_part -= digit;
        
        if (frac_part < 0.0000001f) {
            break;
        }
    }
    invalidate();
}

void runtime_session_clear_buffer(RuntimeSession* session)
{
    if (!session) return;
    memset(session->video, 0, sizeof(session->video));
    session->cursor_x = 0;
}

void runtime_session_load_code(RuntimeSession* session, const char* code)
{
    if (!session) return;
    
    session->loaded = 0;
    strcpy(session->code, code);
    session->loaded = 1;
    session->load_year = get_year();
    session->load_month = get_month();
    session->load_day = get_day();
    session->load_hour = get_hour();
    session->load_minute = get_minute();
    session->load_second = get_second();
}

void runtime_session_stop_execute(RuntimeSession* session)
{
    if (!session) return;
    
    session->execution_requested = 0;
    Interpreter_Instance* current = runtime_session_get_current_instance(session);
    if (session->executing && current && current->is_running) {
        interpreter_stop(current);
    }
    session->executing = 0;
    session->instance_stack_top = -1;
    session->input_requested = 0;
    session->input_buffer[0] = '\0';
    session->input_buffer_length = 0;
}

void runtime_session_clear_code(RuntimeSession* session)
{
    if (!session) return;
    
    runtime_session_stop_execute(session);
    session->loaded = 0;
    session->error_code = 0;
    session->execution_requested = 0;
    session->executing = 0;
    memset(session->video, 0, sizeof(session->video));
    session->cursor_x = 0;
}

void runtime_session_execute(RuntimeSession* session)
{
    if (!session) return;
    
    session->error_code = 0;
    memset(session->video, 0, sizeof(session->video));
    session->cursor_x = 0;

    if (!session->loaded) {
        printf("Runtime: No code loaded to execute.\n");
        return;
    }
    
    session->instance_stack_top = 0;
    interpreter_instance_init(&session->instances[0]);
    
    if (interpreter_load_code(&session->instances[0], session->code) != 0) {
        printf("Runtime: Failed to load code.\n");
        session->error_code = 1;
        session->executing = 0;
        session->instance_stack_top = -1;
        return;
    }
    if (interpreter_parse_functions(&session->instances[0]) != 0) {
        printf("Runtime: Failed to parse functions.\n");
        session->error_code = 1;
        session->executing = 0;
        session->instance_stack_top = -1;
        return;
    }
    
    int result = interpreter_execute(&session->instances[0]);
    if (result == 0) {
        session->executing = 0;
        session->instance_stack_top = -1;
    }
    else if (result > 0) {
        session->executing = 1;
    }
    else if (result < 0) {
        printf("Runtime: Execution failed.\n");
        session->error_code = 1;
        session->executing = 0;
        session->instance_stack_top = -1;
    }
}

void runtime_session_continue_execution(RuntimeSession* session)
{
    if (!session) return;
    
    Interpreter_Instance* current = runtime_session_get_current_instance(session);
    if (session->executing && current && current->is_running) {
        int result = interpreter_execute_chunk(current, 32);
        if (result == 0) {
            session->instance_stack_top--;
            if (session->instance_stack_top < 0) {
                session->executing = 0;
            }
        }
        else if (result < 0) {
            printf("Runtime: Execution failed.\n");
            session->error_code = 1;
            session->executing = 0;
            session->instance_stack_top = -1;
        }
    }
}

void runtime_session_request_execute(RuntimeSession* session)
{
    if (!session) return;
    if (!session->executing) {
        session->execution_requested = 1;
    }
}

void runtime_session_process(RuntimeSession* session)
{
    if (!session) return;
    
    if (session->execution_requested && !session->executing) {
        session->execution_requested = 0;
        session->executing = 1;
        runtime_session_execute(session);
    }
    else if (session->executing) {
        runtime_session_continue_execution(session);
    }
}

void runtime_session_request_input(RuntimeSession* session)
{
    if (!session) return;
    session->input_requested = 1;
    session->input_buffer[0] = '\0';
    session->input_buffer_length = 0;
}

void runtime_session_send_io(RuntimeSession* session)
{
    if (!session) return;
    
    if (session->input_requested && session->instance_stack_top >= 0) {
        Interpreter_Instance* current = runtime_session_get_current_instance(session);
        if (current && current->waiting_for_input) {
            strncpy(current->input_buffer, session->input_buffer, sizeof(current->input_buffer) - 1);
            current->input_buffer[sizeof(current->input_buffer) - 1] = '\0';
            current->input_ready = 1;
            
            runtime_session_print(session, session->input_buffer);
            runtime_session_print_char(session, '\n');
            session->input_requested = 0;
            session->input_buffer[0] = '\0';
            session->input_buffer_length = 0;
        }
    }
}

int runtime_session_push_instance_from_file(RuntimeSession* session, const char* filename)
{
    if (!session) return -1;
    
    if (session->instance_stack_top >= 1) {
        printf("Error: EXEC: Maximum instance depth reached.\n");
        return -1;
    }
    
    if (!files_exists(filename)) {
        printf("Error: EXEC: File not found.\n");
        return -1;
    }
    
    unsigned int file_size = 0;
    
    read_from_storage(filename, interpreter_public_buffer, &file_size);
    
    if (file_size == 0) {
        printf("Error: EXEC: File is empty.\n");
        return -1;
    }
    
    if (file_size >= STORAGE_RECORD_SIZE) {
        printf("Error: EXEC: File size invalid.\n");
        return -1;
    }
    
    interpreter_public_buffer[file_size] = '\0';
    
    session->instance_stack_top++;
    Interpreter_Instance* new_instance = &session->instances[session->instance_stack_top];
    
    interpreter_instance_init(new_instance);
    
    if (interpreter_load_code(new_instance, interpreter_public_buffer) != 0) {
        printf("Error: EXEC: Failed to load code.\n");
        session->instance_stack_top--;
        return -1;
    }
    
    if (interpreter_parse_functions(new_instance) != 0) {
        printf("Error: EXEC: Failed to parse functions.\n");
        session->instance_stack_top--;
        return -1;
    }
    
    int result = interpreter_execute(new_instance);
    if (result < 0) {
        printf("Error: EXEC: Execution failed.\n");
        session->instance_stack_top--;
        return -1;
    }
    
    if (result == 0) {
        session->instance_stack_top--;
    }
    
    return 0;
}
