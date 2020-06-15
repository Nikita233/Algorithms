#include <random>
#include <ctime>
#include <vector>
#include <algorithm>

using std::vector;
using std::shared_ptr;

/**
* @brief Class for implementing Bplus Tree
*/
template<typename T>
class BplusTree : public Tree<T> {
public:
	class BplusNode;
	typedef shared_ptr<BplusNode> NodePtr;
public:
	explicit BplusTree(const int m);

	void insert(const T& key) override;
	void remove(const T& key) noexcept override;
	void print() const noexcept override;

	BplusIterator<T> find(const T& key) const noexcept;
	BplusIterator<T> begin() const noexcept;
	BplusIterator<T> end() const noexcept;
	BplusIterator<T> rbegin() const noexcept;
	BplusIterator<T> rend() const noexcept;

public:
	class BplusNode : public Tree<T>::Node {
	public:
		explicit BplusNode();
		~BplusNode();

		[[nodiscard]] bool is_leaf() const noexcept;

		std::shared_ptr<typename Tree<T>::Node> next() const noexcept override;
		std::shared_ptr<typename Tree<T>::Node> previous() const noexcept override;

		NodePtr left() const noexcept;
		NodePtr right() const noexcept;

		template<typename OStream>
		void print(OStream& os, int level = 1) const noexcept;
	public:
		NodePtr parent;
		NodePtr nxt;
		NodePtr prv;
		vector<NodePtr> children;
		vector<int> indexes;
		vector<T> data;
	};

private:
	const int M, MININD;
	NodePtr minNode;
private:
	///Utility functions for insert and remove
	int _min(NodePtr start) const noexcept;
	void split(NodePtr ptr) noexcept;
	void borrowLeft(NodePtr ptr) noexcept;
	void borrowRight(NodePtr ptr) noexcept;
	NodePtr merge(NodePtr ptr) const noexcept;
	void shrink() noexcept;
};


template<typename T>
int firstGreater(const vector<T>& v, const int val) {
	for (int i = 0; i < v.size(); ++i)
		if (val < v[i])
			return i;
	return v.size();
}

int generate() {
	static std::random_device rd;
	static std::seed_seq seed{ rd(), static_cast<unsigned int>(time(nullptr)) };
	static std::mt19937_64 gen(seed);
	static std::uniform_int_distribution<>dis(1, 1000);
	int res = dis(gen);
	return res;
}


template<typename T>
BplusTree<T>::BplusNode::BplusNode() :
	parent(nullptr),
	nxt(nullptr),
	prv(nullptr),
	children(vector<NodePtr>()),
	indexes(vector<int>()),
	data(vector<T>()) {}

template<typename T>
BplusTree<T>::BplusNode::~BplusNode() {
	indexes.clear();
	children.clear();
	data.clear();
}

template<typename T>
bool BplusTree<T>::BplusNode::is_leaf() const noexcept {
	return children.size() == 0;
}

template<typename T>
typename std::shared_ptr<typename Tree<T>::Node> BplusTree<T>::BplusNode::next() const noexcept {
	return indexes[0] < nxt->indexes[0] ? nxt : nullptr;
}

template<typename T>
typename std::shared_ptr<typename Tree<T>::Node> BplusTree<T>::BplusNode::previous() const noexcept {
	return  indexes[0] > prv->indexes[0] ? prv : nullptr;
}

template<typename T>
typename BplusTree<T>::NodePtr BplusTree<T>::BplusNode::left() const noexcept {
	if (parent)
		for (int i = 1; i < parent->children.size(); ++i)
			if (parent->children[i].get() == this)
				return parent->children[i - 1];
	return nullptr;
}

template<typename T>
typename BplusTree<T>::NodePtr BplusTree<T>::BplusNode::right() const noexcept {
	if (parent)
		for (int i = 0; i < parent->children.size() - 1; ++i)
			if (parent->children[i].get() == this)
				return parent->children[i + 1];
	return nullptr;
}

template<typename T>
template<typename OStream>
void BplusTree<T>::BplusNode::print(OStream& os, int level) const noexcept {
	for (int i = 0; i < indexes.size(); ++i) {
		if (!this->is_leaf())
			os << indexes[i] << ", ";
		else
			os << indexes[i] << '{' << data[i] << '}' << ", ";
	}
	os << '\n';
	for (int i = 0; i < children.size(); ++i) {
		for (int j = 0; j < level; ++j)
			os << '\t';
		children[i]->print(os, level + 1);
	}
}



