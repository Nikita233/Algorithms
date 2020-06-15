struct FibHeapNode
{
	int key; // assume the element is int...
	FibHeapNode* left;
	FibHeapNode* right;
	FibHeapNode* parent;
	FibHeapNode* child;
	int degree;
	bool mark;
};


class FibHeap {
public:
	FibHeapNode* m_minNode;
	int m_numOfNodes;

	FibHeap() {  // initialize a new and empty Fib Heap
		m_minNode = nullptr;
		m_numOfNodes = 0;
	}

	~FibHeap() {
		_clear(m_minNode);
	}

	/* Insert a node with key value new_key
	   and return the inserted node*/
	FibHeapNode* insert(int newKey);

	/* Merge current heap with another*/
	void merge(FibHeap& another);

	/* Return the key of the minimum node*/
	int  extract_min();

	/* Decrease the key of node x to newKey*/
	void decrease_key(FibHeapNode* x, int newKey);

	/*Delete a specified node*/
	void delete_node(FibHeapNode* x);

private:
	static const int m_minimumKey;
	FibHeapNode* _create_node(int newKey);
	void _insert_node(FibHeapNode* newNode);
	void _remove_from_circular_list(FibHeapNode* x);
	FibHeapNode* _merge(FibHeapNode* a, FibHeapNode* b);
	void _make_child(FibHeapNode* child, FibHeapNode* parent);
	void _consolidate();
	void _unparent_all(FibHeapNode* x);
	FibHeapNode* _extract_min_node();
	void _decrease_key(FibHeapNode* x, int newKey);
	void _cut(FibHeapNode* x, FibHeapNode* y);
	void _cascading_cut(FibHeapNode* y);
	void _clear(FibHeapNode* x);
};


const int FibHeap::m_minimumKey = 0x80000000; // the minimum int value

FibHeapNode* FibHeap::insert(int newKey)
{
	FibHeapNode* newNode = _create_node(newKey);
	_insert_node(newNode);
	return newNode;
}

void FibHeap::merge(FibHeap& another)
{
	m_minNode = _merge(m_minNode, another.m_minNode);
	m_numOfNodes += another.m_numOfNodes;
	another.m_minNode = nullptr; // so that another 
	another.m_numOfNodes = 0;
}

int  FibHeap::extract_min()
{
	FibHeapNode* minNode = _extract_min_node();
	int ret = minNode->key;
	delete minNode;
	return ret;
}

void FibHeap::decrease_key(FibHeapNode* x, int newKey)
{
	_decrease_key(x, newKey);
}

void FibHeap::delete_node(FibHeapNode* x)
{
	_decrease_key(x, m_minimumKey);
	extract_min();
}

FibHeapNode* FibHeap::_create_node(int newKey)
{
	FibHeapNode* newNode = new FibHeapNode;
	newNode->key = newKey;
	newNode->left = newNode;
	newNode->right = newNode;
	newNode->parent = nullptr;
	newNode->child = nullptr;
	newNode->degree = 0;
	newNode->mark = false;
	return newNode;
}


void FibHeap::_insert_node(FibHeapNode* newNode)
{
	m_minNode = _merge(m_minNode, newNode);
	m_numOfNodes++;
}

void FibHeap::_remove_from_circular_list(FibHeapNode* x)
{
	if (x->right == x) // the root list only has one node before the operation
	{
		return;
	}
	FibHeapNode* leftSib = x->left;
	FibHeapNode* rightSib = x->right;
	leftSib->right = rightSib;
	rightSib->left = leftSib;
}


FibHeapNode* FibHeap::_merge(FibHeapNode* a, FibHeapNode* b)
{
	if (a == nullptr)
		return b;
	if (b == nullptr)
		return a;
	if (a->key > b->key) // swap node 
	{
		FibHeapNode* temp = a;
		a = b;
		b = temp;
	}
	FibHeapNode* aRight = a->right;
	FibHeapNode* bLeft = b->left;
	a->right = b;
	b->left = a;
	aRight->left = bLeft;
	bLeft->right = aRight;
	return a;
}

