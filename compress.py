import sys
import os
import re

if len(sys.argv) != 2:
    print(f"Usage: python3 {sys.argv[0]} <main.cpp>");
    quit()

includes = []

def getNameIncludedFile(line):
    line = str(line)
    i = 10
    l = []
    while line[i] != "\"" :
        l.append(line[i])
        i += 1
    return "".join(l)

def printFile(file_name, spaces=0):
    file_name = os.path.abspath(file_name)
    if file_name in includes:
        return
    
    print(" " * spaces + "// *** " + "Start of: " + file_name + " *** ")
    includes.append(file_name)
    
    file = open(file_name, 'r')

    os.chdir(os.path.dirname(file_name))

    content = file.read().split('\n')

    for line in content:
        if line.strip() == "#pragma once":
            continue

        if re.match(r"\#include \".+\"", line.strip()) != None:
            include_file = getNameIncludedFile(line)
        
            printFile(os.path.abspath(include_file), spaces + 1)
            os.chdir(os.path.dirname(file_name))
            # print("")
            continue
        print(" " * spaces + line)
    print(" " * spaces + "// *** " + "End of: " + file_name + " *** ");
    file.close()

printFile(sys.argv[1])
