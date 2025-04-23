(module
  ;; Declare linear memory (64KiB = 1 page)
  (memory $mem 1)
  (export "memory" (memory $mem))

  ;; Stack pointer stored at memory offset 0
  (global $sp (mut i32) (i32.const 4)) ;; stack grows at offset 4+
  
  ;; Constants
  (global $stack_base (mut i32) (i32.const 4))
  (global $stack_limit (mut i32) (i32.const 4096)) ;; max size = (4096 - 4) / 4 = 1023 elements

  ;; Push function
  (func $stack_push (param $val i32)
    global.get $sp
    global.get $stack_limit
    i32.ge_u
    if ;; Check for overflow
      unreachable ;; could trap or return error instead
    end

    global.get $sp
    local.get $val
    i32.store ;; store the value at [sp]

    global.get $sp
    i32.const 4
    i32.add
    global.set $sp ;; increment stack pointer
  )
  (export "stack_push" (func $stack_push))

  ;; Pop function
  (func $stack_pop (result i32)
    global.get $sp
    global.get $stack_base
    i32.le_u
    if ;; Check for underflow
      unreachable ;; could trap or return error instead
    end

    global.get $sp
    i32.const 4
    i32.sub
    global.set $sp

    global.get $sp
    i32.load ;; return value
  )
  (export "stack_pop" (func $stack_pop))
)
