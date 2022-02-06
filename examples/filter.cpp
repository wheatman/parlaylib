#include <iostream>
#include <string>

#include <parlay/delayed.h>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/utilities.h>

namespace delayed = parlay::delayed;

// **************************************************************
// An implementation of filter with other primitives.
// Makes use of delayed sequences and should be competitive with the
// built-in filter.
// It accepts a delayed sequence as an argument.
// Makes use of an uninitialized sequence, which requires the use
// of assign_uninitialized(dest, src) to write to.
// Could use regular sequence and = for assignment, but would
// be more costly.
// ************************************************************** 

template<typename Range, typename UnaryPred>
auto filter(const Range& A, const UnaryPred&& f) {
  long n = A.size();
  using T = typename Range::value_type;
  auto flags = parlay::delayed_map(A, [&] (const T& x) {
		 return (long) f(x);});
  auto [offsets, sum] = delayed::scan(flags);
  auto r = parlay::sequence<T>::uninitialized(sum);

  delayed::apply(delayed::enumerate(offsets), [&] (auto&& x) {
    auto [i, offset] = x;
    if (flags[i]) parlay::assign_uninitialized(r[offset], A[i]); });

  return r;
}

// **************************************************************
// Driver code
// **************************************************************
int main(int argc, char* argv[]) {
  auto usage = "Usage: filter <n>";
  if (argc != 2) std::cout << usage << std::endl;
  else {
    long n;
    try { n = std::stol(argv[1]); }
    catch (...) { std::cout << usage << std::endl; return 1; }
    auto result = filter(parlay::iota(n+1), [] (long i) { return i%2 == 0; });
    std::cout << "number of even integers up to n: " << result.size() << std::endl;
  }
}
