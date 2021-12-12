#include "llvm/Demangle/Demangle.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <array>
#include <cxxabi.h>
#include <sstream>
#include <vector>

using namespace llvm;

// std::array<char const*, 4> lookup = {
//     {"The demangling operation succeeded", "A memory allocation failure occurred",
//      "mangled_name is not a valid name under the C++ ABI mangling rules",
//      "One of the arguments is invalid"}};

// std::string demangle(const std::string& Name) {
//     if (Name.rfind("_Z", 0) != 0) {
//         return Name;
//     }
//     int status;
//     std::size_t size = Name.length() + 1;
//     char* buffer = static_cast<char*>(std::malloc(size));
//     char* demangled_name = NULL;
//     char* cstr_name = new char[size];
//     strcpy(cstr_name, Name.c_str());
//     demangled_name = abi::__cxa_demangle(cstr_name, buffer, &size, &status);

//     if (!demangled_name) {
//         return std::string();
//     }
//     std::string s(demangled_name);
//     return s;
// }

namespace {

    struct printfDebugPass : public FunctionPass {
        static char ID;

        printfDebugPass() : FunctionPass(ID) {}

        void processFunction(Function&);

        bool runOnFunction(Function& F) override {
            processFunction(F);
            return true;
        }
    };
} // end of anonymous namespace

void printfDebugPass::processFunction(Function& F) {
    std::string funcName = demangle(F.getName().str());

    Module* module = F.getParent();
    LLVMContext& context = module->getContext();
    Type* intType = Type::getInt32Ty(context);

    // Declare C standard library printf
    std::vector<Type*> printfArgsTypes({Type::getInt8PtrTy(context)});
    FunctionType* printfType = FunctionType::get(intType, printfArgsTypes, true);

    FunctionCallee printfFunc = module->getOrInsertFunction("printf", printfType);

    for (Function::iterator bb = F.begin(); bb != F.end(); ++bb) {
        for (BasicBlock::iterator inst = bb->begin(); inst != bb->end(); ++inst) {
            std::ostringstream message;
            Instruction* I = &*inst;

            // Check if this is the first instruction
            if (inst == bb->begin() && bb == F.begin()) {
                message << " ---> Entering " << funcName << std::endl;
            }
            // Check if this is a return instruction
            else if (isa<ReturnInst>(I)) {
                message.clear();
                message << " <--- Leaving " << funcName << std::endl;
            } else {
                continue;
            }

            IRBuilder<> Builder(I);
            Value* str;
            str = Builder.CreateGlobalStringPtr(message.str(), "str");
            std::vector<Value*> argsV({str});
            Builder.CreateCall(printfFunc, argsV);
        }
    }
}

/*
 * Register Pass
 */

char printfDebugPass::ID = 0;

static RegisterPass<printfDebugPass> X("printfDebugPass", "Debug Pass", false, false);

static RegisterStandardPasses Y(PassManagerBuilder::EP_EnabledOnOptLevel0,
                                [](const PassManagerBuilder& Builder, legacy::PassManagerBase& PM) {
                                    PM.add(new printfDebugPass());
                                });
