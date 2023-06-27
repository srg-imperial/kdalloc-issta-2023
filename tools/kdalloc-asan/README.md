# AddressSanitizer with KDAlloc Support

A recent addition to `klee-replay` allows the use of KLEE's new deterministic allocator (KDAlloc) during replay.
Now, runs outside of KLEE can use the heap layout produced by KDAlloc.
However, a common use case of such replays is to independently check errors reported by KLEE using AddressSanitizer (ASan).
Due to ASan always using its internal allocator, this use case cannot be easily covered by `klee-replay` alone.
Instead, we provide a (protype) patch for ASan, which changes its internal allocator to KDAlloc.
The patched version combines ASan's error detection (based on its shadow memory) with KDAlloc's memory layout, redzones and quarantine.


## Build Instructions

Our patch was developed using LLVM version 14.0.6.
For other versions modifications of varying extends may be necessary to apply the patch.

First, KLEE has to be compiled to produce the static library placed under `lib/libKDAllocAsan.a` in its build tree.
Then, we can patch and build `compiler-rt` (containing ASan) separately.
In the following, we assume that `$KLEE` and `$KLEE_BUILD` contain the absolute path to KLEE's source and build tree, respectively.

```bash
# sparse clone only the neccessary files for building the compiler-rt runtime library from LLVM
$ git clone --filter=blob:none --no-checkout --depth 1 --sparse -b llvmorg-14.0.6 git@github.com:llvm/llvm-project.git kdalloc-asan
$ cd kdalloc-asan
$ git sparse-checkout add cmake compiler-rt llvm/cmake
$ git checkout

# patch ASan
$ git apply < "$KLEE/tools/kdalloc-asan/asan-llvm14.0.6.patch"

# build patched version
$ mkdir build
$ cd build
$ cmake -GNinja \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCOMPILER_RT_DEFAULT_TARGET_TRIPLE=$(clang -dumpmachine) \
    -DKLEE_INCLUDE_DIR="$KLEE/include" \
    -DKDALLOC_ASAN_LIB_DIR="$KLEE_BUILD/lib" \
    ../compiler-rt
$ cmake --build .
```


## Usage

In the following, we assume `$BUILD` contains the absolute path to the compiler-rt build tree.

```bash
$ clang -fsanitize=address -shared-libsan -g -o test test.c
$ LD_LIBRARY_PATH="$BUILD/lib/linux/" KDALLOC_HEAP_START_ADDRESS=0x3f0000000000 ./test
# ...
==2450872==Using KDALLOC_HEAP_START_ADDRESS: 0x3f0000000000
# ...
```

The patched ASan version respects the following environment variables:
* `KDALLOC_HEAP_START_ADDRESS`
* `KDALLOC_HEAP_SIZE`
* `KDALLOC_QUARANTINE`


## Known Limitations

Due to the prototype nature of our patch, there are several known limitiations:

* Only shared-library runtime is supported; use `-shared-libsan`!
* Some standard ASan settings are ignored (e.g. `quarantine_size_mb`); use `KDALLOC_` environment variables instead!
* KDAlloc (huge) redzones are only partially poisoned (but we poison at least as much as vanilla ASan would)
* No multithreading support
