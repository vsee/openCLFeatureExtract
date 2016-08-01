#include <string>
#include <memory>
#include <iostream>
#include <unordered_set>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ADT/Twine.h>

using namespace std;
using namespace llvm;

/* TODO
 * count loads and stores total, global and local
 * 
 * count compute operations 
 * Binary Ops: add, fadd, sub, fsub, mul, fmul, udiv, sdif, fdiv, urem, srem, frem
 * Bitwise Binary Ops: shl, lshr, ashr, and, or xor
 * vector ops: extractelement, insertelement, shufflevector
 * aggregate ops: extractvalue, insertvalue
 */

static const unordered_set<string> BIN_OPS = {"add","fadd"};

int main(int argc, char* argv[]) {

    const string fileName = "res/kernelSample/nw.bc";
    
    ErrorOr<unique_ptr<MemoryBuffer>> fileBuffer = MemoryBuffer::getFile(fileName);
    if (error_code ec = fileBuffer.getError())
		outs() << "ERROR loading bitcode file: " << fileName << " -- " << ec.message() << "\n";
	else
		outs() << "Bitcode file loaded: " << fileName << "\n";  
    
    
    LLVMContext context;
    MemoryBufferRef memRef = (*fileBuffer)->getMemBufferRef();
    ErrorOr<unique_ptr<Module>> bcModule = parseBitcodeFile(memRef, context);
    if (error_code ec = bcModule.getError())
            outs() << "ERROR parsing bitcode file" << ec.message() << "\n";
    else
            outs() << "Bitcode parsed successfully!"<< "\n";
    
    int bbcount = 0;
    int funcount = 0;
    int instcount = 0;
       
    for (Module::const_iterator curFref = (*bcModule)->getFunctionList().begin(),
         endFref = (*bcModule)->getFunctionList().end();
         curFref != endFref; curFref++) {
        funcount++;
        outs() << "Found function: " << curFref->getName() << "\n";
 
        for (Function::const_iterator curBref = curFref->begin(), endBref = curFref->end();
         curBref != endBref; curBref++) {
            bbcount++;
            outs() << "Found a block: " << curBref->getName() << "\n";
            
            for (BasicBlock::const_iterator curIref = curBref->begin(), endIref = curBref->end();
                 curIref != endIref; curIref++) {
                instcount++;
                outs() << "Found an instruction: " << *curIref << "\n";
            }
            
        }
        
        
    }
    
    outs() << "\n###############################\n";
    outs() << "#Functions: " << funcount << "\n";
    outs() << "#Basic Blocks: " << bbcount << "\n";
    outs() << "#Instructions: " << instcount << "\n";
}
