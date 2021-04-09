# CMake-gitpack

[![](https://img.shields.io/github/issues-raw/RWTH-HPC/CMake-gitpack.svg?style=flat-square)](https://github.com/RWTH-HPC/CMake-gitpack/issues)
[![](https://img.shields.io/badge/license-BSD--3--clause-blue.svg?style=flat-square)](LICENSE)
![CMake 2.6 required](http://img.shields.io/badge/CMake_required-2.6-lightgrey.svg?style=flat-square)

CMake module to enhance CPack with ignore patterns from `.gitattributes`.


## Usage

To use [GitPack.cmake](cmake/GitPack.cmake), simply add this repository as git
submodule into your own repository
```Shell
mkdir externals
git submodule add git://github.com/RWTH-HPC/CMake-gitpack.git externals/CMake-gitpack
```
and add `externals/cmake-gitpack/cmake` to your `CMAKE_MODULE_PATH`:
```CMake
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/externals/cmake-gitpack/cmake" ${CMAKE_MODULE_PATH})
```

If you don't use git or dislike submodules, just copy the
[GitPack.cmake](cmake/GitPack.cmake) file into your repository.

Now, simply switch from `include(CPack)` to `include(GitPack)` to automatically
use ignore patterns from `.gitignores` in CPack. This also covers submodules.


## Contribute

Anyone is welcome to contribute. Simply fork this repository, make your changes
**in an own branch** and create a pull-request for your changes. Please submit
one change per pull-request only.

You found a bug? Please fill out an
[issue](https://github.com/RWTH-HPC/CMake-gitpack/issues) and include all data
to reproduce the bug.


#### LICENSE

CMake-gitpack is licensed under a 3-clause-BSD license. See the
[LICENSE file](LICENSE) for details.

&copy; 2018 RWTH Aachen University, Federal Republic of Germany
