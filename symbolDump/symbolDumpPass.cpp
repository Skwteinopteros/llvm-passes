#include "llvm/Demangle/Demangle.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <filesystem>
#include <fstream>
#include <iostream>

using namespace llvm;

namespace {

    struct symbolDumpPass : public ModulePass {
        static char ID;

        symbolDumpPass() : ModulePass(ID) {}

        void processModule(Module& M);

        bool runOnModule(Module& M) override {
            processModule(M);
            return true;
        }
    };
} // end of anonymous namespace

void symbolDumpPass::processModule(Module& M) {
    // std::string funcName = demangle(F.getName().str());
    std::filesystem::path moduleFile = M.getSourceFileName();

    std::fstream outputFile;
    std::string fileName = (moduleFile.parent_path() / moduleFile.stem()).c_str();
    fileName.append(".symbols");

    outputFile.open(fileName, std::ios::out);
    for (auto& F : M) {
        std::string funcName = F.getName().str();
        outputFile << funcName << "|" << demangle(funcName) << std::endl;
    }
    outputFile.close();
}

/*
 * Register Pass
 */

char symbolDumpPass::ID = 0;

static RegisterPass<symbolDumpPass> X("symbolDumpPass", "Symbol Dump Pass", false, false);

static RegisterStandardPasses Y(PassManagerBuilder::EP_EnabledOnOptLevel0,
                                [](const PassManagerBuilder& Builder, legacy::PassManagerBase& PM) {
                                    PM.add(new symbolDumpPass());
                                });
