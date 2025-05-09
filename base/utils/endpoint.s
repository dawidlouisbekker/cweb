.globl endp_qry
.extern printf

.section .data
print_fmt: .asciz "%d\n"  # Format string for printing integers
print_fmt_addr:
    .asciz "Address: %p\n"

endp_qry:
    pushq %rbp
    movq %rsp, %rbp

    movq %rdi, %rax        # rax = pointer to stack (int*)
    movq %rsi, %rbx        # rbx = pointer to request string (char*)

    movzbl 1(%rbx), %ecx   # ecx = ASCII of request[1]
                           # movzbl (zero-extend byte to 32-bit)

    leaq (%rax,%rcx,8), %r8  # r8 = address of stack + ecx * sizeof(int)
                             # each int is 4 bytes
    popq %rbp
    ret

.loop:
    cmpq %rcx, %rbx        # if (i >= count) break
    jge .done

    movl (%rax,%rbx,4), %edx   # edx = stack[i] (4 bytes per int)
    # do something with %edx...

    incq %rbx             # i++
    jmp .loop

.done:
    popq %rbp
    ret


