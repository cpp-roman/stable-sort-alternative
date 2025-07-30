#include <chrono>
#include <iostream>
#include <random>

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
    for (size_t i = 0; i < n; ++i) {
      data[i] = {dist(rng), static_cast<int>(i)};
    }
  } else if (pattern == "sorted") {
    for (size_t i = 0; i < n; ++i) {
      data[i] = {static_cast<int>(i), static_cast<int>(i)};
    }
  } else if (pattern == "reversed") {
    for (size_t i = 0; i < n; ++i) {
      data[i] = {static_cast<int>(n - i), static_cast<int>(i)};
    }
  } else if (pattern == "duplicates") {
    for (size_t i = 0; i < n; ++i) {
      data[i] = {static_cast<int>(i % 5), static_cast<int>(i)};
    }
  } else if (pattern == "almost_sorted") {
    for (size_t i = 0; i < n; ++i) {
      data[i] = {static_cast<int>(i), static_cast<int>(i)};
    }

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

// ---- Stability test ----
bool run_stability_test() {
  std::vector<Record> data = {
      {5, 0},  {3, 1},  {3, 2},  {3, 3},  {1, 4},  {1, 5},  {2, 6},  {2, 7},  {4, 8},
      {4, 9},  {5, 10}, {3, 11}, {1, 12}, {1, 13}, {2, 14}, {2, 15}, {4, 16}, {4, 17},
      {5, 18}, {3, 19}, {3, 20}, {3, 21}, {1, 22}, {1, 23}, {2, 24}, {2, 25}, {4, 26},
      {4, 27}, {5, 28}, {5, 29}, {1, 30}, {2, 31}, {3, 32}, {4, 33}, {5, 34}, {1, 35},
      {2, 36}, {3, 37}, {4, 38}, {5, 39}, {1, 40}, {2, 41}, {3, 42}, {4, 43}, {5, 44},
      {1, 45}, {2, 46}, {3, 47}, {4, 48}, {5, 49}};

  std::vector<Record> expected = data;
  std::vector<Record> buffer(data.size());

  std::stable_sort(expected.begin(), expected.end(), KeyCompare{});
  my_stable_sort(data.data(), data.size(), buffer.data(), KeyCompare{});

  if (data == expected) {
    std::cout << "\n✅✅ Stability test passed\n\n";
    return true;
  } else {
    std::cerr << "\n❌❌ Stability test failed!\n\n";
    for (size_t i = 0; i < data.size(); ++i) {
      if (!(data[i] == expected[i])) {
        std::cerr << "  At index " << i << ": expected " << expected[i] << ", got "
                  << data[i] << "\n\n";
        break;
      }
    }
    return false;
  }
}
