[bits 32]
section .text
; for the thread schedule

global switch_to
switch_to:
   ; push the results fro savings    
   push esi
   push edi
   push ebx
   push ebp

   mov eax, [esp + 20]		 
   mov [eax], esp       ; cache the thread envs
;------------------  reload the next one  ----------------
   mov eax, [esp + 24]		
   mov esp, [eax]
   pop ebp
   pop ebx
   pop edi
   pop esi
   ret				
