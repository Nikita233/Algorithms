#pragma once

#include <functional>
#include <utility>
#include <iostream>
#include <stack>
#include <queue>
#include <memory>
#include <deque>
#include <iterator>
#include <type_traits>
#include <stack>
#include <stdexcept>

namespace lab
{

	template <typename T, typename Func = std::less<T>>
	class PersistentSet
	{

	public:

		PersistentSet();
		explicit PersistentSet(Func);


		bool add(const T&);
		bool add(T&&);

		bool remove(const T& key);

		bool empty() const;

		size_t history_size() const;

		const auto get_roots() const {
			return roots;
		}


		class TreeIterator
			: public std::iterator<std::forward_iterator_tag,
			std::remove_cv_t<T>,
			std::ptrdiff_t,
			const T*,
			const T&>
		{
			using node = typename lab::PersistentSet< std::remove_cv_t<T>, Func>::Nodeptr;
			node itr;
			node nil;
			std::stack<node> path;

			node find_successor(node n)
			{
				n = n->rigth;
				if (n != nil)
				{
					while (n->left != nil)
					{
						path.push(n);
						n = n->left;
					}
				}
				else
				{
					n = path.top();
					path.pop();
				}
				return n;
			}
		public:

			explicit TreeIterator(node n, node pnil) : nil(pnil) //begin
			{
				if (n == nil)
					itr = nil;
				else
				{
					path.push(nil);
					while (n->left != nil)
					{
						path.push(n);
						n = n->left;
					}
					itr = n;
				}
			}
			explicit TreeIterator(node pnil) // end
				: itr(pnil), nil(pnil)
			{ }


			TreeIterator& operator++ ()
			{
				itr = find_successor(itr);
				return *this;
			}
			TreeIterator operator++ (int)
			{
				TreeIterator tmp(*this);
				itr = find_successor(itr);
				return tmp;
			}

			bool operator == (const TreeIterator& rhs) const
			{
				return itr == rhs.itr;
			}

			bool operator != (const TreeIterator& rhs) const
			{
				return itr != rhs.itr;
			}

			const T& operator* () const
			{
				return itr->key;
			}

			const T& operator-> () const
			{
				return itr->key;
			}

		};


		typedef TreeIterator const_iterator;

		const_iterator begin() const
		{
			return begin(roots.size() - 1);
		}
		const_iterator begin(size_t index) const
		{
			if (index >= roots.size())
				throw std::out_of_range("out of range");

			return const_iterator(roots[index], nil);
		}
		const_iterator end() const
		{
			return const_iterator(nil);
		}

	private:

		struct Node;
		using Nodeptr = std::shared_ptr<Node>;

		struct Node
		{
			T key;
			bool isRed;

			Nodeptr left;
			Nodeptr rigth;

			Node(const T& pkey, bool pisRed, Nodeptr pleft, Nodeptr prigth)
				: key(pkey), isRed(pisRed), left(pleft), rigth(prigth)
			{ }

			Node(T&& pkey, bool pisRed, Nodeptr pleft, Nodeptr prigth)
				: key(std::move(pkey)), isRed(pisRed), left(pleft), rigth(prigth)
			{ }
		};

		std::vector<Nodeptr> roots;
		Func cmp;
		Nodeptr nil;

		template <typename TT>
		Nodeptr create_node(TT&& key);

		Nodeptr copy_node(Nodeptr) const;

		template <typename TT>
		bool generic_add(TT&&);

		template <typename TT>
		Nodeptr BST_add_recursive(std::queue<Nodeptr>&, TT&& key, Nodeptr& node);

		void fixed_add(std::queue<Nodeptr>& x);

		template <typename ChildA, typename ChildB>
		void generic_fixed_add(Nodeptr&, Nodeptr&, std::queue<Nodeptr>&, ChildA, ChildB);

		Nodeptr build_path(const T& key, Nodeptr root, std::deque<Nodeptr>& path);

		void delete_node(std::deque<Nodeptr>&);

		Nodeptr build_min_path(Nodeptr node, std::deque<Nodeptr>& path);

		void transplant(Nodeptr p, Nodeptr x, Nodeptr y);

		void fixed_remove(Nodeptr x, std::deque<Nodeptr>& path);

		template <typename ChildA, typename ChildB >

		void generic_fixed_delete(Nodeptr&, Nodeptr&, std::deque<Nodeptr>& path, ChildA, ChildB);

		template <typename ChildA, typename ChildB >
		Nodeptr generic_rotate(Nodeptr, Nodeptr, ChildA, ChildB);

		static Nodeptr& left(Nodeptr x) { return x->left; };

