#pragma once

#include <cstddef>

// Basic non-thread-safe singly linked list.
// Neither the node pointer manipulation nor the size counter is protected by
// any synchronisation primitive; concurrent access causes data races.
template <typename T>
class LinkedList {
public:
    LinkedList() : head_(nullptr), size_(0) {}

    ~LinkedList() {
        Node* current = head_;
        while (current) {
            Node* next = current->next;
            delete current;
            current = next;
        }
    }

    // Prepend a value to the front of the list
    void push_front(const T& value) {
        head_ = new Node{value, head_};
        ++size_;
    }

    std::size_t size() const { return size_; }

private:
    struct Node {
        T value;
        Node* next;
    };

    Node* head_;
    std::size_t size_;
};
