#include <algorithm>
#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <windows.h>

// ---- Custom stable sort ----
template <typename T, typename Compare = std::less<T>>
void my_stable_sort(T *data, size_t size, T *buffer, Compare comp = Compare{}) {
  if (size <= 1)
    return;

  auto merge = [&](T *left, size_t left_size, T *right, size_t right_size, T *out) {
    size_t i = 0, j = 0, k = 0;
    while (i < left_size && j < right_size) {
      if (comp(right[j], left[i])) {
        out[k++] = std::move(right[j++]);
      } else {
        out[k++] = std::move(left[i++]);
      }
    }
    while (i < left_size)
      out[k++] = std::move(left[i++]);
    while (j < right_size)
      out[k++] = std::move(right[j++]);
  };

  for (size_t width = 1; width < size; width *= 2) {
    for (size_t i = 0; i < size; i += 2 * width) {
      size_t left = i;
      size_t mid = std::min(i + width, size);
      size_t right = std::min(i + 2 * width, size);

      merge(data + left, mid - left, data + mid, right - mid, buffer + left);
    }
    std::copy(buffer, buffer + size, data);
  }
}

// ---- Struct to test stability ----
struct Record {
  int key;
  int original_index;

  bool operator==(const Record &other) const {
    return key == other.key && original_index == other.original_index;
  }
};

std::ostream &operator<<(std::ostream &os, const Record &r) {
  return os << "(" << r.key << ", " << r.original_index << ")";
}

struct KeyCompare {
  bool operator()(const Record &a, const Record &b) const {
    return a.key < b.key;
  }
};

// ---- Generate test data ----
std::vector<Record> generate_data(size_t n, const std::string &pattern) {
  std::vector<Record> data(n);
  if (pattern == "random") {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 1000);
    for (size_t i = 0; i < n; ++i)
      data[i] = {dist(rng), static_cast<int>(i)};
  } else if (pattern == "sorted") {
    for (size_t i = 0; i < n; ++i)
      data[i] = {static_cast<int>(i), static_cast<int>(i)};
  } else if (pattern == "reversed") {
    for (size_t i = 0; i < n; ++i)
      data[i] = {static_cast<int>(n - i), static_cast<int>(i)};
  } else if (pattern == "duplicates") {
    for (size_t i = 0; i < n; ++i)
      data[i] = {static_cast<int>(i % 5), static_cast<int>(i)};
  } else if (pattern == "almost_sorted") {
    for (size_t i = 0; i < n; ++i)
      data[i] = {static_cast<int>(i), static_cast<int>(i)};

    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> dist(0, n - 2);
    size_t swaps = n / 100;
    for (size_t s = 0; s < swaps; ++s) {
      size_t idx = dist(rng);
      std::swap(data[idx], data[idx + 1]);
    }
  } else {
    throw std::invalid_argument("Unknown pattern: " + pattern);
  }
  return data;
}

// ---- Benchmark one sorting function ----
template <typename Func> long long benchmark(Func sort_fn, std::vector<Record> &data) {
  auto start = std::chrono::steady_clock::now();
  sort_fn(data);
  auto end = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

// ---- Test runner comparing two sort functions ----
template <typename SortFunc1, typename SortFunc2>
bool run_test_case(const std::string &label, size_t size, SortFunc1 sort_fn1,
                   SortFunc2 sort_fn2, const std::string &name1,
                   const std::string &name2) {
  std::cout << "Test case: " << label << " [" << size << " elements]\n";

  auto original = generate_data(size, label);
  auto data1 = original;
  auto data2 = original;

  std::vector<Record> buffer(size);

  auto time1 = benchmark([&](auto &data) { sort_fn1(data, buffer.data()); }, data1);
  auto time2 = benchmark([&](auto &data) { sort_fn2(data, buffer.data()); }, data2);

  bool ok = (data1 == data2);
  if (ok) {
    std::cout << "✅ Match\n";
  } else {
    std::cerr << "❌ Mismatch detected!\n";
    for (size_t i = 0; i < size; ++i) {
      if (!(data1[i] == data2[i])) {
        std::cerr << "  At index " << i << ": expected " << data1[i] << ", got "
                  << data2[i] << "\n";
        break;
      }
    }
  }

  std::cout << "   " << name1 << ": " << time1 << " µs\n";
  std::cout << "   " << name2 << ": " << time2 << " µs\n\n";

  return ok;
}

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
  SetConsoleOutputCP(CP_UTF8);

  std::vector<std::string> patterns = {"random", "sorted", "reversed", "duplicates",
                                       "almost_sorted"};

  int total_failures = 0;
  int total_tests = 0;

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
    std::cout << "✅✅ All " << total_tests << " tests passed.\n";
  } else {
    std::cout << "❌❌ " << total_passes << "/" << total_tests << " tests passed. "
              << total_failures << " tests failed.\n";
  }

  return total_failures == 0 ? 0 : 1;
}