		static Nodeptr& rigth(Nodeptr x) { return x->rigth; };

	};

	template<typename T, typename Func>
	size_t  PersistentSet<T, Func>::history_size() const
	{
		return roots.size();
	}
	template<typename T, typename Func>
	bool PersistentSet<T, Func>::empty() const
	{
		return roots.back() == nil;
	}

	template <typename K, typename Func>
	void PersistentSet<K, Func>::transplant(Nodeptr p, Nodeptr x, Nodeptr y)
	{
		if (p == nil)
		{
			roots.pop_back();
			roots.push_back(y);
		}
		else if (p->left == x)
			p->left = y;
		else
			p->rigth = y;
	}


	template<typename T, typename Func>
	PersistentSet<T, Func>::PersistentSet() : PersistentSet(Func()) {}

	template<typename T, typename Func>
	PersistentSet<T, Func>::PersistentSet(Func pcmp)
		: cmp(pcmp),
		roots(std::vector<Nodeptr>()),
		nil(std::make_shared<Node>(T(), false, nullptr, nullptr))
	{
		roots.push_back(nil);
	}

	template<typename T, typename Func>
	template <typename ChildA, typename ChildB >
	typename  PersistentSet<T, Func>::Nodeptr
		PersistentSet<T, Func>::generic_rotate(

			Nodeptr p,
			Nodeptr x,
			ChildA childA,
			ChildB childB)
	{
		Nodeptr y = childB(x);
		childB(x) = childA(y);

		if (x == roots.back())
		{
			roots.pop_back();
			roots.push_back(y);
		}
		else if (x == childA(p))
			childA(p) = y;
		else
			childB(p) = y;

		childA(y) = x;

		return y;
	}

	template <typename T, typename Func>
	template<typename TT>
	bool PersistentSet<T, Func>::generic_add(TT&& element)
	{

		std::queue<Nodeptr> path;
		auto newRoot = BST_add_recursive(
			path,
			std::forward<TT>(element),
			roots.back()
			);

		bool added = newRoot != nullptr;
		if (added)
		{
			roots.push_back(newRoot);
			path.push(nil);
			fixed_add(path);
		}
		return added;
	}

	template<typename T, typename Func>
	template<typename TT>
	typename PersistentSet<T, Func>::Nodeptr
		PersistentSet<T, Func>::BST_add_recursive(std::queue<Nodeptr>& path, TT&& element, Nodeptr& node)
	{

		if (node == nil)
		{
			auto copy = create_node(std::forward<T>(element));
			path.push(copy);
			return copy;
		}

		bool isless = cmp(element, node->key);
		if (!isless && !cmp(node->key, element))
			return nullptr;

		auto dir = isless ? left : rigth;
		auto child = BST_add_recursive(
			path,
			std::forward<TT>(element),
			dir(node)
			);

		if (child == nullptr) return child;

		auto copy = copy_node(node);
		path.push(copy);
		dir(copy) = child;

		return copy;
	}

	template <typename T, typename Func>
	typename PersistentSet<T, Func>::Nodeptr
		PersistentSet<T, Func>::build_path(const T& element, Nodeptr node, std::deque<Nodeptr>& path)
	{
		if (node == nil)
			return nullptr;

		bool isless = cmp(element, node->key);
		if (!isless && !cmp(node->key, element))
		{
			auto copy = copy_node(node);
			path.push_back(copy);
			return copy;
		}

		auto dir = isless ? left : rigth;
		auto child = build_path(element, dir(node), path);

		if (child == nullptr) return child;

		auto copy = copy_node(node);
		path.push_back(copy);
		dir(copy) = child;

		return copy;
	}

	template <typename T, typename Func>
	bool PersistentSet<T, Func>::add(const T& element)
	{
		return generic_add(const_cast<T&> (element));
	}

	template <typename T, typename Func>
	bool PersistentSet<T, Func>::add(T&& element)
	{
		return generic_add(std::move(element));
	}

	template<typename T, typename Func>
	typename PersistentSet<T, Func>::Nodeptr
		PersistentSet<T, Func>::copy_node(Nodeptr node) const
	{
		if (node == nil) return nil;
		return std::make_shared<Node>(node->key, node->isRed, node->left, node->rigth);
	}

	template <typename T, typename Func>
	template <typename TT>
	typename PersistentSet<T, Func>::Nodeptr
		PersistentSet<T, Func>::create_node(TT&& key)
	{
		return std::make_shared<Node>(std::forward<TT>(key), true, nil, nil);
	}

