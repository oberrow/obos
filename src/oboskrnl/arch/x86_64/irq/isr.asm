; oboskrnl/arch/x86_64/isr.asm
;
; Copyright (c) 2024 Omar Berrow

section .text

align 32
global __b_isr_handler
global __e_isr_handler
global idtFlush
__b_isr_handler:
%macro isr_handler_no_ec 1
align 32
isr%1:
	cld
	push 0
	push %1
	jmp int_handler_common
%endmacro
%macro isr_handler_ec 1
align 0x20
isr%1:
	cld
	push %1
	jmp int_handler_common
%endmacro
isr_handler_no_ec  0
isr_handler_no_ec  1
isr_handler_no_ec  2
isr_handler_no_ec  3
isr_handler_no_ec  4
isr_handler_no_ec  5
isr_handler_no_ec  6
isr_handler_no_ec  7
isr_handler_ec     8
isr_handler_no_ec  9
isr_handler_ec    10
isr_handler_ec    11
isr_handler_ec    12
isr_handler_ec    13
isr_handler_ec    14
isr_handler_no_ec 15
isr_handler_no_ec 16
isr_handler_ec    17
isr_handler_no_ec 18
isr_handler_no_ec 19
isr_handler_no_ec 20
isr_handler_ec    21
isr_handler_no_ec 22
isr_handler_no_ec 23
isr_handler_no_ec 24
isr_handler_no_ec 25
isr_handler_no_ec 26
isr_handler_no_ec 27
isr_handler_no_ec 28
isr_handler_no_ec 29
isr_handler_no_ec 30
isr_handler_no_ec 31
%assign current_int_number 32
%rep 224
isr_handler_no_ec current_int_number
%assign current_int_number current_int_number+1
%endrep
__e_isr_handler:

%macro pushaq 0
push rax
; rax has rsp.
mov rax, rsp
add rax, 8
push rcx
push rdx
push rbx
push rax ; Push rsp
push rsi
push rdi
push r8
push r9
push r10
push r11
push r12
push r13
push r14
push r15
push qword [rsp+0x88]
push rbp
%endmacro

; Cleans up after pushaq.

%macro popaq 0
pop rbp
add rsp, 8
pop r15
pop r14
pop r13
pop r12
pop r11
pop r10
pop r9
pop r8
pop rdi
pop rsi
add rsp, 8
pop rbx
pop rdx
pop rcx
pop rax
%endmacro

extern _ZN4obos10g_handlersE

int_handler_common:
	pushaq
	mov rbp, rsp

	mov rax, ds
	push rax

	mov rax, [rsp+0x90]
	cmp rax, 255
	ja .finished
	mov rax, [_ZN4obos10g_handlersE+rax*8]
	
	swapgs

	test rax,rax
	jz .finished

	mov rdi, rsp
	call rax

.finished:
	swapgs
	mov rsp, rbp
	popaq

	add rsp, 16

	iretq

idtFlush:
	lidt [rdi]
	ret
global _ZN4obos4arch7SetIRQLEh
global _ZN4obos4arch7GetIRQLEv
_ZN4obos4arch7GetIRQLEv:
	push rbp
	mov rbp, rsp
	
	mov rax, cr8
	
	leave
	ret
_ZN4obos4arch7SetIRQLEh:
	push rbp
	mov rbp, rsp
	
	mov cr8, rdi
	
	leave
	ret
global disablePIC
disablePIC:
	mov al, 0xff
	out 0x21, al
	out 0xA1, al
	ret