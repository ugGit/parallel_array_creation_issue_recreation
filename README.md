# nvc++: Create arrays on GPU and use them on CPU

Clone the repo:

```
git clone git@github.com:ugGit/parallel_array_creation_issue_recreation.git
```

## Requirements
The issue has been encountered in an environment using the following modules, which must be available during the compilation:

* gcc/11.2.0
* nvhpc/22.5 (using CUDA 11.7)

## Information about the setup
The code has been tested on an Nvidia A6000 and a GeForce RTX 2080.

This project contains the minimal required modules and code to recreate the issue encountered.

## Issue description
Through a loop, that uses a parallel execution policy, multiple computations are offloaded to the GPU and performed in parallel.
The output of each computation is stored at a given index in a beforehand initialized output array. 
However, each result is itself an array of structs of which the number of elements `N` in reality is depending on input data, and therefore not known before the execution of the computation.
To work around this limitation, we dynamically create the array for each result within the parallelized loop ([line 33](https://github.com/ugGit/parallel_array_creation_issue_recreation/blob/main/main.cpp#L33)), and then store the pointer in the output array ([line 37](https://github.com/ugGit/parallel_array_creation_issue_recreation/blob/main/main.cpp#L37)).

While this works if the code is executed sequentially, the parallel execution policy seems to impact the memory management (probably moving to/from Unified Memory).
The created result array is accessible from within the parallelized loop, but not from the outside.

## Execute the code
```
nvc++ -stdpar=gpu -o main main.cpp && ./main
```

Resulting output:
```
Pointed to addresses after initialization outside the loop
1a) &(nested_arrays[0][1] = 0x7f69ba000108
1b) &(nested_arrays[0][2] = 0x7f69ba000110
--------
Initial value acces verification: 
before) nested_arrays[0][1] = 1
after)  nested_arrays[0][1] = 99
--------
Reassign the pointer within the parallel loop
2a) &(nested_arrays[0][1] = 0x7f69ba000108
3a) &(nested_arrays[0][1] = 0x7f69b93ff928
--------
Intermediate value acces verification: 
before) nested_arrays[0][1] = 1
after)  nested_arrays[0][1] = 77
--------
Pointed to addresses after reassignation outside the loop
4a) &(nested_arrays[0][1] = 0x7f69b93ff928
4b) &(nested_arrays[0][2] = 0x7f69b93ff930
--------
Final value acces verification: 
Segmentation fault (core dumped)
```

## Observations

### Compilation
The following list resumes the changes made that allow to run the program successful but not on the GPU:
* Changing the execution policy to `std::execution::seq` in main.cpp (removing the policy works as well).
* Compile for multicore CPU `-stdpar=multicore`.
* Compile without stdpar `-nostdpar`.

And here things that did not help:
* Appending `-gpu=managed:intercept` during compilation.
* Using nvhpc 22.3 for compilation instead of 22.5.
* Applying the flags `-g`, `-O0`, `-O1`, `-O2`, `-O3` during compilation.


### Memory
Another investigation undertaken is the analysis of the storage location for the result arrays, which varies depending on the target.
Hereafter, a condensed output of the execution for each target is given:

Sequential execution on CPU (successful):
```
2a) &(nested_arrays[0][1] = 0x7efd12000108
3a) &(nested_arrays[0][1] = 0x7efd12000088
```

Parallel execution on Multicore CPU (successful):
```
2a) &(nested_arrays[0][1] = 0x15f6c48
3a) &(nested_arrays[0][1] = 0x15f7368
```

Parallel execution on GPU (fails):
```
2a) &(nested_arrays[0][1] = 0x7f3a9a000108
3a) &(nested_arrays[0][1] = 0x7f3a993ff928
```

The distance in memory between the original address (2a) and the new address allocated within the loop (3a) seems to be closer for the sequential and multicore version than for the GPU version.
This raises the question if the successful versions also should fail but don't because the newly allocated address falls within the accessible space of the function embodying the loop.
