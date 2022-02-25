add_llvm_library(symbolDump MODULE
    symbolDumpPass.cpp
)

set_target_properties(symbolDump
    PROPERTIES PREFIX ""
    COMPILE_FLAGS "-fno-rtti"
)
