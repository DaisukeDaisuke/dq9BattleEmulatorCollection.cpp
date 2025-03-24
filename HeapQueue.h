//
// Created by ESv87g9gvea4 on 2025/03/24.
//

#ifndef HEAPQUEUE_H
#define HEAPQUEUE_H

#include <vector>

#include "Genome.h"


class HeapQueue {
private:
    std::vector<Genome> heap;
    size_t maxSize;

public:
    explicit HeapQueue(size_t maxSize) : maxSize(maxSize) {
        heap.reserve(maxSize);
    }

    void push(const Genome &genome) {
        if (heap.size() < maxSize) {
            heap.push_back(genome);
            std::push_heap(heap.begin(), heap.end());
        } else {
            // 200 を超えたら、最小要素を削除（最小ヒープのように管理）
            if (genome.fitness > heap.front().fitness) {
                std::pop_heap(heap.begin(), heap.end());
                heap.back() = genome;
                std::push_heap(heap.begin(), heap.end());
            }
        }
    }

    void pop() {
        std::pop_heap(heap.begin(), heap.end());
        heap.pop_back();
    }

    [[nodiscard]] Genome top() const {
        return heap.front();
    }

    [[nodiscard]] bool empty() const {
        return heap.empty();
    }

    [[nodiscard]] size_t size() const {
        return heap.size();
    }
};

#endif //HEAPQUEUE_H
