#include <list>
#include <tuple>
#include <algorithm>
#include <iostream>

namespace lab {

    namespace detail {

        template <typename NodePtr>
        auto merge_nodes(NodePtr lhs, NodePtr rhs) noexcept -> NodePtr {
            if (lhs->data > rhs->data) {
                std::swap(lhs, rhs);
            }
            rhs->parent = lhs;
            rhs->sibling = lhs->child;
            lhs->child = rhs;
            ++lhs->degree;

            return lhs;
        }

        /**
         *  @brief Fixes heap by merging trees with one degree
         */
        template <typename NodePtr>
        void adjust(std::list <NodePtr>& heap) noexcept {
            if (heap.size() < 2) {
                return;
            }
            auto [it1, it2, it3] = std::make_tuple(heap.begin(), ++heap.begin(), ++(++heap.begin()));

            while (it1 != heap.end()) {

                if (it2 == heap.end()) {
                    ++it1;
                }
                else if ((*it1)->degree < (*it2)->degree) {
                    ++it1;
                    ++it2;
                    if (it3 != heap.end()) {
                        ++it3;
                    }
                }

                else if (it3 != heap.end() &&
                    (*it1)->degree == (*it2)->degree &&
                    (*it1)->degree == (*it3)->degree)
                {
                    ++it1;
                    ++it2;
                    ++it3;
                }

                else if ((*it1)->degree == (*it2)->degree) {
                    NodePtr temp;
                    *it1 = merge_nodes(*it1, *it2);
                    it2 = heap.erase(it2);
                    if (it3 != heap.end()) {
                        ++it3;
                    }
                }
            }
        }

        template <typename NodePtr>
        auto merge_heaps(std::list<NodePtr>& lhs, std::list<NodePtr>& rhs) -> std::list<NodePtr> {
            auto new_list = std::move(lhs);

            new_list.merge(std::move(rhs), [](auto lhs_node, auto rhs_node) {
                return lhs_node->degree < rhs_node->degree; }
            );

            adjust(new_list);
            return new_list;
        }

        /**
         *  @brief Return all children of node as heap and deletes heap
         */
        template <typename NodePtr>
        auto children_as_heap(NodePtr node) noexcept -> std::list<NodePtr> {

            std::list<NodePtr> heap;
            NodePtr temp = node->child;
            NodePtr clean_temp = nullptr;

            while (temp)
            {
                clean_temp = temp;
                temp = temp->sibling;
                clean_temp->sibling = nullptr;
                clean_temp->parent = nullptr;
                heap.push_front(clean_temp);
            }
            return heap;
        }

        template <typename NodePtr>
        auto remove_min(std::list<NodePtr> heap) -> std::list<NodePtr> {
            auto min = *std::min_element(heap.begin(), heap.end(),
                [](const auto node, const auto smallest) {
                    return node->data < smallest->data; });

            heap.erase(std::remove(heap.begin(), heap.end(), min), heap.end());
            auto mins_children = children_as_heap(min);
            delete min;
            return merge_heaps(mins_children, heap);
        }

        template <typename NodePtr>
        void eraseSubTree(NodePtr node) noexcept {
            while (node) {
                eraseSubTree(node->child);
                auto temp = node;
                node = node->sibling;
                delete temp;
            }
        }
    };

    template <typename T>
    class BinomialHeap {

        struct Node {
            T data;
            std::size_t degree = 0;

            Node* child = nullptr;
            Node* sibling = nullptr;
            Node* parent = nullptr;
        };

        using NodePtr = Node*;

    public:
        BinomialHeap(std::initializer_list<T> elems) {
            for (const auto& el : elems) {
                insert(el);
            }
        }

        void insert(const T& data) {
            std::list <NodePtr> new_list = { new Node{data} };
            _nodes = detail::merge_heaps(_nodes, new_list);
        }

        void pop() {
            _nodes = detail::remove_min(_nodes);
        }

        [[nodiscard]]
        auto front() const noexcept -> const T& {
            return (*std::min_element(_nodes.begin(), _nodes.end(),
                [](const auto node, const auto smallest) {
                    return node->data < smallest->data; }))->data;
        }

        ~BinomialHeap() {
            for (auto& node : _nodes) {
                detail::eraseSubTree(node);
            }
        }

    private:
        std::list<NodePtr> _nodes;
    };
}

int main() {
    using namespace lab;
    constexpr std::size_t size = 6;

    BinomialHeap heap = { 22, 34, 2, 11, 21, 1 };

    std::cout << "Created binomaial heap with elements : {22, 34, 2, 11, 21, 1}\n\n";

    for (int i = 0; i < size; ++i) {
        std::cout << "Min element in heap : " << heap.front() << std::endl;
        std::cout << "Erasing min element...\n\n";
        heap.pop();
    }

    std::cout << "O has a nice heap\n";
    return 0;
}
