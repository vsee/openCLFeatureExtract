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
    const Twine fileName = Twine("res/kernelSample/nws.bc");
    ErrorOr<unique_ptr<MemoryBuffer>> fileBuffer = MemoryBuffer::getFile(fileName);
    if (error_code ec = fileBuffer.getError())
		cout << "Error loading bitcode file: " << fileName.str() << " -- " << ec << endl;
	else
		cout << "Bitcode file loaded: " << fileName.str() << endl;
   
    //~ ErrorOr< std::unique_ptr< Module > > module = parseBitcodeFile(MemoryBufferRef Buffer, &context);	
}
