(module
  ;; Reserve 1 page = 64KiB of memory
  (memory $mem 1)
  (export "memory" (memory $mem))

  ;; Write "Hi" (H = 72, i = 105) into memory at offset 100
  (func $write_hello
    ;; 'H'
    i32.const 100
    i32.const 72
    i32.store8

    ;; 'i'
    i32.const 101
    i32.const 105
    i32.store8

    ;; null terminator
    i32.const 102
    i32.const 0
    i32.store8
  )
  (export "write_hello" (func $write_hello))

  ;; Read first byte from string at offset 100 and return it
  (func $read_first_char (result i32)
    i32.const 100
    i32.load8_u
  )
  (export "read_first_char" (func $read_first_char))
)