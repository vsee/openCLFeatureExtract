#!/usr/bin/python3.4

import fnmatch
import os
import sys
import subprocess
import csv
import argparse

# TODO generate temporary files in temporary dir

parser = argparse.ArgumentParser(description='Collect static kernel features for cl kernel files.')

parser.add_argument("-d", "--dataPath", type=str, help="Root folder with cl kernel files.", required=True)
parser.add_argument("-o", "--outputFile", type=str, help="Output file name for collected features.", required=True)
parser.add_argument("-l", "--libclcHome", type=str, help="Home directory of libclc.", required=True)

args = parser.parse_args()


srcDir = args.dataPath

kernels = []
for root, dirnames, filenames in os.walk(srcDir):
    for filename in fnmatch.filter(filenames, '*.cl'):
        kernels.append(os.path.join(root, filename))


print("Kernels found: ")
print(*kernels, sep="\n")

BITCODE_OUT = "out.bc"
FEATURE_OUT = "feature_out.csv"
COMPILE_CMD = ("clang -include %s -include " + args.libclcHome + "/generic/include/clc/clc.h "
"-I " + args.libclcHome + "/generic/include/ -DBLOCK_SIZE=16 -Dcl_clang_storage_class_specifiers %s "
"-o %s -emit-llvm -c -O3 -x cl -w")

KERNEL_HEADER_FILE = "kernel.h"
KERNEL_HEADER = """
#ifndef ADDRESS_SPACE_H
#define ADDRESS_SPACE_H

#define __local __attribute__((address_space(1)))
#define __global __attribute__((address_space(2)))

#endif
"""

with open(KERNEL_HEADER_FILE, 'w') as f:
    f.write(KERNEL_HEADER)

features = dict()
feature_head = None
for kernel in kernels:
    cmd = COMPILE_CMD % (KERNEL_HEADER_FILE, kernel, BITCODE_OUT)
    print("\n\n############################### COMPILING:\n" + cmd + "\n################################")
    p = subprocess.Popen(cmd, stderr=subprocess.STDOUT, shell=True)
    p.wait()

    cmd = "./oclFeatureExt.out -f ./out.bc -o %s" % (FEATURE_OUT)
    print("\n\n######################## GETTING FEATURES:\n" + cmd + "\n################################")
    p = subprocess.Popen(cmd, stderr=subprocess.STDOUT, shell=True)
    p.wait()
  
    with open(FEATURE_OUT, 'r') as csvfile:
        csvreader = csv.reader(csvfile)

        header = True
        for row in csvreader:
            if header:
                header = False
                if feature_head == None: feature_head = ['kernel'] + row
                continue

            features[kernel] = [kernel] + row


print("\n\n############################### CLEANUP ################################")
os.remove(BITCODE_OUT)
os.remove(FEATURE_OUT)
os.remove(KERNEL_HEADER_FILE)

print("\n\n####################### SAVING FEATURES ################################")

with open(args.outputFile, 'w') as f:
    writer = csv.writer(f)
    writer.writerow(feature_head)
    
    for name, values in features.items():
        writer.writerow(values)

print(str(len(features)) + " feature vectors written to: " + args.outputFile)




