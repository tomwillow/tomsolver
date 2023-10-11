import os
import sys
import shutil
import re

root_path = os.path.abspath(os.path.dirname(__file__)+"/..")

include_path = os.path.join(root_path, "include/tomsolver")

if not os.path.isdir(include_path):
    os.makedirs(include_path)

root_file = f"{root_path}/src/tomsolver.h"


class MyClass:
    def __init__(self, filename):
        with open(filename, "r", encoding="utf-8") as f:
            lines_orig = f.readlines()

        # 内部依赖项
        self.depsInner = []

        # 对外部库的依赖项
        self.depsLib = []

        self.content = []

        lines = []
        for line in lines_orig:
            line = line.strip()

            if line == "#pragma once":
                continue

            innerDep = re.match(r"#include\s+\"([a-z_.]+)\"", line)
            if innerDep is not None:
                self.depsInner.append(innerDep.group(1))
                continue

            libDep = re.match(r"#include\s+<([a-z_.]+)>", line)
            if libDep is not None:
                self.depsLib.append(libDep.group(1))
                continue

            lines.append(line)




        print(self.depsInner)
        print(self.depsLib)


my = MyClass(f"{root_path}/src/subs.cpp")

# with open(include_path+"/tomsolver.hpp", "w") as f:
#     f.write("ddddd")