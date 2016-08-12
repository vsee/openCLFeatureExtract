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
    int loadOpsCount = 0;
    int storeOpsCount = 0;
    int otherOpsCount = 0;
    
    int globalMemAcc = 0;
    int localMemAcc = 0;

    // Global operator<< overload:
    friend ostream &operator<<(ostream &os, const FeatureStats &stats)
    {
        os << "\n###############################\n";
        os << "#Functions: " << stats.funcount << "\n";
        os << "#Basic Blocks: " << stats.bbcount << "\n";

        os << "\n#Bin Ops: " << stats.binOpsCount << "\n";
        os << "#Bit Bin Ops: " << stats.bitbinOpsCount << "\n";
        os << "#Vec Ops: " << stats.vecOpsCount << "\n";
        os << "#Agg Ops: " << stats.aggOpsCount << "\n";
        os << "#Load Ops: " << stats.loadOpsCount << "\n";
        os << "#Store Ops: " << stats.storeOpsCount << "\n";
        os << "#Other Ops: " << stats.otherOpsCount << "\n";
        
        os << "\n#Global Mem Access: " << stats.globalMemAcc << "\n";
        os << "#Local Mem Access: " << stats.localMemAcc << "\n";
        os << "-------------------------------------------\n";
        os << "#Total Ops: " << (stats.binOpsCount + stats.bitbinOpsCount + 
                stats.vecOpsCount + stats.aggOpsCount + stats.otherOpsCount +
                stats.loadOpsCount + stats.storeOpsCount);
        
        return os;
    }
};

void evalInstruction(const Instruction &inst, FeatureStats &stats);

/* TODO dump to CSV
 * TODO initial arguments to point at file
 * TODO more features local and global space loading and storing
 */

int main(int argc, char* argv[]) {
    FeatureStats stats;
    const string fileName = "res/kernelSample/nw.bc";

    ErrorOr<unique_ptr<MemoryBuffer>> fileBuffer = MemoryBuffer::getFile(fileName);
    if (error_code ec = fileBuffer.getError())
		cout << "ERROR loading bitcode file: " << fileName << " -- " << ec.message() << "\n";
	else
		cout << "Bitcode file loaded: " << fileName << "\n";  
    
    
    LLVMContext context;
    MemoryBufferRef memRef = (*fileBuffer)->getMemBufferRef();
    ErrorOr<unique_ptr<Module>> bcModule = parseBitcodeFile(memRef, context);
    if (error_code ec = bcModule.getError())
            cout << "ERROR parsing bitcode file" << ec.message() << "\n";
    else
            cout << "Bitcode parsed successfully!"<< "\n";
    
       
    for (Module::const_iterator curFref = (*bcModule)->getFunctionList().begin(),
         endFref = (*bcModule)->getFunctionList().end();
         curFref != endFref; curFref++) {
        stats.funcount++;

        for (Function::const_iterator curBref = curFref->begin(), endBref = curFref->end();
         curBref != endBref; curBref++) {
            stats.bbcount++;
            for (BasicBlock::const_iterator curIref = curBref->begin(), endIref = curBref->end();
                 curIref != endIref; curIref++) {
                
#ifdef DEBUG
                outs() << "Found an instruction: " << *curIref << "\n";
#endif                
                evalInstruction(*curIref, stats);               
            }
            
        }
        
        
    }
    
    cout << stats << "\n";
}

void evalInstruction(const Instruction &inst, FeatureStats &stats) {

    string opName = inst.getOpcodeName();
    if(BIN_OPS.find(opName) != BIN_OPS.end()) {
        stats.binOpsCount++;
    }
    else if(BITBIN_OPS.find(opName) != BITBIN_OPS.end()) {
        stats.bitbinOpsCount++;
    }
    else if(AGG_OPS.find(opName) != AGG_OPS.end()) {
        stats.vecOpsCount++;
    }
    else if(VEC_OPS.find(opName) != VEC_OPS.end()) {
        stats.aggOpsCount++;
    } else if(opName == "load") {
#ifdef DEBUG
        cout << "Found load op: " << inst.getOpcodeName() << "\n";
#endif
        stats.loadOpsCount++;
  
//        outs() << inst.getNumOperands() << "\n";
//        if(inst.getOperandUse(0)->getType()->isPointerTy()) {
//            PointerType* oppType = (PointerType*) inst.getOperandUse(1)->getType();
//            if(oppType->getAddressSpace() == 1) {
//                stats.localMemAcc++;
//            } else if(oppType->getAddressSpace() == 2) {
//                stats.globalMemAcc++;
//            } else {
//                cout << "WARNING: unhandled address space: " << oppType << "\n";
//            }
//        }
    } else if(opName == "store") {
#ifdef DEBUG
        cout << "Found store op: " << inst.getOpcodeName() << "\n";
#endif
        stats.storeOpsCount++;
        // TODO what about this??
        //outs() << ((StoreInst) inst).getPointerAddressSpace(); "\n";
        
        // TODO what about that??
        //outs() << isa<LoadInst>(inst) << "\n";
        
        outs() << inst.getNumOperands() << "\n";
        if(inst.getOperandUse(1)->getType()->isPointerTy()) {
            PointerType* oppType = (PointerType*) inst.getOperandUse(1)->getType();
            if(oppType->getAddressSpace() == 1) {
                outs() << "Local Mem Access" << "\n";
                stats.localMemAcc++;
            } else if(oppType->getAddressSpace() == 2) {
                outs() << "Global Mem Access" << "\n";
                stats.globalMemAcc++;
            } else {
                cout << "WARNING: unhandled address space: " << oppType << "\n";
            }
        }
    } else {
        stats.otherOpsCount++;
    }
}
