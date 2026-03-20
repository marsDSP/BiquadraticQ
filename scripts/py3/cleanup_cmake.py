import os
import re

cmake_file = "../../CMakeLists.txt"
with open(cmake_file, "r") as f:
    lines = f.readlines()

new_lines = []
for line in lines:
    match = re.search(r'(source/[^\s"\'\)]+)', line)
    if match:
        file_path = match.group(1)
        if not os.path.exists(file_path):
            if '*' in file_path:
                new_lines.append(line)
                continue
            if file_path.endswith(('.h', '.cpp', '.hpp', '.c')):
                print(f"Removing reference to non-existent file: {file_path}")
                continue
    new_lines.append(line)

with open(cmake_file, "w") as f:
    f.writelines(new_lines)
