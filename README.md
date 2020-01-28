# beast-softphone-problem

## Intention

To recreate the problem reported here:

https://github.com/boostorg/beast/issues/1798

## Required toolchain

This program requires a c++11 or better toolchain.

Some very good toolchains files available here:

https://github.com/ruslo/polly

in which case you can invoke cmake with:

`cmake -DCMAKE_TOOLCHAIN_FILE=<POLLY_DIR>/cxx17.cmake -H<SRC_DIR> -B<BUILD_DIR>`

Where:

* `POLLY_DIR` is the cloned polly repo
* `SRC_DIR` is the directory containing this file
* `BUILD_DIR` is the intended build directory (in-source builds are evil)
