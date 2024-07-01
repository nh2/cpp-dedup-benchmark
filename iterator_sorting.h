#ifndef ITERATOR_SORTING_H
#define ITERATOR_SORTING_H

// Algorithms for dealing with containers by sorting arrays of their iterators
// (for example, sorting vectors of indices for `std::vector`).
//
// Provides fast operations for:
//
// * Duplicate removal:
//   * `stable_uniquify()`
//
// Based on:
// https://stackoverflow.com/questions/12200486/how-to-remove-duplicates-from-unsorted-stdvector-while-keeping-the-original-or/15761097#15761097
//
// This provides better performance and lower memory usage than
// using a (hash) set for keeping track of elements,
// when there is only a low number of duplicates in the input.
//
// This is because a sorting vectors has good cache locality,
// while (hash) sets require random memory access for each element.
//
// In the case the input consists mostly of duplicates, using a (hash) set can
// be faster, especially when the set can fit into a fast CPU cache.

#include <algorithm>
#include <vector>

namespace iterator_sorting {

// Returns an array of iterators that would sort the pointed-to values (unstable sort).
//
// Complexity:
// Given `N` as `last - first`:
// * Same as `std::sort` for N elements
template <
  typename It,
  typename Compare = std::less<>,
  typename EqualPred = std::equal_to<>,
  typename Allocator = std::allocator<It>
>
std::vector<It>
unstable_unique_iterators(
  const It begin,
  const It end,
  Compare comp = Compare{},
  EqualPred equalPred = EqualPred{}
)
{
  // Create vector of iterators.
  std::vector<It, Allocator> v;
  v.reserve(static_cast<size_t>(std::distance(begin, end)));
  for (It it = begin; it != end; ++it)
    v.push_back(it);

  // Sort vector of iterators so that their pointed-to values are in order.
  std::sort(v.begin(), v.end(), [&comp](const It & a, const It &b ){ return comp(*a, *b); });
  // Remove from vector of iterators subsequent ones that point to equal values.
  v.erase(std::unique(v.begin(), v.end(), [&equalPred](const It & a, const It & b) { return equalPred(*a, *b); }), v.end());
  // Sort vector of iterators back. Its pointed-to values are now non-duplicates.
  std::sort(v.begin(), v.end());
  return v;
}

// Returns an array of iterators that would stable-sort the pointed-to values.
//
// Complexity:
// Given `N` as `last - first`:
// * Same as `std::stable_sort` for N elements
template <
  typename It,
  typename Compare = std::less<>,
  typename EqualPred = std::equal_to<>,
  typename Allocator = std::allocator<It>
>
std::vector<It>
stable_unique_iterators(
  const It begin,
  const It end,
  Compare comp = Compare{},
  EqualPred equalPred = EqualPred{}
)
{
  // Create vector of iterators.
  std::vector<It, Allocator> v;
  v.reserve(static_cast<size_t>(std::distance(begin, end)));
  for (It it = begin; it != end; ++it)
    v.push_back(it);

  // Sort vector of iterators so that their pointed-to values are in order.
  std::stable_sort(v.begin(), v.end(), [&comp](const It & a, const It &b ){ return comp(*a, *b); });
  // Remove from vector of iterators subsequent ones that point to equal values.
  v.erase(std::unique(v.begin(), v.end(), [&equalPred](const It & a, const It & b) { return equalPred(*a, *b); }), v.end());
  // Sort vector of iterators back. Its pointed-to values are now non-duplicates in their original order.
  std::sort(v.begin(), v.end());
  return v;
}

// Partitions the range `[begin, end)` into two groups: Unique elements, and duplicates.
// Returns an iterator `uniqueRegionEnd` such the two groups are
// `[begin, uniqueRegionEnd)` and `[uniqueRegionEnd, end)`.
// Preserves stable order.
//
// Complexity:
// Given `N` as `last - first`:
// * Same as `std::stable_sort` for N elements
// * O(N) additional memory for iterators
template <
  typename It,
  typename Compare = std::less<>,
  typename EqualPred = std::equal_to<>,
  typename Allocator = std::allocator<It>
>
It
stable_uniquify(
  const It begin,
  const It end,
  Compare comp = Compare{},
  EqualPred equalPred = EqualPred{}
)
{
  const std::vector<It> uniqIts = stable_unique_iterators(begin, end, comp, equalPred);

  // Apply the order of uniqIts to the underlying container:
  // For each of the container's iterators, swap its pointed-to value
  // to the end of the uniq'ed region the iterator is the next unique one.
  It uniqueRegionEnd = begin; // Everthing until here has already been unique-swapped.
  size_t j = 0;
  for (It it = begin; it != end && j != uniqIts.size(); ++it) {
    if (it == uniqIts[j]) {
      std::iter_swap(it, uniqueRegionEnd);
      ++j;
      ++uniqueRegionEnd;
    }
  }
  return uniqueRegionEnd;
}

// Removes duplicate elements form a vector. Preserves stable order.
//
// Complexity:
// Given `N` as `last - first`:
// * Same as `std::stable_sort` for N elements
// * O(N) additional memory for iterators
template <typename T, typename IteratorAllocator = std::allocator<typename std::vector<T>::iterator>>
std::vector<T>::iterator
stable_uniquify(std::vector<T> & v)
{
  return v.erase(
    stable_uniquify<typename std::vector<T>::iterator, IteratorAllocator>(
      v.begin(),
      v.end()
    ),
    v.end()
  );
}

} // namespace

#endif // ITERATOR_SORTING_H