template<typename T>
BplusTree<T>::BplusTree(const int m) :M{ m }, MININD{ int(m / 2) - 1 }, minNode(nullptr) {}

template<typename T>
int BplusTree<T>::_min(NodePtr start) const noexcept {
	NodePtr ptr = start;
	while (!ptr->is_leaf())
		ptr = ptr->children[0];
	return ptr->indexes[0];
}

template<typename T>
void BplusTree<T>::split(NodePtr ptr) noexcept {
	NodePtr root = std::dynamic_pointer_cast<BplusNode>(Tree<T>::root);

	NodePtr newN = std::make_shared<BplusNode>();

	///Split indexes
	newN->indexes.resize(MININD);
	std::copy(ptr->indexes.begin(), ptr->indexes.begin() + MININD, newN->indexes.begin());
	ptr->indexes.erase(ptr->indexes.begin(), ptr->indexes.begin() + MININD);

	if (ptr->is_leaf()) {
		newN->prv = ptr->prv;
		ptr->prv->nxt = newN;
		newN->nxt = ptr;
		ptr->prv = newN;

		if (ptr == minNode)
			minNode = newN;

		///Split data if leaves
		newN->data.resize(MININD);
		std::copy(ptr->data.begin(), ptr->data.begin() + MININD, newN->data.begin());
		ptr->data.erase(ptr->data.begin(), ptr->data.begin() + MININD);
	}
	else {
		///Split children if internal Nodes
		newN->children.resize(MININD + 1);
		std::copy(ptr->children.begin(), ptr->children.begin() + MININD + 1, newN->children.begin());
		for (int i = 0; i < newN->children.size(); ++i)
			newN->children[i]->parent = newN;
		ptr->children.erase(ptr->children.begin(), ptr->children.begin() + MININD + 1);
	}

	///Maintain parent of new Node
	int spl = ptr->indexes[0];
	NodePtr parent = ptr->parent;
	if (ptr == root) {
		parent = std::make_shared<BplusNode>();
		parent->indexes.push_back(spl);
		parent->children.push_back(newN);
		parent->children.push_back(ptr);
		ptr->parent = parent;
		if (!ptr->is_leaf())
			ptr->indexes.erase(ptr->indexes.begin());
		root = parent;
	}
	else {
		int j = firstGreater(parent->indexes, spl);
		if (j != parent->indexes.size()) {
			parent->indexes.insert(parent->indexes.begin() + j, spl);
			parent->children.insert(parent->children.begin() + j, newN);
		}
		else {
			parent->indexes.push_back(spl);
			parent->children.insert(parent->children.begin() + parent->children.size() - 1, newN);
		}
	}

	newN->parent = ptr->parent;
}

template<typename T>
void BplusTree<T>::borrowLeft(NodePtr ptr) noexcept {
	NodePtr br = ptr->left();
	ptr->indexes.insert(ptr->indexes.begin(), br->indexes[br->indexes.size() - 1]);
	br->indexes.erase(br->indexes.begin() + br->indexes.size() - 1);

	if (ptr->is_leaf()) {
		ptr->data.insert(ptr->data.begin(), br->data[br->data.size() - 1]);
		br->data.erase(br->data.begin() + br->data.size() - 1);

		int j = firstGreater(ptr->parent->indexes, ptr->indexes[0]);
		ptr->parent->indexes[j] = ptr->indexes[0];
	}
	else {
		ptr->children.insert(ptr->children.begin(), br->children[br->children.size() - 1]);
		br->children.erase(br->children.begin() + br->children.size() - 1);
		ptr->children[0]->parent = ptr;

		int j = firstGreater(ptr->parent->indexes, ptr->indexes[0]);
		std::swap(ptr->parent->indexes[j], ptr->indexes[0]);
	}
}

