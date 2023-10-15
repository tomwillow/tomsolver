import os
import re


def getFullPath(basename: str, include_dirs) -> str:
    """
    传入一个basename，查找include_dirs里面是否有这个文件，并返回全路径。
    """
    for dir in include_dirs:
        dest = os.path.join(dir, basename)
        if os.path.isfile(dest):
            return dest
    return None


def findAllFile(base):
    """
    通过迭代方式遍历文件夹下的文件名。
    """
    for root, ds, fs in os.walk(base):
        for f in fs:
            fullname = os.path.join(root, f)
            yield fullname


class MyClass:
    """
    这个类用于分析.h或者.cpp文件，读入文件，获取它的include ""的内容和include <>的内容。
    双引号的header视为内部依赖，尖括号的header视为外部依赖。
    #pragma once忽略掉。
    其他的内容存入self.contents。
    """

    def __init__(self, filename, include_dirs):
        self.filename = filename
        with open(filename, "r", encoding="utf-8") as f:
            lines_orig = f.readlines()

        # 内部依赖项，对于拓扑排序而言就是这个节点的入度
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

            innerDep = re.match(r"#include\s+\"([a-z_./\\]+)\"", stripedLine)
            if innerDep is not None:
                basename = innerDep.group(1)
                fullname = getFullPath(basename, include_dirs)
                if fullname is None:
                    raise RuntimeError("can not find full path of ", basename)
                self.depsInner.append(fullname)
                continue

            libDep = re.match(r"#include\s+<([a-z_./\\]+)>", stripedLine)
            if libDep is not None:
                basename = libDep.group(1)
                self.depsLib.append(basename)
                continue

            if stripedLine.find("#define") == 0:
                self.defines.append(stripedLine)
                continue

            self.contents.append(line.rstrip())

        print("analysis: ", filename, ":")
        print("  inner deps: ", self.depsInner)
        print("  std deps: ", self.depsLib)


def combineClasses(elements, output_filename):
    # 3. 拓扑排序
    sorted = []
    while len(elements) > 0:
        temp = []

        # 遍历找到入度为0的节点，加入序列
        for i in range(len(elements) - 1, -1, -1):
            cls = elements[i]
            if len(cls.depsInner) > 0:
                continue

            # 此时入度为0
            temp.append(cls)
            elements.remove(cls)

            for t in elements:
                try:
                    index = t.depsInner.index(cls.filename)
                except ValueError:
                    continue
                obj = t.depsInner[index]
                t.depsInner.remove(obj)

        # 如果本轮没有入度为0的元素，代表有环形依赖
        if len(temp) == 0:
            print("[FAIL] remain elements: ", elements)
            raise RuntimeError("topological sort fail")

        sorted.extend(temp)

    # 4. 提取所有的#define，#include <>的内容

    allDefines = []
    allStdDeps = []
    for ele in sorted:
        allDefines.extend(ele.defines)
        allStdDeps.extend(ele.depsLib)
    allDefines = list(set(allDefines))
    allStdDeps = list(set(allStdDeps))
    allDefines.sort()
    allStdDeps.sort()

    # 5. 写入
    with open(output_filename, "w") as f:
        f.write("#pragma once\n\n")

        # define先于#include写入
        for defs in allDefines:
            f.write(f"{defs}\n")

        # 写入标准库头文件include
        for header in allStdDeps:
            f.write(f"#include <{header}>\n")

        # 逐个把正文写入
        for ele in sorted:
            print("writing content of ", ele.filename, "...")
            f.write("\n".join(ele.contents))
            f.write("\n")

    # 6. 用clang-format处理
    ret = os.system(f"clang-format -i {output_filename}")
    if ret != 0:
        raise RuntimeError("[ERR] fail to run clang-format")

    print("Done.")


def combineCodeFile(target_dir, output_filename, src_filenames, include_dirs):
    # 1. 创建文件夹
    if not os.path.isdir(target_dir):
        os.makedirs(target_dir)

    # 2. 把源路径下所有文件都分析一遍
    elements = []
    for path in src_filenames:
        elements.append(MyClass(path, include_dirs))

    combineClasses(elements, output_filename)
