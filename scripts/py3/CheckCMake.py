import os
import sys

def audit_lists():
    cmake_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../CMakeLists.txt'))
    
    if not os.path.exists(cmake_path):
        print(f"ERROR: CMakeLists.txt not found at {cmake_path}")
        print("Please ensure you have initialized the project with a root CMakeLists.txt.")
        return
        
    with open(cmake_path, 'r') as f:
        lines = f.readlines()

    in_block = False
    block_name = ""
    
    for i, line in enumerate(lines):
        clean = line.strip()
        
        # detect start of blocks
        if clean.startswith("add_library(SharedCode"):
            in_block = True
            block_name = "SharedCode"
            print(f"--- Entering {block_name} at line {i+1} ---")
            continue
        elif clean.startswith("set(TestSources"):
            in_block = True
            block_name = "TestSources"
            print(f"--- Entering {block_name} at line {i+1} ---")
            continue
        elif clean.startswith("add_executable(BiquadraticQTests"):
            in_block = True
            block_name = "BiquadraticQTests"
            print(f"--- Entering {block_name} at line {i+1} ---")
            continue

        if in_block:
            closing = clean.endswith(")")
            if closing:
                clean = clean.rstrip(")").strip()

            # print the line content
            # skip empty lines or comments
            if clean and not clean.startswith("#"):
                print(f"[{block_name} L{i+1}]: {clean}")

            if closing:
                in_block = False
                print(f"--- Leaving {block_name} at line {i+1} ---")

if __name__ == '__main__':
    audit_lists()
