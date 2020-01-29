# runb2

## Intention

Make it easy to manage boost builds with b2

Running b2 can be a pain. If you have custom options, build, stage or prefix directories then you have to remember
all this every time you run b2 on the command line.

This program will:

* find b2 in your directory hierarchy
* optionally cd to the b2 directory
* optionally load your b2 command line options from a named cache
* optionally save your b2 command line options to a named cache
* respect any extra b2 options you supply on the command line

You can load options from one cache, add more options and save to another in one invocation.

Usage:

```bash
runb2 [options...] [options to pass to b2]

Valid Options:
  --load arg            load the given b2 command line from slot <string> and 
                        execute
  --store arg           store the given b2 command line in slot <string> and 
                        execute
  --nocd                normally the current working directory will be set to 
                        the directory where b2 was found. Setting this option 
                        disables this.
  --noexec              set to prevent the launch of b2
  --help                show this help
```

Command line options are saved in a hidden directory in your home directory called `.runb2`

Example of use:

```bash
# create a template called basic but don't run b2
runb2 --save=basic --noexec --with-program_options

# load the basic template, add more options, save to cxx17
runb2 --load=basic --save=cxx17 --noexec cxxstd=17 prefix=${HOME}/installs/cxx17

# load the cxx17 template, add to the command line and execute
runb2 --load=cxx17 build
runb2 --load=cxx17 install
```

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
