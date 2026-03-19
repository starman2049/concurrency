#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>

static const int NUM_THREADS = 4;
static const int NUM_INCREMENTS = 100000;

struct Node {
    int value;
    Node* next;
    Node(int v, Node* n) : value(v), next(n) {}
};

// Count nodes in the list and free all memory.
int count_and_free(Node* head) {
    int count = 0;
    while (head) {
        Node* tmp = head;
        head = head->next;
        delete tmp;
        ++count;
    }
    return count;
}

// Return milliseconds elapsed since the given start point.
static double elapsed_ms(std::chrono::high_resolution_clock::time_point start) {
    return std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - start).count();
}

void test_unprotected() {
    Node* head = nullptr;
    std::vector<std::thread> threads;
    // Time the full operation including thread creation and joining.
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&head]() {
            for (int j = 0; j < NUM_INCREMENTS; ++j) {
                head = new Node(j, head);
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    double ms = elapsed_ms(start);
    int count = count_and_free(head);
    std::cout << "test_unprotected: list size = " << count
              << " (expected " << NUM_THREADS * NUM_INCREMENTS << ")"
              << ", time = " << ms << " ms" << std::endl;
}

void test_sw_locks() {
    Node* head = nullptr;
    std::mutex mtx;
    std::vector<std::thread> threads;
    // Time the full operation including thread creation and joining.
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&head, &mtx]() {
            for (int j = 0; j < NUM_INCREMENTS; ++j) {
                std::lock_guard<std::mutex> lock(mtx);
                head = new Node(j, head);
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    double ms = elapsed_ms(start);
    int count = count_and_free(head);
    std::cout << "test_sw_locks:     list size = " << count
              << " (expected " << NUM_THREADS * NUM_INCREMENTS << ")"
              << ", time = " << ms << " ms" << std::endl;
}

void test_hw_locks() {
    std::atomic<Node*> head{nullptr};
    std::vector<std::thread> threads;
    // Time the full operation including thread creation and joining.
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&head]() {
            for (int j = 0; j < NUM_INCREMENTS; ++j) {
                Node* new_node = new Node(j, nullptr);
                Node* old_head = head.load(std::memory_order_relaxed);
                do {
                    new_node->next = old_head;
                } while (!head.compare_exchange_weak(old_head, new_node,
                                                     std::memory_order_release,
                                                     std::memory_order_relaxed));
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    double ms = elapsed_ms(start);
    int count = count_and_free(head.load());
    std::cout << "test_hw_locks:     list size = " << count
              << " (expected " << NUM_THREADS * NUM_INCREMENTS << ")"
              << ", time = " << ms << " ms" << std::endl;
}

int main() {
    test_unprotected();
    test_sw_locks();
    test_hw_locks();
    return 0;
}