template<typename T>
void BplusTree<T>::borrowRight(NodePtr ptr) noexcept {
	NodePtr br = ptr->right();
	ptr->indexes.push_back(br->indexes[0]);
	br->indexes.erase(br->indexes.begin());

	if (ptr->is_leaf()) {
		ptr->data.push_back(br->data[0]);
		br->data.erase(br->data.begin());

		int j = firstGreater(ptr->parent->indexes, br->indexes[0]);
		ptr->parent->indexes[j - 1] = br->indexes[0];
	}
	else {
		ptr->children.push_back(br->children[0]);
		br->children.erase(br->children.begin());
		ptr->children[MININD]->parent = ptr;

		int j = firstGreater(ptr->parent->indexes, br->indexes[0]);
		std::swap(ptr->parent->indexes[j], ptr->indexes[ptr->indexes.size() - 1]);
	}
}

template<typename T>
typename BplusTree<T>::NodePtr BplusTree<T>::merge(NodePtr ptr) const noexcept {
	NodePtr br;
	if (ptr->left()) {
		br = ptr->left();
	}
	else {
		br = ptr;
		ptr = ptr->right();
	}

	int os = br->indexes.size();
	br->indexes.resize(os + ptr->indexes.size());
	std::copy(ptr->indexes.begin(), ptr->indexes.end(), br->indexes.begin() + os);

	if (ptr->is_leaf()) {
		br->data.resize(os + ptr->data.size());
		std::copy(ptr->data.begin(), ptr->data.end(), br->data.begin() + os);
		br->nxt = ptr->nxt;
		ptr->nxt->prv = br;
	}
	else {
		br->children.resize(os + ptr->children.size());
		std::copy(ptr->children.begin(), ptr->children.end(), br->children.begin() + os);
		for (int i = os - 1; i < br->children.size(); ++i)
			br->children[i]->parent = br;
	}

	int j = firstGreater(br->parent->indexes, br->indexes[os - 1]);
	br->parent->indexes.erase(br->parent->indexes.begin() + j);
	br->parent->children.erase(br->parent->children.begin() + j + 1);

	return br;
}

template<typename T>
void BplusTree<T>::shrink() noexcept {
	NodePtr root = std::dynamic_pointer_cast<BplusNode>(Tree<T>::root);
	NodePtr ch1 = root->children[0];
	NodePtr ch2 = root->children[1];
	root->children.clear();

	if (ch1->is_leaf()) {
		root->indexes.clear();
		root->indexes.resize(ch1->indexes.size() + ch2->indexes.size());
		std::copy(ch1->indexes.begin(), ch1->indexes.end(), root->indexes.begin());
		std::copy(ch2->indexes.begin(), ch2->indexes.end(), root->indexes.begin() + ch1->indexes.size());

		root->data.resize(ch1->data.size() + ch2->data.size());
		std::copy(ch1->data.begin(), ch1->data.end(), root->data.begin());
		std::copy(ch2->data.begin(), ch2->data.end(), root->data.begin() + ch1->data.size());

		root->nxt = root;
		root->prv = root;
		minNode = root;
	}
	else {
		int rootInd = root->indexes[0];
		root->indexes.resize(ch1->indexes.size() + 1 + ch2->indexes.size());
		std::copy(ch1->indexes.begin(), ch1->indexes.end(), root->indexes.begin());
		root->indexes[ch1->indexes.size()] = rootInd;
		std::copy(ch2->indexes.begin(), ch2->indexes.end(), root->indexes.begin() + ch1->indexes.size() + 1);

		root->children.resize(ch1->children.size() + ch2->children.size());
		std::copy(ch1->children.begin(), ch1->children.end(), root->children.begin());
		std::copy(ch2->children.begin(), ch2->children.end(), root->children.begin() + ch1->children.size());
		for (int i = 0; i < root->children.size(); ++i)
			root->children[i]->parent = root;
	}
}


template<typename T>
void BplusTree<T>::insert(const T& key) {
	int ind = generate();
	if (Tree<T>::root != nullptr) {
		NodePtr root = std::dynamic_pointer_cast<BplusNode>(Tree<T>::root);

		//find leaf to insert
		NodePtr ptr = minNode;
		while (ptr) {
			if (ptr->indexes[0] > ind) {
				if (ptr != minNode)
					ptr = ptr->prv;
				break;
			}
			ptr = std::dynamic_pointer_cast<BplusNode>(ptr->next());
		}
		if (!ptr)
			ptr = minNode->prv;

		//Insert new value into the leaf
		int j = firstGreater(ptr->indexes, ind);
		if (j != ptr->indexes.size()) {
			ptr->indexes.insert(ptr->indexes.begin() + j, ind);
			ptr->data.insert(ptr->data.begin() + j, key);
		}
		else {
			ptr->indexes.push_back(ind);
			ptr->data.push_back(key);
		}

		//Leaf is full, splitting is needed
		if (ptr->indexes.size() == M) {
			while (ptr->indexes.size() == M) {
				split(ptr);
				ptr = ptr->parent;
			}
		}
	}
	//Tree was empty, inserting the first value
	else {
		Tree<T>::root = std::make_shared<BplusNode>();
		auto root = std::dynamic_pointer_cast<BplusNode>(Tree<T>::root);

		root->indexes.push_back(ind);
		root->data.push_back(key);
		root->nxt = root;
		root->prv = root;
		minNode = root;
	}
}

