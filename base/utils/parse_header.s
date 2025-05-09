.section .data
host_key:
    .ascii "Host:"
    .byte 0

.section .text
.global parse_header
.type parse_header, @function

parse_header:
    push %rbp
    mov %rsp, %rbp

    mov %rdi, %rsi         # rsi = header pointer

    # Initialize counter for 10 lines
    mov $10, %rcx          # max 10 lines

.next_line:
    cmp $0, %rcx           # Check if we've processed 10 lines
    je .done               # Exit if 10 lines have been processed
    dec %rcx               # Decrement line counter

    mov %rsi, %rdi         # rdi = start of line

    # Skip leading whitespace, if any
.skip_whitespace:
    cmpb $' ', (%rsi)      # Check if the current byte is a space
    je .skip_next_char     # If it's a space, skip it
    cmpb $'\t', (%rsi)     # Check if it's a tab
    je .skip_next_char     # If it's a tab, skip it
    jmp .check_line_end    # If it's neither, move to line processing

.skip_next_char:
    inc %rsi               # Move to the next character
    jmp .skip_whitespace

.check_line_end:
    cmpb $0, (%rsi)        # Check for null byte (end of buffer)
    je .done               # If we're at the end of the buffer, finish
    cmpb $'\n', (%rsi)     # Check for newline (end of line)
    je .found_newline      # If newline, proceed to next line

    # Compare with "Host:" if line is not empty
    lea host_key(%rip), %rdi
    mov %rsi, %rsi
    call strncmp           # strncmp(header_line, "Host:", 5)
    cmp $0, %rax
    jne .skip_line

    # Print if it starts with "Host:"
    mov %rsi, %rdi
    call puts

.skip_line:
    # Skip to next line
    inc %rsi
    jmp .check_line_end

.found_newline:
    inc %rsi              # move past \n
    jmp .next_line

.done:
    pop %rbp
    ret


