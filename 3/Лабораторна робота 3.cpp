#include <initializer_list>
#include <string_view>
#include <numeric>
#include <string>
#include <vector>

namespace lab {
    namespace detail {

        template <typename T>
        struct Node {

            T data;
            Node* left = nullptr;
            Node* right = nullptr;
            Node* parent = nullptr;

            [[nodiscard]]
            bool isLeftSon() const noexcept {
                if (!parent) {
                    return false;
                }
                if (!parent->left) {
                    return false;
                }
                return parent->left == &*this;
            }

            [[nodiscard]]
            bool isRightSon() const noexcept {
                if (!parent) {
                    return false;
                }
                if (!parent->right) {
                    return false;
                }
                return parent->right == &*this;
            }
        };


        /**
         *  @brief Assign var to value, setting var's parent new_parent
         */
        template <typename T>
        void set(Node<T>*& var, Node<T>* value, Node<T>* new_parent = nullptr) {
            var = value;
            if (var) {
                var->parent = new_parent;
            }
        }


        template <typename T>
        auto find(Node<T>* node, const T& elem) noexcept -> Node<T>*
        {
            while (node) {
                if (node->data > elem) {
                    node = node->left;
                }
                else if (node->data < elem) {
                    node = node->right;
                }
                else {
                    return node;
                }
            }
            return nullptr;
        }

        template <typename T>
        auto min(Node<T>* node) noexcept -> Node<T>*
        {
            while (node->left) {
                node = node->left;
            }
            return node;
        }

        template <typename T>
        auto max(Node<T>* node) noexcept -> Node<T>*
        {
            while (node->right) {
                node = node->right;
            }
            return node;
        }

        template <typename T>
        auto merge(Node<T>* lhs, Node<T>* rhs) noexcept -> Node<T>*
        {
            if (!lhs) {
                return rhs;
            }
            if (!rhs) {
                return lhs;
            }
            assert(max(lhs)->data < min(rhs)->data);
            auto lhs_root = splay(max(lhs));
            lhs_root->right = rhs;
            lhs_root->right->parent = lhs_root;
            return lhs_root;
        }

        template <typename T>
        auto split(Node<T>* tree, const T& elem) noexcept -> std::pair<Node<T>*, Node<T>*>
        {
            auto begin = SplayTree<T>::Iterator(min(tree));
            auto end = SplayTree<T>::Iterator(nullptr);

            T upper_bound = std::upper_bound(begin, end, elem);
            auto upper_bound_node = find(tree, elem);
            auto new_tree = splay(upper_bound_node);
            auto leftTree = new_tree->left;
            leftTree->parent = nullptr;
            new_tree->left = nullptr;
            return std::pair(leftTree, new_tree);
        }

        template <typename T>
        void deleteTree(Node<T>* node) {
            if (!node) {
                return;
            }
            deleteTree(node->left);
            deleteTree(node->right);
            delete node;
        }
    } // namespace detail

    class Flight {
    public:
        explicit Flight() noexcept = default;
        explicit Flight(std::string_view name, unsigned length = 0) noexcept
            : m_name(name),
            m_length(length)
        {}

        [[nodiscard]]
        auto name() const noexcept {
            return m_name;
        }

        [[nodiscard]]
        auto length() const noexcept {
            return m_length;
        }

    private:
        std::string m_name;
        unsigned m_length = 0;
    };

    class Airlines {
    public:

        explicit Airlines() noexcept = default;

        explicit Airlines(std::string_view name,
            std::initializer_list<Flight> flights) noexcept
            : m_name(name),
            m_flight(flights)
        {}

        [[nodiscard]]
        auto name() const noexcept {
            return m_name;
        }

        [[nodiscard]]
        auto flights() const noexcept {
            return m_flight;
        }

        [[nodiscard]]
        auto size() const noexcept {
            return std::accumulate(m_flight.begin(),
                m_flight.end(), 0,
                [](auto sum, const auto& flight) {
                    return sum + flight.size();
                });
        }

    private:
        std::vector<Flight> m_flight;
        std::string_view m_name;
    };

    bool operator< (const Airlines& lhs, const Airlines& rhs) {
        return lhs.size() < rhs.size();
    }

    bool operator== (const Airlines& lhs, const Airlines& rhs) {
        return lhs.flights() == rhs.flights();
    }

    bool operator> (const Airlines& lhs, const Airlines& rhs) {
        return lhs.size() > rhs.size();
    }

    bool operator< (const Flight& lhs, const Flight& rhs) {
        return lhs.size() < rhs.size();
    }

    bool operator== (const Flight& lhs, const Flight& rhs) {
        return lhs.size() == rhs.size() && lhs.name() == rhs.name();
    }

    bool operator> (const Flight& lhs, const Flight& rhs) {
        return lhs.size() > rhs.size();
    }

    template <typename T>
    struct Node;

    template <typename T>
    class Splayer {
    public:
        explicit Splayer(Node<T>* node);

        Node<T>* splay() noexcept;

    private:
        void zig() noexcept;
        void zigzig() noexcept;
        void zigzag() noexcept;

    private:
        Node<T>* m_node;
    };

    template <typename T>
    inline auto splay(Node<T>* node) noexcept -> Node<T>*
    {
        return Splayer{ node }.splay();
    }

    template <typename T>
    Splayer<T>::Splayer(Node<T>* node)
        : m_node(node)
    {}

