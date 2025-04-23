//
// Created by ESv87g9gvea4 on 2025/03/23.
//

#ifndef HEAPQUEUE_H
#define HEAPQUEUE_H
#include <vector>

#include "Genome.h"


/**
 * @class HeapQueue
 * @brief 優先度付きキューヒープ構造を使用して、指定された数の要素を効率的に管理するクラス。
 *
 * @details
 * このクラスは、優先度付きキューのように動作し、要素をヒープベースで管理します。
 * 最大要素数を超えた場合、最小要素を削除し、新しい要素を挿入することで管理を行います。
 *
 * @note Fitness（適応度）による比較を基に最小ヒープとして振舞います。
 *
 */
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
