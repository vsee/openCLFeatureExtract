#include <string>
#include <memory>
#include <iostream>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ADT/Twine.h>

using namespace std;
using namespace llvm;

int main(int argc, char* argv[]) {

    LLVMContext context;
    const string fileName = "res/kernelSample/nw.bc";
    ErrorOr<unique_ptr<MemoryBuffer>> fileBuffer = MemoryBuffer::getFile(fileName);
    if (error_code ec = fileBuffer.getError())
		cout << "ERROR loading bitcode file: " << fileName << " -- " << ec.message() << endl;
	else
		cout << "Bitcode file loaded: " << fileName << endl;  
    
    MemoryBufferRef memRef = (*fileBuffer)->getMemBufferRef();
    ErrorOr<unique_ptr<Module>> bcModule = parseBitcodeFile(memRef, context);
    if (error_code ec = bcModule.getError())
            cout << "ERROR parsing bitcode file" << ec.message() << endl;
    else
            cout << "Bitcode parsed successfully!"<< endl;
    
    
}
