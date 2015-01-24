# Mitos

Mitos is a library and a tool for collecting sampled memory
performance data to view with
[MemAxes](https://github.com/scalability-llnl/MemAxes)

----

# Quick Start

## Requirements

Mitos requires:

* A Linux kernel with perf_events support for memory
  sampling.  This originated in the 3.10 Linux kernel, but is backported
  to some versions of [RHEL6.6](https://www.redhat.com/promo/Red_Hat_Enterprise_Linux6/).

* [Dyninst](http://www.dyninst.org) version 8.2 or higher.

## Building

1. Make sure that Dyninst is installed and its location is added to the
   `CMAKE_PREFIX_PATH` environment variable.

2. Run the following commands from the root of the MemAxes source:
   ```
   mkdir build && cd build
   cmake -DCMAKE_INSTALL_PREFIX=/path/to/install/location ..
   make
   make install
   ```

## Running

1. Find the `mitosrun` command in the `bin` directory in the install
   directory.

2. Run any binary with `mitosrun` like this to generate a `.csv` file
   full of memory samples.  For example:

   ```
   mitosrun ls -la
   ```

   The above command will run the ls command and will output a
   `samples.out` file containing memory samples.


   `mitosrun` can also be fine-tuned with the following parameters:

   ```
   Usage:
   ./mitosrun [options] <cmd> [args]
       [options]:
           -o filename (default samples.out)
           -b sample buffer size (default 4096)
           -p sample period (default 4000)
           -t sample latency threshold (default 10)
       <cmd>: command to sample on (required)
       [args]: command arguments
   ```

3. Run `mitosprocess` on the memory samples file, supplying a binary 
   with debug information (compiled with -g) to generate a new output
   file with source and line information. For example:

   ```
   mitosprocess samples.out ./a.out
   ```

   Where `a.out` is some binary with debug information and `samples.out`
   was obtained by executing `mitosrun ./a.out`. The above command
   will produce a new file, `processed_samples.out`, which contains
   the same samples plus source files and line numbers
   associated with each sample.

   `mitosprocess` also accepts the following parameters:

   ```
   Usage:
   ./mitosprocess [options] <sample_file> <debug_binary>
       [options]:
           -o filename (default processed_<sample_file>)
       <sample_file>: a csv file created using mitosrun
       <debug_binary>: the binary executed with mitosrun (must contain debug symbols to be useful)
   ```

# Authors

Mitos and MemAxes were written by Alfredo Gimenez.

# License

Mitos is released as part of MemAxes under an LGPL license. For more
details see the LICENSE file.

`LLNL-CODE-663358`
