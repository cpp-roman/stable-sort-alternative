#include <functional>

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
