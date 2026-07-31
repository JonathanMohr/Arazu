[bits 32]

section .text

global test_function

test_function:
    mov eax, number
.local_symbol:
    inc [eax + 4]
    ret

section .data

global msg
global number

msg db "Hello, World!", 0x10, 0x00
number dq 0x12D687
