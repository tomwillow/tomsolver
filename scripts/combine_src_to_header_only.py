import os
import util


# repository的根目录
root_dir = os.path.abspath(os.path.dirname(__file__) + "/..")


if __name__ == "__main__":
    # 输出目标的include目录
    target_dir = os.path.join(root_dir, "single/include/tomsolver")
    output_filename = f"{target_dir}/tomsolver.h"

    srcFilenames = []
    for path in util.findAllFile(f"{root_dir}/src"):
        srcFilenames.append(path)
    srcFilenames.sort()

    include_dirs = [f"{root_dir}/src/tomsolver"]

    util.combineCodeFile(target_dir, output_filename, srcFilenames, include_dirs)
