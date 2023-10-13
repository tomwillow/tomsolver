import os
import re

# repository的根目录
root_path = os.path.abspath(os.path.dirname(__file__) + "/..")

# 输出目标的include目录
include_path = os.path.join(root_path, "single/include/tomsolver")
output_filename = f"{include_path}/tomsolver.hpp"

# 分析依赖的起始文件
root_file = f"{root_path}/src/tomsolver.h"
src_path = f"{root_path}/src"


class MyClass:
    """
    这个类用于分析.h或者.cpp文件，读入文件，获取它的include ""的内容和include <>的内容。
    双引号的header视为内部依赖，尖括号的header视为外部依赖。
    #pragma once忽略掉。
    其他的内容存入self.contents。
    """

    def __init__(self, filename):
        with open(filename, "r", encoding="utf-8") as f:
            lines_orig = f.readlines()

        # 内部依赖项
        self.depsInner = []

        # 对外部库的依赖项
        self.depsLib = []

        # #define的内容
        self.defines = []

        self.contents = []

        for line in lines_orig:
            stripedLine = line.strip()

            # 忽略
            if stripedLine == "#pragma once":
                continue

            innerDep = re.match(r"#include\s+\"([a-z_.]+)\"", stripedLine)
            if innerDep is not None:
                self.depsInner.append(innerDep.group(1))
                continue

            libDep = re.match(r"#include\s+<([a-z_.]+)>", stripedLine)
            if libDep is not None:
                self.depsLib.append(libDep.group(1))
                continue

            if stripedLine.find("#define") == 0:
                self.defines.append(stripedLine)
                continue

            self.contents.append(line.rstrip())

        print("analysis: ", filename, ":")
        print("  inner deps: ", self.depsInner)
        print("  std deps: ", self.depsLib)


# 1. 创建文件夹
if not os.path.isdir(include_path):
    os.makedirs(include_path)

# 2. 从入口文件的include中逐个展开，得到['a.h', 'b.h']

# 这里使用BFS来依次遍历依赖关系。innerDeps在这里起到的作用是存储访问过的节点。
stdDeps = []
innerDeps = []
defines = []  # 所有的#define内容
innerDepsClass = {}  # key 是依赖文件字符串; value 是依赖文件的MyClass实例。
q = ["tomsolver.h"]
while len(q) > 0:
    f = q[0]
    q.pop(0)

    c = MyClass(f"{src_path}/{f}")
    innerDepsClass[f] = c

    # 没见过的外部依赖，加入到stdDeps中
    for dep in c.depsLib:
        if dep not in stdDeps:
            stdDeps.append(dep)

    for dep in c.depsInner:
        if dep in innerDeps:
            continue

        insertPos = len(innerDeps)

        # 如果当前查看的f已经位于依赖列表中，那么它的依赖应该插入到它的前面
        if f in innerDeps:
            if dep in innerDeps:
                innerDeps.remove(dep)

            insertPos = innerDeps.index(f)
        innerDeps.insert(insertPos, dep)
        q.append(dep)

    # 没见过的#define，加入到defines里面
    for defs in c.defines:
        if defs not in defines:
            defines.append(defs)

# 从这里开始识别.cpp的依赖

# 为了防止之后for循环中动态向innerDeps加入内容导致循环不结束，所以复制出一个innerDeps的副本
innerDeps2 = []
innerDeps2.extend(innerDeps)
for dep in innerDeps:
    depCpp = dep.replace(".h", ".cpp")
    if not os.path.isfile(f"{src_path}/{depCpp}"):
        print("[NOTICE] file not exist: ", depCpp)
        continue

    c = MyClass(f"{src_path}/{depCpp}")
    for dep in c.depsLib:
        if dep not in stdDeps:
            stdDeps.append(dep)

    innerDeps2.append(depCpp)
    innerDepsClass[depCpp] = c

innerDeps = innerDeps2

print("  inner deps: ", innerDeps)
print("  std deps: ", stdDeps)

# 3. 首先写入#pragma once
with open(output_filename, "w") as f:
    f.write("#pragma once\n\n")

    # 4. #define先于#include写入
    for defs in defines:
        f.write(f"{defs}\n")

    # 5. 写入标准库头文件include
    for header in stdDeps:
        f.write(f"#include <{header}>\n")

    # 6. 从依赖列表中逐个把.h的正文写入，把.cpp的正文写入
    for dep in innerDeps:
        print("writing content of ", dep, "...")
        c = innerDepsClass[dep]
        f.write("\n".join(c.contents))
        f.write("\n")

# 7. 用clang-format处理
ret = os.system(f"clang-format -i {output_filename}")
if ret != 0:
    raise RuntimeError("[ERR] fail to run clang-format")

print("Done.")
