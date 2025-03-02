[bits 32]
section .text
global switch_to
switch_to:
   ; The return address is located here on the stack.
   push esi
   push edi
   push ebx
   push ebp

   mov eax, [esp + 20]         ; Get the parameter `cur` from the stack, cur = [esp + 20].
   mov [eax], esp              ; Save the stack pointer (esp) into the `self_kstack` field of the task_struct.
                               ; The `self_kstack` field is at offset 0 in the task_struct,
                               ; so we can directly store 4 bytes at the beginning of the thread structure.

;------------------  Above is backing up the current thread's context. Below is restoring the next thread's context. ----------------
   mov eax, [esp + 24]         ; Get the parameter `next` from the stack, next = [esp + 24].
   mov esp, [eax]              ; The first member of the PCB is the `self_kstack` member, which records the top of the 0-level stack.
                               ; It is used to restore the 0-level stack when the thread is scheduled on the CPU.
                               ; The 0-level stack contains all the information of the process or thread, including the 3-level stack pointer.
   pop ebp
   pop ebx
   pop edi
   pop esi
   ret                         ; Return to the return address mentioned in the comment below `switch_to`.
                               ; If not entered via an interrupt, the first execution will return to `kernel_thread`.