	template <typename T, typename Func>
	void PersistentSet<T, Func>::delete_node(std::deque<Nodeptr>& path)
	{

		auto z = path.front();
		auto x = z->rigth, y = z;

		if (z->left == nil)
		{
			path.pop_front();
			transplant(path.front(), z, x);
		}
		else if (z->rigth == nil)
		{
			path.pop_front();
			transplant(path.front(), z, x = z->left);
		}
		else
		{
			z->rigth = copy_node(z->rigth);
			y = build_min_path(z->rigth, path);
			x = y->rigth;
			z->key = std::move(y->key);
			transplant(path.front(), y, x);
		}

		if (!y->isRed)
			fixed_remove(x, path);

	}


	template <typename T, typename Func>
	typename PersistentSet<T, Func>::Nodeptr
		PersistentSet<T, Func>::build_min_path(Nodeptr node, std::deque<Nodeptr>& path)
	{
		while (node->left != nil)
		{
			node->left = copy_node(node->left);
			path.push_front(node);
			node = node->left;
		}
		return node;
	}

	template <typename T, typename Func>
	void PersistentSet<T, Func>::fixed_remove(Nodeptr x, std::deque<Nodeptr>& path)
	{

		auto p = path.front();
		path.pop_front();
		while (x != roots.back() && !x->isRed)
		{
			if (p->left == x)
				generic_fixed_delete(x, p, path, left, rigth);
			else
				generic_fixed_delete(x, p, path, rigth, left);
		}

		auto newX = copy_node(x);
		transplant(p, x, newX);
		newX->isRed = false;

	}
	template <typename T, typename Func>
	template <typename ChildA, typename ChildB >
	void PersistentSet<T, Func>::generic_fixed_delete(

		Nodeptr& x,
		Nodeptr& p,
		std::deque<Nodeptr>& path,
		ChildA childA,
		ChildB childB)
	{
		Nodeptr w = childB(p);
		if (w->isRed)
		{
			w = childB(p) = copy_node(w);

			std::swap(w->isRed, p->isRed);
			generic_rotate(path.front(), p, childA, childB);
			path.push_front(w);
			w = childB(p);
		}
		if (!w->left->isRed && !w->rigth->isRed)
		{
			w = childB(p) = copy_node(w);

			w->isRed = true;
			x = p;
			p = path.front();
			path.pop_front();
		}
		else
		{
			if (!childB(w)->isRed)
			{
				w = childB(p) = copy_node(w);

				childA(w) = copy_node(childA(w));
				std::swap(w->isRed, childA(w)->isRed);
				w = generic_rotate(p, w, childB, childA);
			}
			w = childB(p) = copy_node(w);

			childB(w) = copy_node(childB(w));
			w->isRed = p->isRed;
			p->isRed = false;
			childB(w)->isRed = false;
			generic_rotate(path.front(), p, childA, childB);

			x = roots.back();
			p = nil;
		}

	}

	template <typename T, typename Func>
	bool PersistentSet<T, Func>::remove(const T& element)
	{
		std::deque<Nodeptr> dq;

		auto node = build_path(element, roots.back(), dq);
		bool exist = node != nullptr;
		if (exist)
		{
			roots.push_back(node);
			dq.push_back(nil);
			delete_node(dq);
		}
		return exist;
	}


	template <typename T, typename Func>
	void PersistentSet<T, Func>::fixed_add(std::queue<Nodeptr>& path)
	{

		auto x = path.front();
		path.pop();
		auto p = path.front();
		path.pop();

		while (p->isRed)
		{
			if (path.front()->left == p)
				generic_fixed_add(p, x, path, left, rigth);
			else
				generic_fixed_add(p, x, path, rigth, left);
		}
		roots.back()->isRed = false;

	}

	template <typename T, typename Func>
	template <typename ChildA, typename ChildB >
	void PersistentSet<T, Func>::
		generic_fixed_add(Nodeptr& p, Nodeptr& x, std::queue<Nodeptr>& path, ChildA childA, ChildB childB)
	{
		Nodeptr& uncle = childB(path.front());
		if (uncle->isRed)
		{
			uncle = copy_node(uncle);
			childB(path.front()) = uncle;
			p->isRed = false;
			uncle->isRed = false;
			path.front()->isRed = true;

			x = path.front();
			path.pop();
			p = path.front();
			path.pop();
		}
		else
		{
			if (x == childB(p))
			{
				std::swap(x, p);
				generic_rotate(path.front(), x, childA, childB);
			}
			auto gp = path.front();
			path.pop();

			std::swap(gp->isRed, p->isRed);
			generic_rotate(path.front(), gp, childB, childA);
		}
	}

}
