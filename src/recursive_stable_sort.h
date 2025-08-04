#include <functional>

/**
 * @brief Stable sort algorithm without dynamic memory allocation.
 *
 * Sorts the given array in a stable manner using the provided buffer.
 * This implementation performs a recursive, top-down merge sort, similar
 * to GCC's std::stable_sort, and avoids all dynamic memory allocations.
 *
 * @tparam T       The element type.
 * @tparam Compare A comparison function object (e.g., std::less<T>).
 *
 * @param data   Pointer to the array to be sorted.
 * @param size   Number of elements in the array.
 * @param buffer External buffer of at least `size` elements.
 *               Must be the same type as `data`.
 * @param comp   Comparison function to define the sorting order.
 *               Defaults to std::less<T>.
 *
 * @note The `buffer` must have a size equal to the input array (`size`),
 *       and must be preallocated by the caller.
 *
 * #### Example usage:
 * ```
 * struct Record {
 *   int key;
 *   int original_index;
 * };
 *
 * struct KeyCompare {
 *   bool operator()(const Record &a, const Record &b) const {
 *     return a.key < b.key;
 *   }
 * };
 *
 * std::vector<Record> data = {
 *   {3, 0}, {1, 1}, {2, 2}, {1, 3}, {2, 4}, {3, 5}
 * };
 * std::vector<Record> buffer(data.size());
 *
 * recursive_stable_sort(data.data(), data.size(), buffer.data(), KeyCompare{});
 * ```
 */
template <typename T, typename Compare = std::less<T>>
void recursive_stable_sort(T *data, size_t size, T *buffer, Compare comp = Compare{}) {
  if (size <= 1) {
    return;
  }

  // Recursive helper function to sort range [data, data + size)
  auto recursive_merge_sort = [&](T *data, size_t size, T *buffer, auto &self) {
    if (size <= 1) {
      return;
    }
    size_t mid = size / 2;

    // Recursively sort left and right halves
    self(data, mid, buffer, self);
    self(data + mid, size - mid, buffer + mid, self);

    // Merge sorted halves into buffer
    size_t i = 0, j = mid, k = 0;
    while (i < mid && j < size) {
      if (comp(data[j], data[i])) {
        buffer[k++] = std::move(data[j++]);
      } else {
        buffer[k++] = std::move(data[i++]);
      }
    }
    while (i < mid) {
      buffer[k++] = std::move(data[i++]);
    }
    while (j < size) {
      buffer[k++] = std::move(data[j++]);
    }

    // Copy merged result back to data
    std::copy(buffer, buffer + size, data);
  };

  // Start recursive sort
  recursive_merge_sort(data, size, buffer, recursive_merge_sort);
}
