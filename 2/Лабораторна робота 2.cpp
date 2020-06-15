#include <iostream>

enum class Colour { Black, Red };

template<class T>
struct Node {

    T key;
    Node* parent;
    Colour color;
    Node* left;
    Node* right;
};


template<class T>
class RBtree {
    Node<T>* root;
    Node<T>* q;
public:
    RBtree();

    void insert(T);
    void insertfix(Node<T>*);
    void leftrotate(Node<T>*);
    void rightrotate(Node<T>*);
    void del(T);
    auto successor(Node<T>*)->Node<T>*;
    void delfix(Node<T>*);
    void disp();
    void display(Node<T>*);
    void search();
};

template<class T>
RBtree<T>::RBtree() : q(nullptr), root(nullptr) {}


template<class T>
void RBtree<T>::insert(T z) {
    int i = 0;
    Node<T>* p, * q;

    auto* t = new Node<T>;

    t->key = z;
    t->left = nullptr;
    t->right = nullptr;
    t->color = Colour::Red;
    p = root;
    q = nullptr;
    if (root == nullptr) {
        root = t;
        t->parent = nullptr;
    }
    else {
        while (p != nullptr) {
            q = p;
            if (p->key < t->key)
                p = p->right;
            else
                p = p->left;
        }
        t->parent = q;
        if (q->key < t->key)
            q->right = t;
        else
            q->left = t;
    }
    insertfix(t);
    std::cout << z << " inserted\n";
}

template<class T>
void RBtree<T>::insertfix(Node<T>* t) {
    Node<T>* u;
    if (root == t) {
        t->color = Colour::Black;
        return;
    }
    while (t->parent != nullptr && t->parent->color == Colour::Red) {
        Node<T>* g = t->parent->parent;
        if (g->left == t->parent) {
            if (g->right != nullptr) {
                u = g->right;
                if (u->color == Colour::Red) {
                    t->parent->color = Colour::Black;
                    u->color = Colour::Black;
                    g->color = Colour::Red;
                    t = g;
                }
            }
            else {
                if (t->parent->right == t) {
                    t = t->parent;
                    leftrotate(t);
                }
                t->parent->color = Colour::Black;
                g->color = Colour::Red;
                rightrotate(g);
            }
        }
        else {
            if (g->left != nullptr) {
                u = g->left;
                if (u->color == Colour::Red) {
                    t->parent->color = Colour::Black;
                    u->color = Colour::Black;
                    g->color = Colour::Red;
                    t = g;
                }
            }
            else {
                if (t->parent->left == t) {
                    t = t->parent;
                    rightrotate(t);
                }
                t->parent->color = Colour::Black;
                g->color = Colour::Red;
                leftrotate(g);
            }
        }
        root->color = Colour::Black;
    }
}

template<class T>
void RBtree<T>::del(T x) {
    if (root == nullptr) {
        std::cout << "\nEmpty Tree.";
        return;
    }
    Node<T>* p;
    p = root;
    Node<T>* y = nullptr;
    Node<T>* q = nullptr;
    int found = 0;
    while (p != nullptr && found == 0) {
        if (p->key == x)
            found = 1;
        if (found == 0) {
            if (p->key < x)
                p = p->right;
            else
                p = p->left;
        }
    }
    if (found == 0) {
        std::cout << "\nElement Not Found.";
        return;
    }
    else {
        std::cout << "\nDeleted Element: " << p->key;
        std::cout << "\nColour: ";
        if (p->color == Colour::Black)
            std::cout << "Black\n";
        else
            std::cout << "Red\n";

        if (p->parent != nullptr)
            std::cout << "\nParent: " << p->parent->key;
        else
            std::cout << "\nThere is no parent of the node.  ";
        if (p->right != nullptr)
            std::cout << "\nRight Child: " << p->right->key;
        else
            std::cout << "\nThere is no right child of the node.  ";
        if (p->left != nullptr)
            std::cout << "\nLeft Child: " << p->left->key;
        else
            std::cout << "\nThere is no left child of the node.  ";
        std::cout << "\nNode Deleted.";
        if (p->left == nullptr || p->right == nullptr)
            y = p;
        else
            y = successor(p);
        if (y->left != nullptr)
            q = y->left;
        else {
            if (y->right != nullptr)
                q = y->right;
            else
                q = nullptr;
        }
        if (q != nullptr)
            q->parent = y->parent;
        if (y->parent == nullptr)
            root = q;
        else {
            if (y == y->parent->left)
                y->parent->left = q;
            else
                y->parent->right = q;
        }
        if (y != p) {
            p->color = y->color;
            p->key = y->key;
        }
        if (y->color == Colour::Black)
            delfix(q);
    }
}