template<typename T>
void BplusTree<T>::print() const noexcept {
	NodePtr root = std::dynamic_pointer_cast<BplusNode>(Tree<T>::root);
	root->print(std::cout);
}

template<typename T>
void BplusTree<T>::remove(const T& key) noexcept {
	NodePtr root = std::dynamic_pointer_cast<BplusNode>(Tree<T>::root);

	BplusIterator<T> it = find(key);
	NodePtr ptr = it.curr_node;

	if (ptr) {
		int ind = ptr->indexes[it.curr_ind];

		bool mult = false;
		if ((it.curr_ind != ptr->indexes.size() - 1) && (ptr->indexes[it.curr_ind + 1] == ind))
			mult = true;
		ptr->indexes.erase(ptr->indexes.begin() + it.curr_ind);
		ptr->data.erase(ptr->data.begin() + it.curr_ind);

		if (ptr == root && root->indexes.size() == 0) {
			root = nullptr;
			return;
		}

		if (ptr->indexes.size() < MININD) {
			while (true) {
				//Borrow from left brother
				if ((ptr->left()) && (ptr->left()->indexes.size() > MININD)) {
					borrowLeft(ptr);
					break;
				}
				//Borrow from right brother
				else if ((ptr->right()) && (ptr->right()->indexes.size() > MININD)) {
					borrowRight(ptr);
					break;
				}
				//Otherwise we can merge with brother and reduce the size of parent
				else if ((ptr->parent != root) || ((ptr->parent == root) && (root->indexes.size() > 1))) {
					ptr = merge(ptr);
					if ((ptr->parent != root) && (ptr->parent->indexes.size() < MININD))
						ptr = ptr->parent;
					else
						break;
				}
				//If nothing helped, reducing the height is needed
				else break;
			}

			//Shrink the tree
			if (ptr->indexes.size() < MININD) {
				shrink();
			}
		}

		//Fix predecessors
		if (!mult) {
			ptr = ptr->parent;
			while (ptr) {
				for (int i = 0; i < ptr->indexes.size(); ++i)
					if (ptr->indexes[i] == ind)
						ptr->indexes[i] = _min(ptr->children[i + 1]);
				ptr = ptr->parent;
			}
		}
	}
}

template<typename T>
BplusIterator<T> BplusTree<T>::find(const T& key) const noexcept {
	NodePtr ptr = minNode;
	int pos = -1;
	while (ptr && (pos == -1)) {
		for (int i = 0; i < ptr->data.size(); ++i)
			if (key == ptr->data[i]) {
				pos = i;
				break;
			}
		ptr = std::dynamic_pointer_cast<BplusNode>(ptr->next());
	}

	if (ptr)
		ptr = ptr->prv;
	else if (!ptr && pos != -1)
		ptr = minNode->prv;

	return BplusIterator<T>(ptr, std::make_shared<ForwardIteration<T>>(), pos);
}


template<typename T>
BplusIterator<T> BplusTree<T>::begin() const noexcept {
	return BplusIterator<T>(minNode, std::make_shared<ForwardIteration<T>>());
}

template<typename T>
BplusIterator<T> BplusTree<T>::end() const noexcept {
	return BplusIterator<T>(nullptr, std::make_shared<ForwardIteration<T>>());
}

template<typename T>
BplusIterator<T> BplusTree<T>::rbegin() const noexcept {
	return BplusIterator<T>(minNode->prv, std::make_shared<ReverseIteration<T>>());
}

template<typename T>
BplusIterator<T> BplusTree<T>::rend() const noexcept {
	return BplusIterator<T>(nullptr, std::make_shared<ReverseIteration<T>>());
}
