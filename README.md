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

1. Find the `mitosrun` and `mitosmpirun` commands in the `bin` directory in the install
   directory.

2. Run any non-MPI binary with `mitosrun` like this to generate a `.csv` file
   full of memory samples.  For example:

   ```
   mitosrun ./examples/matmul
   ```

   The above command will run the ls command and will output a
   `samples.out` file containing memory samples.

   For MPI binaries, use `mitosmpirun` e.g.:

   ```
   srun -N2 ./mitosrun examples/mpi_matmul
   ```

   Both `mitosrun` and `mitosmpirun` can also be fine-tuned with the following parameters:

   ```
   [options]:
       -o filename (default samples.out or MPI host name)
       -b sample buffer size (default 4096)
       -p sample period (default 4000)
       -t sample latency threshold (default 10)
   ```

3. Run `mitosprocess` on the memory samples file, supplying a binary 
   with debug information (compiled with -g) to generate a new output
   file with source and line information. For example:

   ```
   mitosprocess samples.out ./matmul
   ```

   The above command will produce a new file, `processed_samples.out`, 
   which contains the same samples plus source files and line numbers
   associated with each sample.

# Authors

Mitos and MemAxes were written by Alfredo Gimenez. 

Thanks to Todd Gamblin for suggestions and for giving Mitos a proper build setup.

# License

Mitos is released as part of MemAxes under an LGPL license. For more
details see the LICENSE file.

`LLNL-CODE-663358`
