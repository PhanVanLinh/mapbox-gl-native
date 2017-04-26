#pragma once

#include <cstddef>
#include <functional>
#include <iterator>
#include <vector>

namespace mbgl {

template <class InIt, class OutIt, class Equal>
OutIt longest_common_subsequence(InIt a, InIt endA,
                                 InIt b, InIt endB,
                                 OutIt outIt,
                                 Equal eq) {
    const std::ptrdiff_t N = endA - a;
    const std::ptrdiff_t M = endB - b;
    const std::ptrdiff_t D = N + M;

    if (D == 0) {
        return outIt;
    }

    std::vector<std::vector<std::ptrdiff_t>> vs;

    // Self-executing lambda to allow `return` to break from inner loop, and avoid shadowing `v`.
    [&] () {
        std::vector<std::ptrdiff_t> v;
        v.resize(2 * D + 1);
        v[1] = 0;

        for (std::ptrdiff_t d = 0; d <= D; ++d) {
            for (std::ptrdiff_t k = -d; k <= d; k += 2) {
                std::ptrdiff_t x = (k == -d || (k != d && v.at(k - 1 + D) < v.at(k + 1 + D)))
                    ? v.at(k + 1 + D)       // moving down
                    : v.at(k - 1 + D) + 1;  // moving right

                std::ptrdiff_t y = x - k;

                while (x < N && y < M && eq(a[x], b[y])) {
                    x++;
                    y++;
                }

                v[k + D] = x;

                if (x >= N && y >= M) {
                    vs.push_back(v);
                    return;
                }
            }

            vs.push_back(v);
        }
    }();

    std::ptrdiff_t x = N;
    std::ptrdiff_t y = M;

    using E = typename std::iterator_traits<InIt>::value_type;
    std::vector<E> lcsReverse;

    for (std::ptrdiff_t d = vs.size() - 1; x > 0 || y > 0; --d) {
        const std::vector<std::ptrdiff_t>& v = vs.at(d);
        const std::ptrdiff_t k = x - y;
        const bool down = (k == -d || (k != d && v.at(k - 1 + D) < v.at(k + 1 + D)));
        const std::ptrdiff_t kPrev = down ? k + 1 : k - 1;

        x = v.at(kPrev + D);
        y = x - kPrev;

        for (std::ptrdiff_t c = v[k + D]; c != (down ? x : x + 1); --c) {
            lcsReverse.push_back(a[c - 1]);
        }
    }

    return std::copy(lcsReverse.rbegin(), lcsReverse.rend(), outIt);
}

template < typename InIt, typename OutIt >
OutIt longest_common_subsequence(InIt a, InIt endA,
                                 InIt b, InIt endB,
                                 OutIt outIt) {
    return longest_common_subsequence(a, endA, b, endB, outIt, std::equal_to<>());
}

}
