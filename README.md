# OpenCL Kernel Static Feature Extractor

This program is able to extract static code features such as *number of basic blocks* or *number of compute operations* from an OpenCL kernel. The intent behind it is to collect static code features of multiple kernels to generate training data for machine learning models.

## Feature Extraction Approach

Rather than walking the abstract syntax tree of an OpenCL kernel to collect feature statistics, this program uses *llvm's* C frontend *clang* to translate an OpenCL C file into llvm [bit code](http://llvm.org/docs/BitCodeFormat.html). With *llvm's* [bit code reader API](http://llvm.org/docs/doxygen/html/ReaderWriter_8h.html) the generated code is then parsed and evaluated.

#### Extracted Features
* Functions
* Basic Blocks
* Binary Operations
* Bitwise Binary Operations
* Vector Operations
* Aggregate Operations
* Memory Load Operations
* Memory Store Operations
* Other Operations
* Global Memory Accesses
* Local Memory Accesses
* Private Memory Accesses
* Total Operations

#### Detailed Steps

1. Get OpenCL C kernel file (.cl) from target program.

2. Instrument kernel file with necessary macros to mark global and local memory accesses.

    ```
    #define __global __attribute__((address_space(1)))   
    #define __local __attribute__((address_space(2))) 
    int get_global_id(int index);
    ```

3. Compile the kernel to llvm bit code using clang and [libclc](https://github.com/llvm-mirror/libclc) header files. For the project clang version 3.8.0 was used.

    LLVM Bitcode:
    
    ```
    $ clang -include $LIBCLC_HOME/generic/include/clc/clc.h -I $LIBCLC_HOME/libclc/generic/include/ -DBLOCK_SIZE=16 -Dcl_clang_storage_class_specifiers nw.cl -o nw.bc -emit-llvm -c -O3 -x cl
    ```

    LLVM Assembly:
     
    ```
    $ clang -include $LIBCLC_HOME/libclc/generic/include/clc/clc.h -I $LIBCLC_HOME/libclc/generic/include/ -DBLOCK_SIZE=16 -Dcl_clang_storage_class_specifiers nw.cl -o nw.ll -emit-llvm -S -O3 -x cl
    ```
    
4. Use LLVM [bitcode reader](http://stackoverflow.com/questions/1838304/call-llvm-jit-from-c-program
) to parse LLVM bc file and extract features using library calls.

#### Example

1. Build the feature extractor with make

    ``` $ make ```

2. Execute it
    ```
    $ ./oclFeatureExt.out -f ./res/kernelSample/nw.bc -o features.csv
    Bitcode file loaded: ./res/kernelSample/nw.bc
    Bitcode parsed successfully!
    
    ###############################
    #Functions: 6
    #Basic Blocks: 23
    
    #Bin Ops: 206
    #Bit Bin Ops: 20
    #Vec Ops: 0
    #Agg Ops: 0
    #Load Ops: 86
    #Store Ops: 74
    #Other Ops: 271
    
    #Global Mem Access: 90
    #Local Mem Access: 70
    #Private Mem Access: 0
    -------------------------------------------
    #Total Ops: 657
    
    Writing to file: features.csv
    ```
 
## Python script to find and evaluate multiple kernels
Features are extracted from all kernels found in a subdirectory by running the provided tool after building the feature extractor with make.

```
$ ./tools/collectKernelFeatures.py -d ./res -o ./staticKernelFeatures.csv -l ../llvm_3.7.1/libclc/
```