    template <typename T>
    auto Splayer<T>::splay() noexcept -> Node<T>*
    {
        while (m_node->parent) {
            if ((m_node->isRightSon() && m_node->parent->isLeftSon()) ||
                (m_node->isLeftSon() && m_node->parent->isRightSon())) {
                zigzag();
            }
            else if ((m_node->isLeftSon() && m_node->parent->isLeftSon()) ||
                (m_node->isRightSon() && m_node->parent->isRightSon())) {
                zigzig();
            }
            else {
                zig();
                return m_node;
            }
        }
        return m_node;
    }

    template <typename T>
    void Splayer<T>::zig() noexcept
    {
        auto& parent = m_node->parent;
        if (m_node->isLeftSon()) {
            set(parent->left, m_node->right, parent);
            set(m_node->right, parent, m_node);
        }
        else if (m_node->isRightSon()) {
            set(parent->right, m_node->left, parent);
            set(m_node->left, parent, m_node);
        }
        parent = nullptr;
    }

    template <typename T>
    void Splayer<T>::zigzig() noexcept
    {
        auto mostParent = m_node->parent->parent->parent;
        auto grandparent = m_node->parent->parent;
        if (grandparent->isLeftSon()) {
            mostParent->left = m_node;
        }
        else if (grandparent->isRightSon()) {
            mostParent->right = m_node;
        }
        auto parent = m_node->parent;

        if (m_node->isLeftSon() && parent->isLeftSon()) {
            set(grandparent->left, parent->right, grandparent);
            set(parent->right, grandparent, parent);
            set(parent->left, m_node->right, parent);
            set(m_node->right, parent, m_node);
        }

        else if (m_node->isRightSon() && parent->isRightSon()) {
            set(grandparent->right, parent->left, grandparent);
            set(parent->left, grandparent, parent);
            set(parent->right, m_node->left, parent);
            set(m_node->left, parent, m_node);
        }
        m_node->parent = mostParent;
    }

    template <typename T>
    void Splayer<T>::zigzag() noexcept
    {
        auto mostParent = m_node->parent->parent->parent;
        auto grandparent = m_node->parent->parent;
        if (grandparent->isLeftSon()) {
            mostParent->left = m_node;
        }
        else if (grandparent->isRightSon()) {
            mostParent->right = m_node;
        }
        auto parent = m_node->parent;

        if (m_node->isRightSon() && parent->isLeftSon()) {
            set(grandparent->left, m_node->right, grandparent);
            set(parent->right, m_node->left, parent);

            set(m_node->left, parent, m_node);
            set(m_node->right, grandparent, m_node);
        }
        else if (m_node->isLeftSon() && parent->isRightSon()) {
            set(grandparent->right, m_node->left, grandparent);
            set(parent->left, m_node->right, parent);

            set(m_node->right, parent, m_node);
            set(m_node->left, grandparent, m_node);
        }
        m_node->parent = mostParent;
    }

    template <typename T>
    bool SplayTree<T>::Iterator::operator!= (const SplayTree::Iterator& other) noexcept {
        return m_curr != other.m_curr;
    }

    template <typename T>
    bool SplayTree<T>::Iterator::operator== (const SplayTree::Iterator& other) noexcept {
        return m_curr == other.m_curr;
    }

    template <typename T>
    auto SplayTree<T>::Iterator::operator++() noexcept -> typename SplayTree<T>::Iterator&
    {
        if (m_curr->right) {
            auto s = m_curr->right->data;
            auto temp = m_curr->right;
            while (temp->left) {
                temp = temp->left;
            }
            m_curr = temp;
            return *this;
        }

        while (true) {
            if (!m_curr->parent) {
                m_curr = nullptr;
                return *this;
            }
            if (m_curr->isLeftSon()) {
                m_curr = m_curr->parent;
                return *this;
            }
            m_curr = m_curr->parent;
        }
    }

    template <typename T>
    auto SplayTree<T>::Iterator::operator*() noexcept -> const T& {
        return m_curr->data;
    }

    template <typename T>
    SplayTree<T>::Iterator::Iterator(SplayTree::NodePtr root) noexcept
        : m_curr(root)
    {}

    template <typename T>
    auto SplayTree<T>::Iterator::operator+ (int n) const noexcept -> typename SplayTree<T>::Iterator {
        SplayTree<T>::Iterator temp(*this);
        for (int i = 0; i < n; ++i) {
            ++temp;
        }
        return temp;
    }
} // namespace lab


int main() {

    using namespace lab;
    SplayTree<Flight> tree;

    std::array airlines = { Airline{" Airline 1",
                                 {Flight{"Monaco", 5},
                                  Flight{"Alabama", 12}},
                       Airline{"Airline 2",
                                  Flight{"Canada ", 7}} }

};

std::cout << "Airline to insert: \n";

for (const auto& airline : airlines) {
    for (const auto& flight : airline.flight()) {
        std::cout << flight.name() << ' '
            << "size: " << flight.size() << std::endl;
        tree.insert(flight);
    }
}

std::cout << "\n\nIterating in tree after insertion :\n";
for (const auto& i : tree) {
    std::cout << i.name() << ' '
        << "size: " << i.size() << std::endl;
}

std::cout << "\n\nErasing flight " << teams[0].flight()[0].name() << "\n";

tree.erase(teams[0].flights()[0]);

std::cout << "\nIterating in tree after erasing :\n";
for (const auto& i : tree) {
    std::cout << i.name() << ' '
        << "size: " << i.size() << std::endl;
}

return 0;
}
