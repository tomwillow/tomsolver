# Environment

* all the C++ code must be formated by clang-format, so you should prepare an executive clang-format and configure it well. e.g.
  * at Visual Studio, you can get it from this:
    https://learn.microsoft.com/en-us/visualstudio/ide/reference/options-text-editor-c-cpp-formatting?view=vs-2022
  * at Visual Studio Code, you can install an extension named "clang-format".
* ensure you have a python (version 3.6+), and then
  copy ${repo}/scripts/pre-commit to ${repo}/.git/hooks

Now, you can modify the code freely.