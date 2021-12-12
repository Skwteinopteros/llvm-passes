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
