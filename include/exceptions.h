#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

void crash(const char* message, int code);
void exception_divide_error();
void exception_debug();
void exception_nmi();
void exception_breakpoint();
void exception_overflow();
void exception_bound_range();
void exception_invalid_opcode();
void exception_device_not_available();
void exception_double_fault();
void exception_invalid_tss();
void exception_segment_not_present();
void exception_stack_segment_fault();
void exception_general_protection();
void exception_page_fault();
void exception_fpu_error();
void exception_alignment_check();
void exception_machine_check();
void exception_simd_exception();

#endif
