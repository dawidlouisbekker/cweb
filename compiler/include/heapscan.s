.section .text
.globl scan_heap
.type scan_heap, @function

# int scan_heap(char* heap, int size);
# rdi = heap pointer
# rsi = size

scan_heap:
    movl $0, %eax             # eax = 0 (index)

.loop:
    cmpl %esi, %eax           # if (eax >= size) goto not_found
    jge .not_found

    movzbl (%rdi,%rax,1), %edx   # edx = heap[eax]
    cmpb $0, %dl
    jne .next

    movzbl 1(%rdi,%rax,1), %edx  # edx = heap[eax+1]
    cmpb $0, %dl
    je .found

.next:
    incl %eax
    jmp .loop

.not_found:
    movl $-1, %eax
    ret

.found:
    ret

.section .note.GNU-stack,"",@progbits
