; oboskrnl/arch/x86_64/thread_ctx.asm
;
; Copyright (c) 2024 Omar Berrow

[BITS 64]

; obos_status CoreS_SetupThreadContext(thread_ctx* ctx, uintptr_t entry, uintptr_t arg1, bool makeUserMode, void* stackBase, size_t stackSize);
; OBOS_NORETURN void CoreS_SwitchToThreadContext(const thread_ctx* ctx);
; obos_status CoreS_FreeThreadContext(thread_ctx* ctx);
; uintptr_t CoreS_CallFunctionOnStack(uintptr_t(*func)(uintptr_t), uintptr_t userdata);
; 
; void CoreS_SetThreadIRQL(thread_ctx* ctx, irql newIRQL);
; irql CoreS_GetThreadIRQL(const thread_ctx* ctx);

global CoreS_SetupThreadContext
global CoreS_SwitchToThreadContext
global CoreS_FreeThreadContext
global CoreS_CallFunctionOnStack
global CoreS_SetThreadIRQL
global CoreS_GetThreadIRQL

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

struc thread_ctx
align 8
.extended_ctx_ptr: resq 1
.cr3: resq 1
.irql: resb 1
.gs_base: resq 1
.fs_base: resq 1
.frame: resq 0x1a
endstruc
%endmacro

CoreS_SwitchToThreadContext:
	; Disable interrupts, getting an interrupt in the middle of execution of this function can be deadly.
	cli
	mov rbx, [rdi]
	cmp rbx, 0
	je .no_xstate
	; Restore the extended state.
	xor rcx,rcx
	xgetbv
	xrstor [rbx]
.no_xstate:
	add rdi, 8
	; Restore CR3 (address space)
	mov rax, [rdi]
	mov cr3, rax
	add rdi, 8
	; Restore IRQL.
	mov rax, [rdi]
	mov cr8, rax
	add rdi, 8
	; Restore GS_BASE
	mov eax, [rdi]
	mov edx, [rdi+4]
	mov ecx, 0xC0000101
	add rdi, 8
	; Restore FS_BASE
	mov eax, [rdi]
	mov edx, [rdi+4]
	mov ecx, 0xC0000100
	wrmsr
	; Restore thread GPRs.
	mov rsp, rdi
	add rsp, 8 ; Skip the saved DS
	popaq
	iretq
CoreS_FreeThreadContext:
	push rbp
	mov rbp, rsp
	; TODO: Implement.

	leave
	ret
CoreS_SetupThreadContext:
	push rbp
	mov rbp, rsp

	;             rdi,             rsi,            rdx,               rcx,              r8,               r9
	; thread_ctx* ctx, uintptr_t entry, uintptr_t arg1, bool makeUserMode, void* stackBase, size_t stackSize

	cmp rcx, 0
	jne .L1
	mov rax, 3 ; OBOS_STATUS_UNIMPLEMENTED
	jmp .finish

.L1:
	; Setup the registers.
	mov [rdi+thread_ctx.frame+0xA8], rsi       ; ctx->frame.rip
	mov [rdi+thread_ctx.frame+0x58], rdx       ; ctx->frame.rdi
	mov qword [rdi+thread_ctx.frame+0xB0], 0x8       ; ctx->frame.cs
	mov qword [rdi+thread_ctx.frame+0xC8], 0x10      ; ctx->frame.ss
	mov qword [rdi+thread_ctx.frame+0xB8], 0x200202  ; ctx->frame.rflags
	lea rax, [r8+r9]
	mov qword [rdi+thread_ctx.frame+0xC0], rax       ; ctx->frame.rsp
	; TODO: Add user space transition possible.

	; Setup the IRQL.
	mov byte [rdi+thread_ctx.irql], 0 ; Unmasked
	; Setup the page map.
	mov rax, cr3
	mov qword [rdi+thread_ctx.cr3], rax
	; Setup GS_BASE.
	mov rcx, 0xC0000101 ; GS.Base
	rdmsr
	shl rdx, 32
	or rax, rdx
	mov qword [rdi+thread_ctx.gs_base], rax

	xor rax, rax ; OBOS_STATUS_SUCCESS
.finish:
	leave
	ret
extern Arch_InitialISTStack
CoreS_CallFunctionOnStack:
	push rbp
	mov rbp, rsp

	; TODO: Make thread-safe
	lea rsp, [Arch_InitialISTStack+0x10000]
	xchg rdi, rsi
	call rsi

	leave
	ret
CoreS_SetThreadIRQL:
	push rbp
	mov rbp, rsp

	lea rax, [rdi+thread_ctx.irql]
	mov [rax], sil

	leave
	ret
CoreS_GetThreadIRQL:
	push rbp
	mov rbp, rsp

	mov rax, [rdi+thread_ctx.irql]

	leave
	ret