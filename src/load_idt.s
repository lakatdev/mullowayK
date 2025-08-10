.global exception0
.global exception1
.global exception2
.global exception3
.global exception4
.global exception5
.global exception6
.global exception7
.global exception8
.global exception10
.global exception11
.global exception12
.global exception13
.global exception14
.global exception16
.global exception17
.global exception18
.global exception19

.global irq0
.global irq1
.global irq2
.global irq3
.global irq4
.global irq5
.global irq6
.global irq7
.global irq8
.global irq9
.global irq10
.global irq11
.global irq12
.global irq13
.global irq14
.global irq15

.global load_idt

.extern exception_divide_error
.extern exception_debug
.extern exception_nmi
.extern exception_breakpoint
.extern exception_overflow
.extern exception_bound_range
.extern exception_invalid_opcode
.extern exception_device_not_available
.extern exception_double_fault
.extern exception_invalid_tss
.extern exception_segment_not_present
.extern exception_stack_segment_fault
.extern exception_general_protection
.extern exception_page_fault
.extern exception_fpu_error
.extern exception_alignment_check
.extern exception_machine_check
.extern exception_simd_exception

.extern irq0_handler
.extern irq1_handler
.extern irq2_handler
.extern irq3_handler
.extern irq4_handler
.extern irq5_handler
.extern irq6_handler
.extern irq7_handler
.extern irq8_handler
.extern irq9_handler
.extern irq10_handler
.extern irq11_handler
.extern irq12_handler
.extern irq13_handler
.extern irq14_handler
.extern irq15_handler

// Exception stubs
exception0:
    pusha
    call exception_divide_error
    popa
    iret

exception1:
    pusha
    call exception_debug
    popa
    iret

exception2:
    pusha
    call exception_nmi
    popa
    iret

exception3:
    pusha
    call exception_breakpoint
    popa
    iret

exception4:
    pusha
    call exception_overflow
    popa
    iret

exception5:
    pusha
    call exception_bound_range
    popa
    iret

exception6:
    pusha
    call exception_invalid_opcode
    popa
    iret

exception7:
    pusha
    call exception_device_not_available
    popa
    iret

exception8:
    pusha
    call exception_double_fault
    popa
    iret

exception10:
    pusha
    call exception_invalid_tss
    popa
    iret

exception11:
    pusha
    call exception_segment_not_present
    popa
    iret

exception12:
    pusha
    call exception_stack_segment_fault
    popa
    iret

exception13:
    pusha
    call exception_general_protection
    popa
    iret

exception14:
    pusha
    call exception_page_fault
    popa
    iret

exception16:
    pusha
    call exception_fpu_error
    popa
    iret

exception17:
    pusha
    call exception_alignment_check
    popa
    iret

exception18:
    pusha
    call exception_machine_check
    popa
    iret

exception19:
    pusha
    call exception_simd_exception
    popa
    iret

irq0:
    pusha
    call irq0_handler
    popa
    iret

irq1:
    pusha
    call irq1_handler
    popa
    iret

irq2:
    pusha
    call irq2_handler
    popa
    iret

irq3:
    pusha
    call irq3_handler
    popa
    iret

irq4:
    pusha
    call irq4_handler
    popa
    iret

irq5:
    pusha
    call irq5_handler
    popa
    iret

irq6:
    pusha
    call irq6_handler
    popa
    iret

irq7:
    pusha
    call irq7_handler
    popa
    iret

irq8:
    pusha
    call irq8_handler
    popa
    iret

irq9:
    pusha
    call irq9_handler
    popa
    iret

irq10:
    pusha
    call irq10_handler
    popa
    iret

irq11:
    pusha
    call irq11_handler
    popa
    iret

irq12:
    pusha
    call irq12_handler
    popa
    iret

irq13:
    pusha
    call irq13_handler
    popa
    iret

irq14:
    pusha
    call irq14_handler
    popa
    iret

irq15:
    pusha
    call irq15_handler
    popa
    iret

load_idt:
    mov 4(%esp), %edx
    lidt (%edx)
    sti
    ret
