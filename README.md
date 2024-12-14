# tomsolver

![workflow](https://github.com/tomwillow/tomsolver/actions/workflows/cmake-multi-platform.yml/badge.svg)

[中文](https://github.com/tomwillow/tomsolver/blob/master/README-zh.md) [English](https://github.com/tomwillow/tomsolver)

Simplest, Well-tested, Non-linear equations solver library by C++14.

origin: https://github.com/tomwillow/tomsolver

> Make C++ solve nonlinear equations as easy as Matlab fsolve

**Contributors:**

- Tom Willow (https://github.com/tomwillow)
- lizho (https://github.com/lizho)

# Features

- Simple! Simple! If you know how to use fsolve, you will use this!
- Single header file, just include it!

# Functions

- Solving nonlinear equations (Newton-Raphson method, LM method)
- Solving linear equations (Gaussian-column pivot iteration method, inverse matrix)
- Matrix and vector operations (matrix inversion, vector cross multiplication, etc.)
- "Pseudo" symbolic operations (derivatives of expressions, Jacobian matrices of symbolic matrices)

# Supported Platforms

Tested at:

- Linux: ubuntu 22.04 x86_64 gcc 11.3.0
- Windows: windows10 x64 Visual Studio 2019

Tested at Github Actions:

- Linux-latest gcc Debug&Release
- Linux-latest clang Debug&Release
- Windows-latest msvc Debug&Release

# Example

```C++
#include <tomsolver/tomsolver.h>

using namespace tomsolver;

int main() {
    /*
    Translate from Matlab code:

    root2d.m:
        function F = root2d(x)
            F(1) = exp(-exp(-(x(1)+x(2)))) - x(2)*(1+x(1)^2);
            F(2) = x(1)*cos(x(2)) + x(2)*sin(x(1)) - 0.5;
        end

    root2d_solve.m:
        format long
        fun = @root2d;
        x0 = [0,0];
        x = fsolve(fun,x0)

    result:
        x =

            0.353246561920553   0.606082026502285
     */
    try {
        // create equations from string
        SymVec f = {
            "exp(-exp(-(x1 + x2))) - x2 * (1 + x1 ^ 2)"_f,
            "x1 * cos(x2) + x2 * sin(x1) - 0.5"_f,
        };

        // solve it!
        VarsTable ans = Solve(f);

        // print the solution
        std::cout << ans << std::endl;

        // get the values of solution
        std::cout << "x1 = " << ans["x1"] << std::endl;
        std::cout << "x2 = " << ans["x2"] << std::endl;
    } catch (const std::runtime_error &err) {
        // if any error occurs, exception will be thrown
        std::cerr << err.what() << std::endl;
        return -1;
    }

    return 0;
}
```

# Usage

## 1. Header-Only usage

Just include a single header file:
`single/include/tomsolver/tomsolver.h`

## 2. VCPKG

You could use [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/overview) for unified package usage.

* Manifest mode(recommended)
  Create a `vcpkg.json` file:

  ```
  {
    "dependencies": [
      "tomsolver"
    ]
  }
  ```

  Then run the following command in the same directory:
  `$ vcpkg install`
  Tutorial: [https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash)
* Classic mode
  `$ vcpkg install tomsolver`

## 2. Binary Library + Header Files usage

```bash
$ git clone https://github.com/tomwillow/tomsolver
$ mkdir build
$ cd build
$ cmake ../tomsolver
$ cmake --build . --target INSTALL
```

Then add the include directory and link to the library file.

# Directory Structure

- **src**: source files
- **tests**: unit tests
- **single/include**: the folder where the header-only tomsolver.h is located
- **single/test**: All unit tests are integrated into one .cpp file to test whether tomsolver.h is correct.
- **scripts**: used to generate single-file header files and single-file tests under single

### examples

- **examples/solve**: Example of solving nonlinear equations, demonstrating basic usage and how to set a unified initial value
- **examples/solve2**: Example of solving nonlinear equations, demonstrating how to switch solution methods and replace known quantities in the equation
- **examples/diff_machine**: Derivator, input a line of expression and output the derivation result of this expression

# Development Plan

- add doxygen comments + tutorial document (CN+EN)
- add benchmark tests
- add an option to use Eigen library as matrix library
- aim at Matlab fsolve, add more solving methods of nonlinear equations
- add an optional Config parameter in Solve() function
  (similar to Matlab fsolve's option)
- add support for binary/multivariate functions, such as pow(x, y)
- the current Simplify function is still very simple, modify Simplify to be better
- add LaTeX format formula output

# Thanks

https://github.com/taehwan642

# WeChat group

If you have any questions, want to communicate, want to participate in development, or want to contact the authors, you are welcome to add the WeChat _tomwillow_.

If you think this repository is good, please give it a star!
