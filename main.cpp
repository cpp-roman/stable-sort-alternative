#include <algorithm>
#include <string>

#include "src/recursive_stable_sort.h"
#include "src/test_utils.h"

#ifdef _WIN32 // Only for Windows, to output non-ascii characters in the terminal
#include <windows.h>
#endif

const std::string sort_fn_name = "recursive_stable_sort";
auto sort_fn = [](std::vector<Record> &data, Record *buffer) {
  recursive_stable_sort(data.data(), data.size(), buffer, KeyCompare{});
};

// ---- Main ----
int main() {

#ifdef _WIN32 // Only for Windows, to output non-ascii characters in the terminal
  SetConsoleOutputCP(CP_UTF8);
#endif

  std::vector<std::string> patterns = {"random", "sorted", "reversed", "duplicates",
                                       "almost_sorted"};

  int total_failures = 0;
  int total_tests = 0;

  // --- Run stability test ---
  total_tests++;
  if (!run_stability_test(sort_fn)) {
    total_failures++;
  }

  // --- Run pattern-based benchmarks ---
  for (const auto &pattern : patterns) {
    for (size_t size : {10, 100, 1'000, 10'000, 100'000, 1'000'000}) {
      total_tests++;
      bool ok = run_test_case(pattern, size, sort_fn, sort_fn_name);
      if (!ok) {
        total_failures++;
      }
    }
  }

  int total_passes = total_tests - total_failures;

  if (total_failures == 0) {
    std::cout << "✅✅✅✅✅ All " << total_tests << " tests passed.\n";
  } else {
    std::cout << "❌❌❌❌❌ " << total_passes << "/" << total_tests << " tests passed. "
              << total_failures << " tests failed.\n";
  }

  return total_failures == 0 ? 0 : 1;
}
