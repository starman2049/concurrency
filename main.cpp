#include <iostream>
#include <thread>
#include <vector>
#include "linked_list.h"

// Non-thread-safe test: 5 threads each add 1000 items to a shared LinkedList.
// Because the list is not protected by a mutex, data races will occur and the
// final count will typically be less than 5000.
void test__unprotected() {
    LinkedList<int> list;
    const int num_threads = 5;
    const int items_per_thread = 1000;

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&list, t, items_per_thread]() {
            for (int i = 0; i < items_per_thread; ++i) {
                list.push_front(t * items_per_thread + i);
            }
        });
    }

    for (auto& th : threads) {
        th.join();
    }

    std::cout << "test__unprotected: total items added = " << list.size() << std::endl;
}

int main() {
    test__unprotected();
    return 0;
}

