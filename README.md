# PP-sum
**Report**

**Hand-Drawn Design Sketches (The Sketch is at the End)**

(At the beginning, -S and -M modes were considered, instead of designing normal mode first and modifying it later.)

**REVERSE Function Design**

The REVERSE function was designed based on the common practice of reversing a chained table. Both iteration and recursion were initially considered:

1. **Recursion**: This was excluded due to its high time and space complexity.
2. **Iteration**: Although it has a space complexity of O(1), it requires creating additional structure nodes, which increases memory usage. Hence, this approach was also discarded.

**General Processing Approach**

- Normal Mode (Initially Considered as -S Mode)

This approach reads all frames into memory at once, processes them, and writes the results to the output file. Initially classified as **-S** mode due to its reduced I/O operations and higher efficiency, it was later redefined as the **normal mode** because of its significant memory requirements. While suitable for small datasets, this method is less efficient for large-scale data processing.

- M Mode

This approach processes data in smaller portions. A portion of data (excluding metadata like frame count, height, width, and channels) is read, processed, and written back to the file. This reduces memory usage to O(portion size), but requires multiple read/write operations. Despite the additional I/O, tests showed it can outperform the normal mode in some scenarios.

-S Mode (Final Implementation: Optimized)

The final implementation enhances the normal mode by introducing OpenMP parallelization to improve speed. Multiple threads are used, and mp_get_wtime() replaces clock() for accurate timing. Although memory usage increases due to thread overhead, this approach balances speed and memory efficiency effectively.

![image.png](attachment:image.png)

**In different modes, memory and runtime**

· **Row** **1-4:** Normal mode (no -S or -M): process all data in the normal way, may have higher memory usage and longer runtimes.

- As expected, the average runtime is around **0.01 sec**, and memory usage is at **6500KB**. performance is high, but but when the amount of data is large, memory consumption and runtime may increase significantly, leading to a performance bottleneck.

· **Row 5-8:** —S (parallelisation with OpenMP): turn on OpenMP for parallelisation, expect lower runtimes, but may increase memory usage, depending on how parallelisation is implemented.

- Memory footprint: Due to the additional overheads introduced by parallelisation, the memory footprint has increased and varies between **7790KB** and **11000KB** depending on the task.

- Runtime: When measured using the **clock()** function, the runtime is much **higher** than expected, possibly related to the measurement method or system load. When measured using the **mp_get_wtime()** function, the runtime of most tasks is about **one-third shorter** than in normal mode, indicating that parallelisation improves performance to some extent.

· **Row 9-12:** —M (read and process in batches): read and process data in batches to reduce memory usage, may affect runtime as data needs to be processed in multiple batches.

- Memory footprint: Successfully reduced the memory footprint to about **a third** of the normal mode, or even lower (e.g. only **952KB** for the clip_channel task).

- Runtime: Runtime is **faster** than normal mode when measured with the **clock**() function, but **longer** than normal mode when measured with the **mp_get_wtime()** function.

Possible reasons for different results after testing with mp_get_wtime() function and clock() function:

- **clock**() is a CPU clock cycle-based timing method that calculates the actual CPU time consumed by the program (i.e., the slice of time the CPU spends during program execution). It does not take into account whether the program is waiting for an I/O operation, or whether it is running in multiple threads.
- However, **mp_get_wtime**() is a high-precision time function in the MPI (Message Passing Interface) designed to accurately measure the actual running time of a program. It does not depend on the CPU time slice, but measures the actual time from start to finish (including I/O operations, wait times, etc.).
