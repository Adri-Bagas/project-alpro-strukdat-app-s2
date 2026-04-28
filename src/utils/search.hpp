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
    bool linear_search_if(const std::vector<T>& data, MatchFn match_fn) {
        for (const auto& item : data) {
            if (match_fn(item)) {
                return true;
            }
        }
        return false;
    }

} // namespace SearchUtils