template<class T>
void RBtree<T>::delfix(Node<T>* p) {
    Node<T>* s;
    while (p != root && p->color == Colour::Black) {
        if (p->parent->left == p) {
            s = p->parent->right;
            if (s->color == Colour::Red) {
                s->color = Colour::Black;
                p->parent->color = Colour::Red;
                leftrotate(p->parent);
                s = p->parent->right;
            }
            if (s->right->color == Colour::Black && s->left->color == Colour::Black) {
                s->color = Colour::Red;
                p = p->parent;
            }
            else {
                if (s->right->color == Colour::Black) {
                    s->left->color == Colour::Black;
                    s->color = Colour::Red;
                    rightrotate(s);
                    s = p->parent->right;
                }
                s->color = p->parent->color;
                p->parent->color = Colour::Black;
                s->right->color = Colour::Black;
                leftrotate(p->parent);
                p = root;
            }
        }
        else {
            s = p->parent->left;
            if (s->color == Colour::Red) {
                s->color = Colour::Black;
                p->parent->color = Colour::Red;
                rightrotate(p->parent);
                s = p->parent->left;
            }
            if (s->left->color == Colour::Black && s->right->color == Colour::Black) {
                s->color = Colour::Red;
                p = p->parent;
            }
            else {
                if (s->left->color == Colour::Black) {
                    s->right->color = Colour::Black;
                    s->color = Colour::Red;
                    leftrotate(s);
                    s = p->parent->left;
                }
                s->color = p->parent->color;
                p->parent->color = Colour::Black;
                s->left->color = Colour::Black;
                rightrotate(p->parent);
                p = root;
            }
        }
        p->color = Colour::Black;
        root->color = Colour::Black;
    }
}


template<class T>
void RBtree<T>::leftrotate(Node<T>* p) {
    if (p->right == nullptr)
        return;
    else {
        Node<T>* y = p->right;
        if (y->left != nullptr) {
            p->right = y->left;
            y->left->parent = p;
        }
        else
            p->right = nullptr;
        if (p->parent != nullptr)
            y->parent = p->parent;
        if (p->parent == nullptr)
            root = y;
        else {
            if (p == p->parent->left)
                p->parent->left = y;
            else
                p->parent->right = y;
        }
        y->left = p;
        p->parent = y;
    }
    std::cout << "  Forced Left Rotate <--\n";
}


template<class T>
void RBtree<T>::rightrotate(Node<T>* p) {
    if (p->left == nullptr)
        return;
    else {
        Node<T>* y = p->left;
        if (y->right != nullptr) {
            p->left = y->right;
            y->right->parent = p;
        }
        else
            p->left = nullptr;
        if (p->parent != nullptr)
            y->parent = p->parent;
        if (p->parent == nullptr)
            root = y;
        else {
            if (p == p->parent->left)
                p->parent->left = y;
            else
                p->parent->right = y;
        }
        y->right = p;
        p->parent = y;
    }
    std::cout << "  Forced Right Rotate -->\n";
}

template<class T>
Node<T>* RBtree<T>::successor(Node<T>* p) {
    Node<T>* y = nullptr;
    if (p->left != nullptr) {
        y = p->left;
        while (y->right != nullptr)
            y = y->right;
    }
    else {
        y = p->right;
        while (y->left != nullptr)
            y = y->left;
    }
    return y;
}

template<class T>
void RBtree<T>::disp() {
    display(root);
}


template<class T>
void RBtree<T>::display(Node<T>* p) {
    if (root == nullptr) {
        std::cout << "\nEmpty Tree.";
        return;
    }
    if (p != nullptr) {
        std::cout << "\n\t NODE: ";
        std::cout << "\n Key: " << p->key;
        std::cout << "\n Colour: ";
        if (p->color == Colour::Black)
            std::cout << "Black";
        else
            std::cout << "Red";
        if (p->parent != nullptr)
            std::cout << "\n Parent: " << p->parent->key;
        else
            std::cout << "\n There is no parent of the node.  ";
        if (p->right != nullptr)
            std::cout << "\n Right Child: " << p->right->key;
        else
            std::cout << "\n There is no right child of the node.  ";
        if (p->left != nullptr)
            std::cout << "\n Left Child: " << p->left->key;
        else
            std::cout << "\n There is no left child of the node.  ";
        std::cout << std::endl;
        if (p->left) {
            std::cout << "\n\nLeft:\n";
            display(p->left);
        }
        /*else
         cout<<"\nNo Left Child.\n";*/
        if (p->right) {
            std::cout << "\n\nRight:\n";
            display(p->right);
        }
        /*else
         cout<<"\nNo Right Child.\n"*/
    }
}


template<class T>
void RBtree<T>::search() {
    if (root == nullptr) {
        std::cout << "\nEmpty Tree\n";
        return;
    }
    int x;
    std::cout << "\n Enter key of the node to be searched: ";
    std::cin >> x;
    Node<T>* p = root;
    int found = 0;
    while (p != nullptr && found == 0) {
        if (p->key == x)
            found = 1;
        if (found == 0) {
            if (p->key < x)
                p = p->right;
            else
                p = p->left;
        }
    }
    if (found == 0)
        std::cout << "\nElement Not Found.";
    else {
        std::cout << "\n\t FOUND NODE: ";
        std::cout << "\n Key: " << p->key;
        std::cout << "\n Color: ";
        if (p->color == Colour::Black)
            std::cout << "Black";
        else
            std::cout << "Red";
        if (p->parent != nullptr)
            std::cout << "\n Parent: " << p->parent->key;
        else
            std::cout << "\n There is no parent of the node.  ";
        if (p->right != nullptr)
            std::cout << "\n Right Child: " << p->right->key;
        else
            std::cout << "\n There is no right child of the node.  ";
        if (p->left != nullptr)
            std::cout << "\n Left Child: " << p->left->key;
        else
            std::cout << "\n There is no left child of the node.  ";
        std::cout << std::endl;

    }
}
