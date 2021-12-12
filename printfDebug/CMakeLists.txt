add_llvm_library(printfDebug MODULE
    printfDebugPass.cpp
)

set_target_properties(printfDebug
    PROPERTIES PREFIX ""
    COMPILE_FLAGS "-fno-rtti"
)
