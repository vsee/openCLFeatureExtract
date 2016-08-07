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
 */

const unordered_set<string> BIN_OPS = {"add","fadd", "sub", "fsub", "mul", "fmul", "udiv", "sdif", "fdiv", "urem", "srem", "frem"};
const unordered_set<string> BITBIN_OPS = {"shl","lshr", "ashr", "and", "or", "xor"};
const unordered_set<string> VEC_OPS = {"extractelement","insertelement", "shufflevector"};
const unordered_set<string> AGG_OPS = {"extractvalue","insertvalue"};

class FeatureStats {
    
public:
    int bbcount = 0;
    int funcount = 0;
 
    int binOpsCount = 0;
    int bitbinOpsCount = 0;
    int vecOpsCount = 0;
    int aggOpsCount = 0;
    int otherOpsCount = 0;

    // Global operator<< overload:
    friend raw_ostream &operator<<(raw_ostream &os, const FeatureStats &stats)
    {
        os << "\n###############################\n";
        os << "#Functions: " << stats.funcount << "\n";
        os << "#Basic Blocks: " << stats.bbcount << "\n";

        os << "\n#Bin Ops: " << stats.binOpsCount << "\n";
        os << "#Bit Bin Ops: " << stats.bitbinOpsCount << "\n";
        os << "#Vec Ops: " << stats.vecOpsCount << "\n";
        os << "#Agg Ops: " << stats.aggOpsCount << "\n";
        os << "#Other Ops: " << stats.otherOpsCount << "\n";
        os << "-------------------------------------------\n";
        os << "#Total Ops: " << (stats.binOpsCount + stats.bitbinOpsCount + 
                stats.vecOpsCount + stats.aggOpsCount + stats.otherOpsCount);
        
        return os;
    }
};

void evalInstruction(const Instruction &inst, FeatureStats &stats);

/* TODO dump to CSV
 * TODO initial arguments to point at file
 * TODO more features local and global space loading and storing
 * TODO get rid of all the debug output
 */

int main(int argc, char* argv[]) {

    FeatureStats stats;
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
    
       
    for (Module::const_iterator curFref = (*bcModule)->getFunctionList().begin(),
         endFref = (*bcModule)->getFunctionList().end();
         curFref != endFref; curFref++) {
        stats.funcount++;
#ifdef DEBUG
        outs() << "Found function: " << curFref->getName() << "\n";
#endif

        for (Function::const_iterator curBref = curFref->begin(), endBref = curFref->end();
         curBref != endBref; curBref++) {
            stats.bbcount++;
#ifdef DEBUG
            outs() << "Found a block: " << curBref->getName() << "\n";
#endif
            for (BasicBlock::const_iterator curIref = curBref->begin(), endIref = curBref->end();
                 curIref != endIref; curIref++) {
#ifdef DEBUG
                outs() << "Found an instruction: " << *curIref << "\n";
#endif                
                evalInstruction(*curIref, stats);
                
            }
            
        }
        
        
    }
    
    outs() << stats << "\n";
}



void evalInstruction(const Instruction &inst, FeatureStats &stats) {

    string opName = inst.getOpcodeName();
    if(BIN_OPS.find(opName) != BIN_OPS.end()) {
#ifdef DEBUG
        outs() << "Found bin op: " << inst.getOpcodeName() << "\n";
#endif
        stats.binOpsCount++;
    }
    else if(BITBIN_OPS.find(opName) != BITBIN_OPS.end()) {
#ifdef DEBUG
        outs() << "Found bitwise bin op: " << inst.getOpcodeName() << "\n";
#endif
        stats.bitbinOpsCount++;
    }
    else if(AGG_OPS.find(opName) != AGG_OPS.end()) {
#ifdef DEBUG
        outs() << "Found aggregate op: " << inst.getOpcodeName() << "\n";
#endif
        stats.vecOpsCount++;
    }
    else if(VEC_OPS.find(opName) != VEC_OPS.end()) {
#ifdef DEBUG
        outs() << "Found vector op: " << inst.getOpcodeName() << "\n";
#endif
        stats.aggOpsCount++;
    }
    else {
#ifdef DEBUG
        outs() << "Found other op: " << inst.getOpcodeName() << "\n";
#endif
        stats.otherOpsCount++;
    }
}
