# [First assembly program: working with strings and integers](https://codereview.stackexchange.com/questions/277564)
### tags: ['performance', 'beginner', 'assembly', 'x86']

I started learning assembly some days ago and I wrote my first program today. Nothing exceptional here: some user inputs, strings manipulations, integer to string conversion etc... The only purpose was to test some things. I would like to have your reviews and advices to improve my code. Also if i didn't make big mistakes.

I'm looking for advice about instructions alignment in loops for example. Except inserting nops, I don't have other idea. Or know the size of all instructions and predict the good alignment on 16 or 32 bytes boundaries.

```
SYS_READ    equ 3
SYS_WRITE   equ 4
SYS_EXIT    equ 1
STDIN       equ 0
STDOUT      equ 1

%macro printm 2
    mov eax, SYS_WRITE
    mov ebx, STDOUT
    mov ecx, %1
    mov edx, %2
    int 0x80
%endmacro

%macro readm 2
    mov eax, SYS_READ
    mov ebx, STDIN
    mov ecx, %1
    mov edx, %2
    int 0x80
%endmacro

%macro prolog 0
    push ebp,
    mov ebp, esp
%endmacro

%macro epilog 0    
    mov esp, ebp
    pop ebp
%endmacro

%use smartalign

section .text

global _start

_start:

    push ebp
    mov ebp, esp
    and esp, 0xFFFFFFF0
    
    ; first check if our strlen proc works
    push dword msgbegin
    call strlen
    add esp, byte 4
    cmp eax, lenbegin
    jne .exit       ; it works, we continue
    
    ; after we copy the msgbegin in string strdst
    mov ecx, lenbegin
    mov esi, msgbegin
    mov edi, strdst
    rep movsb

    ; we print the string to check if the memcpy is good 
    printm strdst, lenbegin

    ; after we ask for user to enter two number (1 digit each)
    printm msgbinp1, leninp1

    readm num1, 2

    printm msgbinp2, leninp2

    readm num2, 2

    ; user input to enter a number greater than one digit
    printm msgbinp3, leninp3

    readm bignum, 4

    ; we convert this string number to an (dword) integer value
    mov edx, bignum
    call atoi
    ; we compare it with 123 to check if atoi worked
    cmp eax, dword 123
    jne .exit     ; exit if bignum != 123

    ; need to strip line feed from bignum
    printm bignum, 4
    printm msgoutp, lenoutp

    ; now we compute the sum with the first two digits
    mov al, byte [num1]
    sub al, '0'
    mov bl, byte [num2]
    sub bl, '0'
    add al, bl
    add al, '0'

    mov [sum], al
    
    ; we put the string msgres to uppercase
    mov esi, msgres

.next_char:
    lodsb
    test al, al     ; check for end of string
    jz .done      
    cmp al, 'a'     ; ignore unless in range
    jl .next_char
    cmp al, 'z'
    jg .next_char
    sub al, 0x20 ; convert to upper case         
    mov [esi-1], al ; write back to string
    jmp .next_char

.done:

    printm msgres, lenres
    ; we print the sum
    printm sum, 1

.exit:    
    ; exiting the programm
    mov     eax, SYS_EXIT
    int     0x80

strlen:
    push edi
    mov edi, [esp + 8]
    sub	ecx, ecx
	sub	al, al
	mov	ecx, -1
    cld
    repne scasb
    not ecx
    mov	eax, ecx ; keep null term in size
    pop edi
    ret

atoi:
    xor eax, eax            ; zero a "result so far"
.top:
    movzx ecx, byte [edx]   ; get a character
    inc edx                 ; ready for next one
    cmp ecx, '0'            ; valid?
    jb .done
    cmp ecx, '9'
    ja .done
    sub ecx, '0'            ; "convert" character to number
    imul eax, 10            ; multiply "result so far" by ten
    add eax, ecx            ; add in current digit
    jmp .top                ; until done

.done:
    ret

    ; not implemented yet
rand:
    push ebp
    mov ebp, esp

    rdtsc

    xor edx, edx        ; to always fit

    div dword [ebp + 8] ; range
    mov eax, edx

    mov esp, ebp
    pop ebp
    ret    

section .data

    msgbegin db  "hello everyone !", 0xa, 0
    lenbegin equ $ - msgbegin
    msgbinp1 db  "Enter a digit : ", 0xa, 0
    leninp1 equ $ - msgbinp1
    msgbinp2 db  "Enter second digit : ", 0xa, 0
    leninp2 equ $ - msgbinp2
    msgbinp3 db  "Enter third digit : ", 0xa, 0
    leninp3 equ $ - msgbinp3
    msgoutp db  "is equal to 123 !", 0xa, 0
    lenoutp equ $ - msgoutp
    msgres db  "sum of x and y is ", 0xa,  0
    lenres equ $ - msgres
    strdst times lenbegin db 0

segment .bss

    sum     resb 1
    num1    resb 2
    num2    resb 2
    bignum  resd 1
```
