#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <unordered_set>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ADT/Twine.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Casting.h>

using namespace std;
using namespace llvm;

const unordered_set<string> BIN_OPS = {"add","fadd", "sub", "fsub", "mul", "fmul", "udiv", "sdif", "fdiv", "urem", "srem", "frem"};
const unordered_set<string> BITBIN_OPS = {"shl","lshr", "ashr", "and", "or", "xor"};
const unordered_set<string> VEC_OPS = {"extractelement","insertelement", "shufflevector"};
const unordered_set<string> AGG_OPS = {"extractvalue","insertvalue"};


#define LOCAL_ADDRESS_SPACE 1
#define GLOBAL_ADDRESS_SPACE 2
#define USAGE "Usage:\n\t-h help\n\t-f <kernel bitcode file>\n\t-o <output file>\n\t-v verbose\n"

class InputParser{
    public:
        InputParser (int &argc, char **argv){
            for (int i=1; i < argc; ++i)
                this->tokens.push_back(string(argv[i]));
        }
        /// @author iain
        const string& getCmdOption(const string &option) const{
            vector<string>::const_iterator itr;
            itr =  find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()){
                return *itr;
            }
            static const string empty = "";
            return empty;
        }
        /// @author iain
        bool cmdOptionExists(const string &option) const{
            return find(this->tokens.begin(), this->tokens.end(), option)
                   != this->tokens.end();
        }
    private:
        vector <string> tokens;
};

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
        os << "#Total Ops: " << stats.getTotalOpsCount();
        
        return os;
    }
    
    int getTotalOpsCount() const {
        return binOpsCount + bitbinOpsCount + 
                vecOpsCount + aggOpsCount + otherOpsCount +
                loadOpsCount + storeOpsCount;
    }
};

void evalInstruction(const Instruction &inst, FeatureStats &stats);
void writeToCSV(const string &outFile, const FeatureStats &stats);

bool verbose = false;

int main(int argc, char* argv[]) {
    InputParser input(argc, argv);
    
    if(input.cmdOptionExists("-h")) {
        cout << USAGE;
        exit(0);
    }
    if(input.cmdOptionExists("-v")) {
        verbose = true;
    }
    const string &fileName = input.getCmdOption("-f");
    if (fileName.empty()){
        cout << USAGE;
        exit(1);
    }
    const string &outFile = input.getCmdOption("-o");
    if (outFile.empty()){
        cout << USAGE;
        exit(1);
    }
    
    FeatureStats stats;

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
                if(verbose) outs() << "Found an instruction: " << *curIref << "\n";        
                evalInstruction(*curIref, stats);               
            }
            
        }
        
        
    }
    
    cout << stats << "\n\n";
    writeToCSV(outFile, stats);
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
        stats.loadOpsCount++;
  
        if (const LoadInst *li = dyn_cast<LoadInst>(&inst)) {
            if(li->getPointerAddressSpace() == LOCAL_ADDRESS_SPACE) {
                stats.localMemAcc++;
            } else if(li->getPointerAddressSpace() == GLOBAL_ADDRESS_SPACE) {
                stats.globalMemAcc++;
            } else {
                cout << "WARNING: unhandled address space: " << li->getPointerAddressSpace() << "\n";
            }
        }
    } else if(opName == "store") {
        stats.storeOpsCount++;
        
        if (const StoreInst *si = dyn_cast<StoreInst>(&inst)) {
            if(si->getPointerAddressSpace() == LOCAL_ADDRESS_SPACE) {
                stats.localMemAcc++;
            } else if(si->getPointerAddressSpace() == GLOBAL_ADDRESS_SPACE) {
                stats.globalMemAcc++;
            } else {
                cout << "WARNING: unhandled address space: " << si->getPointerAddressSpace() << "\n";
            }
        }
    } else {
        stats.otherOpsCount++;
    }
}

void writeToCSV(const string &outFile, const FeatureStats &stats) {
    cout << "Writing to file: " << outFile << endl;
    
    ofstream out;
    out.open (outFile);
    out << "functions,bbs,binOps,bitBinOps,vecOps,aggOps,loadOps,storeOps,otherOps,totalOps,gMemAcc,lMemAcc" << endl;
    out << stats.funcount << "," <<
            stats.bbcount << "," <<
            stats.binOpsCount << "," <<
            stats.bitbinOpsCount << "," <<
            stats.vecOpsCount << "," <<
            stats.aggOpsCount << "," <<
            stats.loadOpsCount << "," <<
            stats.storeOpsCount << "," <<
            stats.otherOpsCount << "," <<
            stats.getTotalOpsCount() << "," <<
            stats.globalMemAcc << "," <<
            stats.localMemAcc << endl;
    out.close();
}