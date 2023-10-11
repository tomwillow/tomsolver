import os
import sys
import shutil

root_path = os.path.abspath(os.path.dirname(__file__)+"/..")

include_path = os.path.join(root_path, "include/tomsolver")

if not os.path.isdir(include_path):
    os.makedirs(include_path)

with open(include_path+"/tomsolver.hpp", "w") as f:
    f.write("ddddd")