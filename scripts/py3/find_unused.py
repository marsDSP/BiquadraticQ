import os
import re
from pathlib import Path

source_dir = Path("../../source").resolve()
include_re = re.compile(r'^\s*#include\s*(["<])([^">]+)([">])', re.MULTILINE)
files = list(source_dir.rglob("*.cpp")) + list(source_dir.rglob("*.c")) + list(source_dir.rglob("*.h")) + list(source_dir.rglob("*.hpp"))
file_paths = {f.resolve(): f for f in files}
graph = {f: set() for f in file_paths.keys()}
def resolve_include(current_file, include_path_str):
    rel_path = current_file.parent / include_path_str
    if rel_path.resolve() in file_paths:
        return rel_path.resolve()
    
    src_path = source_dir / include_path_str
    if src_path.resolve() in file_paths:
        return src_path.resolve()
    
    return None

for f in file_paths.keys():
    try:
        content = f.read_text(encoding="utf-8")
        for match in include_re.finditer(content):
            inc_path = match.group(2)
            resolved = resolve_include(f, inc_path)
            if resolved:
                graph[f].add(resolved)
    except Exception as e:
        pass

entry_points = [
    source_dir / "PluginProcessor.h",
    source_dir / "PluginProcessor.cpp",
    source_dir / "PluginEditor.h",
    source_dir / "PluginEditor.cpp",
]

visited_headers = set()
queue = []
for ep in entry_points:
    if ep.resolve() in graph:
        queue.append(ep.resolve())
        visited_headers.add(ep.resolve())

while queue:
    curr = queue.pop(0)
    for neighbor in graph.get(curr, []):
        if neighbor not in visited_headers:
            visited_headers.add(neighbor)
            queue.append(neighbor)

used_files = set(visited_headers)
for f in file_paths.keys():
    if f.suffix in ['.cpp', '.c']:
        includes = graph.get(f, set())
        if any(inc in visited_headers for inc in includes):
            used_files.add(f)
            queue = [f]
            while queue:
                curr = queue.pop(0)
                for neighbor in graph.get(curr, []):
                    if neighbor not in used_files:
                        used_files.add(neighbor)
                        queue.append(neighbor)

unused = set(file_paths.keys()) - used_files

for f in sorted(list(unused)):
    print(f.relative_to(source_dir))
