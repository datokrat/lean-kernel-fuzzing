# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/paul/coding/lean/lean4/build/release/mimalloc/src/mimalloc")
  file(MAKE_DIRECTORY "/home/paul/coding/lean/lean4/build/release/mimalloc/src/mimalloc")
endif()
file(MAKE_DIRECTORY
  "/home/paul/coding/lean/lean4/build/release/mimalloc/src/mimalloc-build"
  "/home/paul/coding/lean/lean4/build/release/mimalloc"
  "/home/paul/coding/lean/lean4/build/release/mimalloc/tmp"
  "/home/paul/coding/lean/lean4/build/release/mimalloc/src/mimalloc-stamp"
  "/home/paul/coding/lean/lean4/build/release/mimalloc/src"
  "/home/paul/coding/lean/lean4/build/release/mimalloc/src/mimalloc-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/paul/coding/lean/lean4/build/release/mimalloc/src/mimalloc-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/paul/coding/lean/lean4/build/release/mimalloc/src/mimalloc-stamp${cfgdir}") # cfgdir has leading slash
endif()
