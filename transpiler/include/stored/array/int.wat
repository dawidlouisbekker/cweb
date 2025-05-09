(module
  (memory $mem 1)
  (export "memory" (memory $mem))
  (func $set_array (param $idx i32) (param $val i32)
    local.get $idx
    i32.const #
    i32.mul
    local.get $val
    i32.store
  )
  (export "set_array" (func $set_array))

  ;; Function to get a value at an index in the array
  ;; Takes (i32 index), returns (i32)
  (func $get_array (param $idx i32) (result i32)
    local.get $idx
    i32.const #
    i32.mul
    i32.load ;; load value from calculated address
  )
  (export "get_array" (func $get_array))
)
