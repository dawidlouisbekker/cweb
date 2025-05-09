(module
  (memory $mem 1)
  (export "memory" (memory $mem))

  ;; Function to set a character at an index
  ;; (i32 index, i32 charCode)
  (func $set_char (param $idx i32) (param $char i32)
    local.get $idx
    local.get $char
    i32.store8 ;; store as a single byte
  )
  (export "set_char" (func $set_char))

  ;; Function to get a character code at an index
  ;; (i32 index) -> i32
  (func $get_char (param $idx i32) (result i32)
    local.get $idx
    i32.load8_u ;; load a single byte (unsigned)
  )
  (export "get_char" (func $get_char))
)
