#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <tuple>
#include <unordered_set>
#include <variant>
#include <vector>

#include "hash_tuple.h"
#include "iterator_sorting.h"

#if __has_include(<parallel_hashmap/phmap.h>)
#include <parallel_hashmap/phmap.h>
#define HAVE_DEPENDENCY_PHMAP
#endif


using namespace std;

using Color = tuple<unsigned char, unsigned char, unsigned char>;
using Position = tuple<double, double, double>;
using Point3D = tuple<Position, Color>;


void
benchmarkUniquify(size_t n)
{
  cout << "Initing vector, n = " << ((double) n) << " ..." << endl;
  vector<Point3D> inputCloud(n);
  {
    cout << "Generating vector..." << endl;
    std::generate(inputCloud.begin(), inputCloud.end(), [n = 0.0] () mutable -> Point3D { n += 0.00000001; return {{n, 0, 0}, {}}; });
  }

  double duration_stable_unique_iterators;
  double duration_stable_unique_iterators_whole;
  double duration_unstable_unique_iterators;
  double duration_direct_vector_stable_sort;
  double duration_direct_vector_stable_sort_whole;
  double duration_direct_vector_unstable_sort;
  double duration_direct_vector_unstable_sort_whole;
  double duration_unordered_set;
#ifdef HAVE_DEPENDENCY_PHMAP
  double duration_flat_hash_set;
#endif

  const auto posLess = [](const Point3D & a, const Point3D & b) { return get<0>(a) < get<0>(b); };
  const auto posEqual = [](const Point3D & a, const Point3D & b) { return get<0>(a) == get<0>(b); };

  // stable_unique_iterators()
  {
    vector<Point3D> v = inputCloud; // copy
    cout << "stable_unique_iterators..." << endl;
    const auto t0 = chrono::steady_clock::now();
    // Only compare point positions.
    const size_t numUniques = iterator_sorting::stable_unique_iterators(v.begin(), v.end(), posLess, posEqual).size();
    const auto t1 = chrono::steady_clock::now();
    duration_stable_unique_iterators = chrono::duration<double>(t1 - t0).count();
    cout << "stable_unique_iterators done, got " << numUniques << " uniques" << endl;
  }

  // stable_unique_iterators(), comparing whole `Point3D`
  {
    vector<Point3D> v = inputCloud; // copy
    cout << "stable_unique_iterators_whole..." << endl;
    const auto t0 = chrono::steady_clock::now();
    // Only compare point positions.
    const size_t numUniques = iterator_sorting::stable_unique_iterators(v.begin(), v.end()).size();
    const auto t1 = chrono::steady_clock::now();
    duration_stable_unique_iterators_whole = chrono::duration<double>(t1 - t0).count();
    cout << "stable_unique_iterators_whole done, got " << numUniques << " uniques" << endl;
  }

  // unstable_unique_iterators()
  {
    vector<Point3D> v = inputCloud; // copy
    cout << "unstable_unique_iterators..." << endl;
    const auto t0 = chrono::steady_clock::now();
    // Only compare point positions.
    const size_t numUniques = iterator_sorting::unstable_unique_iterators(v.begin(), v.end(), posLess, posEqual).size();
    const auto t1 = chrono::steady_clock::now();
    duration_unstable_unique_iterators = chrono::duration<double>(t1 - t0).count();
    cout << "unstable_unique_iterators done, got " << numUniques << " uniques" << endl;
  }

  // direct element stable sorting (no indices)
  {
    vector<Point3D> v = inputCloud; // copy
    cout << "direct_vector_stable_sort..." << endl;
    const auto t0 = chrono::steady_clock::now();
    std::stable_sort(v.begin(), v.end(), posLess);
    v.erase(unique(v.begin(), v.end(), posEqual), v.end());
    const size_t numUniques = v.size();
    const auto t1 = chrono::steady_clock::now();
    duration_direct_vector_stable_sort = chrono::duration<double>(t1 - t0).count();
    cout << "direct_vector_stable_sort done, got " << numUniques << " uniques" << endl;
  }

  // direct element stable sorting (no indices), whole points
  {
    vector<Point3D> v = inputCloud; // copy
    cout << "direct_vector_stable_sort_whole..." << endl;
    const auto t0 = chrono::steady_clock::now();
    std::stable_sort(v.begin(), v.end());
    v.erase(unique(v.begin(), v.end()), v.end());
    const size_t numUniques = v.size();
    const auto t1 = chrono::steady_clock::now();
    duration_direct_vector_stable_sort_whole = chrono::duration<double>(t1 - t0).count();
    cout << "direct_vector_stable_sort_whole done, got " << numUniques << " uniques" << endl;
  }

  // direct element unstable sorting (no indices)
  {
    vector<Point3D> v = inputCloud; // copy
    cout << "direct_vector_unstable_sort..." << endl;
    const auto t0 = chrono::steady_clock::now();
    std::sort(v.begin(), v.end(), posLess);
    v.erase(unique(v.begin(), v.end(), posEqual), v.end());
    const size_t numUniques = v.size();
    const auto t1 = chrono::steady_clock::now();
    duration_direct_vector_unstable_sort = chrono::duration<double>(t1 - t0).count();
    cout << "direct_vector_unstable_sort done, got " << numUniques << " uniques" << endl;
  }

  // direct element unstable sorting (no indices), whole points
  {
    vector<Point3D> v = inputCloud; // copy
    cout << "direct_vector_unstable_sort_whole..." << endl;
    const auto t0 = chrono::steady_clock::now();
    std::sort(v.begin(), v.end());
    v.erase(unique(v.begin(), v.end()), v.end());
    const size_t numUniques = v.size();
    const auto t1 = chrono::steady_clock::now();
    duration_direct_vector_unstable_sort_whole = chrono::duration<double>(t1 - t0).count();
    cout << "direct_vector_unstable_sort_whole done, got " << numUniques << " uniques" << endl;
  }

  // std::unordered_set
  {
    vector<Point3D> v = inputCloud; // copy
    cout << "std::unordered_set-uniqueing vector..." << endl;
    const auto t0 = chrono::steady_clock::now();

    unordered_set<Position, hash_tuple::hash<Position>> seenPositions;
    seenPositions.reserve(v.size());
    v.erase(std::remove_if(v.begin(), v.end(),
      [&seenPositions](const Point3D & point)
      {
        const auto & pos = get<0>(point); // Only compare point positions.
        return !seenPositions.insert(pos).second; // insert().second is false if the value couldn't be inserted (is a duplicate)
      }),
      v.end()
    );
    const size_t numUniques = seenPositions.size();
    const auto t1 = chrono::steady_clock::now();
    duration_unordered_set = chrono::duration<double>(t1 - t0).count();
    cout << "std::unordered_set-uniqueing done, got " << numUniques << " uniques" << endl;
  }

#ifdef HAVE_DEPENDENCY_PHMAP
  // phmap::flat_hash_set
  {
    vector<Point3D> v = inputCloud; // copy
    cout << "phmap::flat_hash_set-uniqueing vector..." << endl;
    const auto t0 = chrono::steady_clock::now();

    phmap::flat_hash_set<Position> seenPositions;
    seenPositions.reserve(v.size());
    v.erase(std::remove_if(v.begin(), v.end(),
      [&seenPositions](const Point3D & point)
      {
        const auto & pos = get<0>(point); // Only compare point positions.
        return !seenPositions.insert(pos).second; // insert().second is false if the value couldn't be inserted (is a duplicate)
      }),
      v.end()
    );
    const size_t numUniques = seenPositions.size();
    const auto t1 = chrono::steady_clock::now();
    duration_flat_hash_set = chrono::duration<double>(t1 - t0).count();
    cout << "phmap::flat_hash_set-uniqueing done, got " << numUniques << " uniques" << endl;
  }
#endif

  const double ref = duration_stable_unique_iterators; // reference time against which we compute factors
  cout << "Timing:" << endl;
  cout << fixed << setprecision(2);
  cout << "  stable_unique_iterators:                         " << setw(7) << duration_stable_unique_iterators << " s" << endl;
  cout << "  stable_unique_iterators_whole:                   " << setw(7) << duration_stable_unique_iterators_whole << " s (" << (duration_stable_unique_iterators_whole / ref) << " x)" << endl;
  cout << "  unstable_unique_iterators:                       " << setw(7) << duration_unstable_unique_iterators << " s (" << (duration_unstable_unique_iterators / ref) << " x)" << endl;
  cout << "  direct_vector_stable_sort:                       " << setw(7) << duration_direct_vector_stable_sort << " s (" << (duration_direct_vector_stable_sort / ref) << " x)" << endl;
  cout << "  direct_vector_stable_sort_whole:                 " << setw(7) << duration_direct_vector_stable_sort_whole << " s (" << (duration_direct_vector_stable_sort_whole / ref) << " x)" << endl;
  cout << "  direct_vector_unstable_sort:                     " << setw(7) << duration_direct_vector_unstable_sort << " s (" << (duration_direct_vector_unstable_sort / ref) << " x)" << endl;
  cout << "  direct_vector_unstable_sort_whole:               " << setw(7) << duration_direct_vector_unstable_sort_whole << " s (" << (duration_direct_vector_unstable_sort_whole / ref) << " x)" << endl;
  cout << "  unordered_set:                                   " << setw(7) << duration_unordered_set << " s (" << (duration_unordered_set / ref) << " x)" << endl;
#ifdef HAVE_DEPENDENCY_PHMAP
  cout << "  flat_hash_set:                                   " << setw(7) << duration_flat_hash_set << " s (" << (duration_flat_hash_set / ref) << " x)" << endl;
#endif
}


void run_benchmark()
{
  for (double size = 1000; size <= 100 * 1000000; size *= sqrtl(10.0))
  {
    benchmarkUniquify( (size_t) round(size) );
    cout << endl;
  }
}


int main(int argc, char const *argv[])
{
  run_benchmark();
  return 0;
}
