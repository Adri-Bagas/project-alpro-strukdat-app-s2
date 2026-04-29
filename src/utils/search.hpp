#pragma once

#include <vector>

namespace SearchUtils {

    template <typename T>
    bool linear_search(const std::vector<T>& data, const T& target) {
        for (const auto& item : data) {
            if (item == target) {
                return true;
            }
        }
        return false;
    }

    template <typename T, typename MatchFn>
    std::vector<T> linear_search_if(const std::vector<T>& data, MatchFn match_fn) {
        std::vector<T> result;

        for (const auto& item : data) {
            if (match_fn(item)) {
                result.push_back(item);
            }
        }

        return result;
    }

} // namespace SearchUtils
