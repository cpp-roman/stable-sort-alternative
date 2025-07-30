#include <algorithm>
#include <string>

#include "src/my_stable_sort.h"
#include "src/test_utils.h"

#ifdef _WIN32 // windows-only - to output non-ascii chars
#include <windows.h>
#endif

// ---- Helpers to wrap std::stable_sort and my_stable_sort ----
const std::string sort1_name = "std::stable_sort";
auto sort1_fn = [](std::vector<Record> &data, Record *) {
  std::stable_sort(data.begin(), data.end(), KeyCompare{});
};

const std::string sort2_name = "my_stable_sort";
auto sort2_fn = [](std::vector<Record> &data, Record *buffer) {
  my_stable_sort(data.data(), data.size(), buffer, KeyCompare{});
};

// ---- Main ----
int main() {

#ifdef _WIN32 // windows-only - to output non-ascii chars
  SetConsoleOutputCP(CP_UTF8);
#endif

  std::vector<std::string> patterns = {"random", "sorted", "reversed", "duplicates",
                                       "almost_sorted"};

  int total_failures = 0;
  int total_tests = 0;

  // --- Run stability test ---
  total_tests++;
  if (!run_stability_test()) {
    total_failures++;
  }

  // --- Run pattern-based benchmarks ---
  for (const auto &pattern : patterns) {
    for (size_t size : {10, 100, 1'000, 10'000, 100'000, 1'000'000}) {
      total_tests++;
      bool ok = run_test_case(pattern, size, sort1_fn, sort2_fn, sort1_name, sort2_name);
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
