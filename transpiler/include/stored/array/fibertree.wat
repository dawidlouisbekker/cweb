(module
  (memory (export "memory") 1)

  ;; Store a null-terminated string "Hello" at memory offset 100
  (data (i32.const 100) "div\00")

  ;; Optionally export the pointer for convenience
  (func (export "get_comp") (result i32)
    (i32.const 100)
  )
)