FibHeapNode* FibHeap::_extract_min_node()
{
	FibHeapNode* min = m_minNode;
	if (min != nullptr) // the heap is not empty
	{
		_unparent_all(min->child);
		_merge(min, min->child); // merge the child circular list into root
		_remove_from_circular_list(min);
		if (min == min->right) // the heap will be empty after the operation
			m_minNode = nullptr;
		else
		{
			m_minNode = min->right; // minNode need not be the minimum at this time
			_consolidate();
		}
		m_numOfNodes--;
	}
	return min;
}

void FibHeap::_unparent_all(FibHeapNode* x)
{
	if (x == nullptr)
		return;
	FibHeapNode* y = x;
	do {
		y->parent = nullptr;
		y = y->right;
	} while (y != x);
}


void FibHeap::_consolidate()
{
	int Dn = (int)(log2(m_numOfNodes) / log2(1.618));
	FibHeapNode** A = new FibHeapNode * [Dn + 1];
	for (int i = 0; i < Dn + 1; i++)
		A[i] = nullptr;
	FibHeapNode* x = m_minNode;
	bool breakFlag = false;
	while (true)
	{
		int d = x->degree;
		while (A[d] != nullptr)
		{
			FibHeapNode* y = A[d];
			if (y == x)
			{
				breakFlag = true;  // when y == x, all root nodes have different degree
				break;			   // so break out of the whole loop
			}
			if (x->key > y->key) // swap x and y, so x always points to the 
								// node with smaller key
			{
				FibHeapNode* temp = x;
				x = y;
				y = temp;
			}
			_make_child(y, x); // make y the child of x
			A[d++] = nullptr; // now the new node has (d + 1) child, so A[d] = nullptr,d = d + 1  
		}
		if (breakFlag)
			break;
		A[x->degree] = x;
		x = x->right; // to next node in the root list
	}
	m_minNode = x;  // update the m_minNode
	FibHeapNode* iter = x;
	do {
		if (iter->key < m_minNode->key)
			m_minNode = iter;
		iter = iter->right;
	} while (iter != x);
	delete[]A;
}

void FibHeap::_make_child(FibHeapNode* child, FibHeapNode* parent)
{
	_remove_from_circular_list(child);
	child->left = child->right = child;
	child->parent = parent;
	parent->child = _merge(parent->child, child); // add child into parent's children list
	parent->degree++;
	child->mark = false;
}

void FibHeap::_decrease_key(FibHeapNode* x, int newKey)
{
	x->key = newKey;
	FibHeapNode* y = x->parent;
	if (y != nullptr && x->key < y->key)
	{
		_cut(x, y);
		_cascading_cut(y);
	}
	if (x->key < m_minNode->key)
		m_minNode = x;
}

void FibHeap::_cut(FibHeapNode* x, FibHeapNode* y)
{
	_remove_from_circular_list(x);
	if (x->right == x) // x is the only child of y
	{
		y->child = nullptr;
	}
	else
	{
		y->child = x->right; // update y's child 
	}
	y->degree--;
	_merge(m_minNode, x);
	x->parent = nullptr;
	x->mark = false;
}

void FibHeap::_cascading_cut(FibHeapNode* y)
{
	FibHeapNode* z = y->parent;
	if (z != nullptr)
	{
		if (y->mark == false)
			y->mark = true;
		else
		{
			_cut(y, z);
			_cascading_cut(z);
		}
	}
}

void FibHeap::_clear(FibHeapNode* x)
{
	if (x != nullptr)
	{
		FibHeapNode* t1 = x;
		do {
			FibHeapNode* t2 = t1;
			t1 = t1->right;
			_clear(t2->child);
			delete t2;
		} while (t1 != x);
	}
}

}
