# runb2

## Intention

Make it easy to manage boost builds with b2

## Building with your own dependencies

By default, this project uses Hunter to find and build required dependencies.

If you would like cmake to find your own dependencies, simply set the following command line 
option on the initial invocation of `cmake`:

`-DHUNTER_ENABLED=OFF`

Hunter documentation is here:

https://github.com/cpp-pm/hunter

## Required toolchain

This program requires a c++17 or better toolchain.

Some very good toolchain files available here:

https://github.com/ruslo/polly

in which case you can invoke cmake with:

`cmake -DCMAKE_TOOLCHAIN_FILE=<POLLY_DIR>/cxx17.cmake -H<SRC_DIR> -B<BUILD_DIR>`

Where:

* `POLLY_DIR` is the cloned polly repo
* `SRC_DIR` is the directory containing this file
* `BUILD_DIR` is the intended build directory (in-source builds are evil)
