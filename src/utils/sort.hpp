#pragma once

#include <utility>
#include <vector>

namespace SortUtils {

    constexpr static const int minRUN = 32;
    inline int calcMinRun(int n) {
        int r = 0;
        while (n >= minRUN) {
            r |= (n & 1);
            n >>= 1;
        }
        return n + r;
    }

    template <typename T, typename Compare>
    void insertion_sort(T *arr, int left, int right, Compare comp) {
        for (int i = left + 1; i <= right; i++) {
            T temp = *(arr + i);
            int j = i - 1;
            while (j >= left && comp(temp, *(arr + j))) {
                *(arr + j + 1) = *(arr + j);
                j--;
            }
            *(arr + j + 1) = temp;
        }
    }

    template <typename T, typename Compare> 
    void merge(T *arr, int l, int m, int r, Compare comp) {
        int len1 = m - l + 1, len2 = r - m;
        T *left = new T[len1], *right = new T[len2];

        for (int i = 0; i < len1; i++)
            *(left + i) = *(arr + l + i);
        for (int i = 0; i < len2; i++)
            *(right + i) = *(arr + m + 1 + i);

        int i = 0, j = 0, k = l;
        while (i < len1 && j < len2) {
            if (comp(*(left + i), *(right + j))) {
                *(arr + k) = *(left + i);
                i++;
            } else {
                *(arr + k) = *(right + j);
                j++;
            }
            k++;
        }
        while (i < len1) {
            *(arr + k) = *(left + i);
            k++;
            i++;
        }
        while (j < len2) {
            *(arr + k) = *(right + j);
            k++;
            j++;
        }

        delete[] left;
        delete[] right;
    }

    template <typename T, typename Compare>
    int findRun(T* arr, int start, int n, Compare comp) {
        int end = start + 1;
        if (end == n) return end;

        if (comp(*(arr + end), *(arr + start))) {
            // Descending
            while (end < n && comp(*(arr + end), *(arr + end - 1))) end++;
            int l = start, r = end - 1;
            while (l < r) {
                std::swap(*(arr + l), *(arr + r));
                l++; r--;
            }
        } else {
            // Ascending
            while (end < n && !comp(*(arr + end), *(arr + end - 1))) end++;
        }
        return end;
    }

    template <typename T, typename Compare>
    void timsort(T* arr, int n, Compare comp) {
        if (n < 2) return;

        int minRun = calcMinRun(n);
        std::vector<std::pair<int, int>> runs;

        int i = 0;
        while (i < n) {
            int runEnd = findRun(arr, i, n, comp);
            int runLen = runEnd - i;

            if (runLen < minRun) {
                int end = std::min(i + minRun, n);
                insertion_sort(arr, i, end - 1, comp);
                runEnd = end;
            }
            runs.push_back({i, runEnd});
            i = runEnd;

            // Merge stack logic
            while (runs.size() > 1) {
                auto& r2 = runs.back();
                auto& r1 = runs[runs.size() - 2];
                
                if ((r1.second - r1.first) <= (r2.second - r2.first)) {
                    merge(arr, r1.first, r1.second - 1, r2.second - 1, comp);
                    r1.second = r2.second;
                    runs.pop_back();
                } else break;
            }
        }

        // cleanup merges
        while (runs.size() > 1) {
            auto& r2 = runs.back();
            auto& r1 = runs[runs.size() - 2];
            merge(arr, r1.first, r1.second - 1, r2.second - 1, comp);
            r1.second = r2.second;
            runs.pop_back();
        }
    }

} // namespace SortUtils
