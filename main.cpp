#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>

static const int NUM_THREADS = 4;
static const int NUM_INCREMENTS = 100000;

void test_unprotected() {
    int counter = 0;
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&counter]() {
            for (int j = 0; j < NUM_INCREMENTS; ++j) {
                ++counter;
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    std::cout << "test_unprotected: counter = " << counter
              << " (expected " << NUM_THREADS * NUM_INCREMENTS << ")" << std::endl;
}

void test_sw_locks() {
    int counter = 0;
    std::mutex mtx;
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&counter, &mtx]() {
            for (int j = 0; j < NUM_INCREMENTS; ++j) {
                std::lock_guard<std::mutex> lock(mtx);
                ++counter;
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    std::cout << "test_sw_locks:     counter = " << counter
              << " (expected " << NUM_THREADS * NUM_INCREMENTS << ")" << std::endl;
}

void test_hw_locks() {
    std::atomic<int> counter{0};
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&counter]() {
            for (int j = 0; j < NUM_INCREMENTS; ++j) {
                ++counter;
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    std::cout << "test_hw_locks:     counter = " << counter
              << " (expected " << NUM_THREADS * NUM_INCREMENTS << ")" << std::endl;
}

int main() {
    test_unprotected();
    test_sw_locks();
    test_hw_locks();
    return 0;
}
