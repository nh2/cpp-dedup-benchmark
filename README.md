# C++ vector deduplication by index sorting

This repo benchmarks ways to remove duplicates from a C++ container, such as `std::vector<T>`.

It shows that sorting an index vector can be much faster than hash sets:

* Baseline: Index sorting from [`iterator_sorting.h`](./iterator_sorting.h) (credits: [StackOverflow _user541686_](https://stackoverflow.com/questions/12200486/how-to-remove-duplicates-from-unsorted-stdvector-while-keeping-the-original-or/15761097#15761097))
* [`std::unordered_set`](https://en.cppreference.com/w/cpp/container/unordered_set): **5x** slower
* [`phmap::flat_hash_set`](https://github.com/greg7mdp/parallel-hashmap): **2x** slower

_Index-sorting deduplication_ is fast in the common case of:

* Only a small fraction of elements are duplicates.

But if the number of distinct elements small enough to fit into a CPU cache, using a hash set can be faster.


## API

[`iterator_sorting.h`](./iterator_sorting.h) provides:

```c++
vector<It> unstable_unique_iterators(It begin, It end);
vector<It> stable_unique_iterators(It begin, It end);
       It  stable_uniquify(It begin, It end);
vector<T>  stable_uniquify(vector<T> & v);
```

Note vector iterators are usually simply 64-bit indices.


## Terminology

* `T`: the types of elements inside the data structure to deduplicate (which can be e.g. `vector<T>`)
* _projection function_ `proj()`:
  * Often we need "sort-by" or "unique-by" operations, that consider only a part of `T`.
  * For example, for a coloured 3D point `( pos=(float x, y, z), color=(uint8_t r, g, b) )`, we may want to deduplicate only based on the `pos`.
  * This is called ["projection"](https://en.wikipedia.org/wiki/Projection_(mathematics)). In the above, `proj(T)` would be `T.pos`.
  * In the benchmarks,
    * `whole` means that the entire input type element is being compared (no projection).
    * None-`whole` means that only the part of interest is being compared, passing a `proj(T)` as e.g. [`std::stable_sort`'s `Compare comp`](https://en.cppreference.com/w/cpp/algorithm/stable_sort) or [`std::unique`'s `BinaryPred p`](https://en.cppreference.com/w/cpp/algorithm/unique).


## Memory usage

Measured:

```
what                      Bytes per element    Notes

Input data type (Point)      11                = 3 double + 3 char

stable_unique_iterators      12                = 1.5 size_t due to std::stable_sort (e.g. merge sort buffer)
unstable_unique_iterators     8                = 1 size_t
unordered_set                56
flat_hash_set                32

direct_vector_unstable_sort   0
direct_vector_stable_sort     1.5*sizeof(T)    T = input element type
```


## Running the benchmark

```
make
```

Dependencies reproducibly pinned with [Nix](https://nixos.org):

```sh
NIX_PATH=nixpkgs=https://github.com/NixOS/nixpkgs/archive/9c0ce522cab22ccaba5f89188d24ef5bb919d914.tar.gz nix-shell -p parallel-hashmap --pure --run make
```


## Results

On `Intel Core i7-7500U`, single-run benchmark (we only care about rough numbers):

```
           n   |--- speedup factor over ---|   notes
               unordered_set   flat_hash_map

        1000   2.3             0.5             flat_hash_map is faster
        3162   3.1             0.5             flat_hash_map is faster
       10000   1.9             0.3             flat_hash_map is faster
       31623   2.8             1.2
      100000   4.0             0.9             flat_hash_map is faster
      316228   3.7             1.1
     1000000   2.6             0.7             flat_hash_map is faster (last time)
     3162277   4.2             1.3
    10000000   4.0             1.8
    31622776   5.0             3.1
   100000000   4.8             2.7
```

![results graph](graph.svg)

Analysis:

*


### Performance vs non-stable algorithms

```
           n   |- speedup factor over -|   notes
               unstable_unique_iterators

        1000   0.6
        3162   1.2
       10000   1.0
       31623   1.1
      100000   1.2
      316228   1.0
     1000000   0.8
     3162277   0.9
    10000000   0.9
    31622776   0.9
   100000000   0.9
```


### Performance vs direct vector element sorting

```
                   |--------------------------- speedup factor over ----------------------|
                   |---------------------------- direct_vector_* -------------------------|
           n       stable_sort    stable_sort_whole    unstable_sort    unstable_sort_whole

        1000       0.5            0.5                  0.5              0.3
        3162       1.3            1.3                  0.6              0.8
       10000       1.2            1.1                  0.7              0.7
       31623       1.5            1.5                  0.7              1.2
      100000       1.6            1.5                  0.7              0.8
      316228       1.3            1.4                  0.5              0.6
     1000000       1.5            1.5                  0.6              0.6
     3162278       1.4            1.5                  0.6              0.7
    10000000       1.9            1.5                  1.0              1.0
    31622777       1.1            1.2                  0.5              0.6
   100000000       1.4            1.3                  0.6              0.6
```

Analysis:

* **Unstable direct sorts are 2x faster**.
* Stable direct sorts are not worth it, compared to stable direct sorts of indices.

I expect that as `sizeof(T)` while `sizeof(proj(T))` stays constant,  `direct_vector_unstable_sort` will lose its benefit over index-based sorting, because it needs to read and write more data at every step, O(n log(n)) times, while index-based sorting only needs to touch the whole `T` O(n) times.


### Summary

* Vector index sorting beats hash maps, unless the number of distinct elements is < 1 M.
* When `sizeof(T)` is small AND you don't need stable order, you can get ~2x faster (and saving memory), by using unstable inplace sort:
  ```c++
  std::sort(v.begin(), v.end(), posLess);
  v.erase(unique(v.begin(), v.end(), posEqual), v.end());
  ```
