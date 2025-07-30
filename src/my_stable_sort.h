#include <functional>

/**
 * @brief Stable sort algorithm without dynamic memory allocation.
 *
 * Sorts the given array in a stable manner using the provided buffer.
 * This implementation performs an iterative, bottom-up merge sort and
 * avoids all dynamic memory allocations.
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
 * @example
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
 * my_stable_sort(data.data(), data.size(), buffer.data(), KeyCompare{});
 * ```
 */
template <typename T, typename Compare = std::less<T>>
void my_stable_sort(T *data, size_t size, T *buffer, Compare comp = Compare{}) {
  if (size <= 1) {
    return;
  }

  auto merge = [&](T *left, size_t left_size, T *right, size_t right_size, T *out) {
    size_t i = 0, j = 0, k = 0;
    while (i < left_size && j < right_size) {
      if (comp(right[j], left[i])) {
        out[k++] = std::move(right[j++]);
      } else {
        out[k++] = std::move(left[i++]);
      }
    }
    while (i < left_size) {
      out[k++] = std::move(left[i++]);
    }
    while (j < right_size) {
      out[k++] = std::move(right[j++]);
    }
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
