import os
import re

import util


# repository的根目录
root_dir = os.path.abspath(os.path.dirname(__file__) + "/..")

# 测试代码中可能包含的本仓库内的include目录。
# 用于合并测试文件时剔除#include语句
INCLUDE_DIR_PREFIXES = [
    os.path.abspath(f"{root_dir}/src"),
    os.path.abspath(f"{root_dir}/tests"),
]

class MyTest:
    """
    这个类用于分析xxx_test.cpp文件，提取出TEST(xxx,xxx) {} 内容填入self.contents。
    其他部分忽略不要。
    """

    def __init__(self, filename):
        self.filename = filename
        with open(filename, "r", encoding="utf-8") as f:
            lines_orig = f.readlines()

        self.contents = []

        inTest = False
        for line in lines_orig:
            line = line.rstrip()

            if re.match(r"TEST\(\w+,\s+\w+\)\s+{", line):
                if inTest:
                    raise RuntimeError("[ERR] parse error at " + line)
                self.contents.append(line)
                inTest = True
                continue

            if line == "}":
                if not inTest:
                    raise RuntimeError("[ERR] parse error at " + line)
                self.contents.append(line)
                inTest = False
                continue

            if inTest:
                self.contents.append(line)


class MyTestDepFile:
    """
    这个类用于解析测试cpp的依赖文件。内容填入self.contents。
    """

    def __init__(self, filename):
        self.filename = filename
        with open(filename, "r", encoding="utf-8") as f:
            lines_orig = f.readlines()

        self.contents = []

        for line in lines_orig:
            line = line.rstrip()

            # 忽略
            if line == "#pragma once":
                continue

            self.contents.append(line)


if __name__ == "__main__":
    # 输出目标的include目录
    target_dir = os.path.join(root_dir, "single/test")
    output_filename = f"{target_dir}/all_tests.cpp"

    srcFilenames = []
    for path in util.findAllFile(f"{root_dir}/tests"):
        if path.find("_test.cpp") != -1:
            srcFilenames.append(path)
    srcFilenames.sort()

    # 1. 创建文件夹
    if not os.path.isdir(target_dir):
        os.makedirs(target_dir)

    #
    elements = []
    for path in srcFilenames:
        elements.append(MyTest(path))

    #
    contents = []
    contents.extend(MyTestDepFile(f"{root_dir}/tests/memory_leak_detection.h").contents)
    contents.extend(
        MyTestDepFile(f"{root_dir}/tests/memory_leak_detection_win.h").contents
    )
    contents.extend(MyTestDepFile(f"{root_dir}/tests/helper.cpp").contents)

    tempContents = []
    for line in contents:
        # 去掉所有#include tomsolver的行
        obj = re.match(r"#include\s+[\"<]([\w_./\\]+)[\">]", line)
        if obj is None:
            tempContents.append(line)
            continue

        isInThisRepository = False
        pureIncludePath = obj.group(1)
        for dir in INCLUDE_DIR_PREFIXES:
            fullIncludePath = f"{dir}/{pureIncludePath}"
            if os.path.isfile(fullIncludePath):
                # if this included header file is in this repository, remove it (not append)
                isInThisRepository = True
                break
        if isInThisRepository:
            continue

        tempContents.append(line)

    contents = tempContents


    # 写入
    with open(output_filename, "w", encoding="utf-8") as f:
        # 填入预先准备好的头
        with open(f"{root_dir}/scripts/all_tests_preheader.cpp", "r") as ff:
            headerLines = ff.readlines()

        f.writelines(headerLines)

        f.write("\n")

        f.write("\n".join(contents))
        f.write("\n")

        # 写入所有测试
        for ele in elements:
            print("writing content of ", ele.filename, "...")
            f.write("\n".join(ele.contents))
            f.write("\n\n")

    # 用clang-format处理
    ret = os.system(f"clang-format -i {output_filename}")
    if ret != 0:
        raise RuntimeError("[ERR] fail to run clang-format")

    print("Done.")
