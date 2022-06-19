#include <iostream>

#include "bpt.hpp"
huang::BPlusTree<int, int> bpt("test");

int main() {
    freopen("test.out", "w", stdout);
    // for (int i = 1000; i >= 1; i--) {
    //     bpt.Insert(i, i);
    // }
    for (int i = 1; i <= 1000; i += 2) {
        bpt.Remove(i);
    }
    for (int i = 1; i <= 1000; i++) {
        auto ret = bpt.GetValue(i);
        if (ret.first)
            std::cout << ret.second << std::endl;
        else
            std::cout << "null" << std::endl;
    }
    return 0;
}
