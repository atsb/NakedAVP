#ifndef _included_heap_tem_hpp_
#define _included_heap_tem_hpp_

// Author: Jake Hotson, Rebellion Developments Ltd.

// Previously, the first declaration of these class templates was as friends
// of Ordered_Heap_Member. But Visual C++ 5 will not parse the friend 
// declarations unless we give a forward declaration first. I think this is
// a compiler bug - Garry.
template<class T>
class Ordered_Heap;
template<class T>
class Ordered_Heap_Iterator_Forward;
template<class T>
class Ordered_Heap_Iterator_Backward;

template<class T>
class Ordered_Heap_Member
{
private:
	T data;
	Ordered_Heap_Member<T> * lower, * higher, * parent;
	int num_lower, num_higher;

	// Ordered_Heap_Member() : lower(0), higher(0), parent(0), num_lower(0), num_higher(0) {}
	Ordered_Heap_Member(const T & d) : lower(0), higher(0), parent(0), num_lower(0), num_higher(0), data(d) {}
	
	void delete_tree()
	{
		if (lower)
		{
			lower->delete_tree();
			delete lower;
		}
		if (higher)
		{
			higher->delete_tree();
			delete higher;
		}
	}

	#if 0
	operator T const & () const { return data; }
	operator T & () { return data; }
	#endif

	friend class Ordered_Heap<T>;
	friend class Ordered_Heap_Iterator_Forward<T>;
	friend class Ordered_Heap_Iterator_Backward<T>;
};

// I haven't implemented balancing of the heap on addition or deletion of entries
// so if you build the heap by adding the elements in order, it will be lopsided and inefficient
// if anyone wants to suggest a good way of keeping the heap balanced, please do...

// the operator < must be defined (and should be transitive).
// equivalent elements (!(a<b) && !(b<a)) may both exist in a heap

template<class T>
class Ordered_Heap
{
private:
	Ordered_Heap_Member<T> * lowest, * highest, * root;
	int n_entries;

public:
	// empty heap
	Ordered_Heap() : lowest(0), highest(0), root(0), n_entries(0) {}
	
	// heap with one element
	Ordered_Heap(T const & d)
	{
		lowest = highest = root = new Ordered_Heap_Member<T>(d);
		n_entries = 1;
	}

	// heap with all the elents of another heap
	Ordered_Heap(const Ordered_Heap<T> & h) : lowest(0), highest(0), root(0), n_entries(0) { add_heap(h); }

	// assignment from another heap
	Ordered_Heap<T> & operator = (const Ordered_Heap<T> & h)
	{
		if (&h != this)
		{
			if (root)
			{
				root->delete_tree();
				delete root;
			}
			n_entries = 0;
			root = highest = lowest = 0;
			add_heap(h);
		}
		return *this;
	}

	// deconstructor
	~Ordered_Heap()
	{
		if (root)
		{
			root->delete_tree();
			delete root;
		}
	}

	// add all elements of another heap (merge)
	void add_heap(const Ordered_Heap<T> & h)
	{
		if (h.root) add_heap(*h.root);
	}

private:
	void add_heap(const Ordered_Heap_Member<T> & m)
	{
		add_entry(m.data);
		if (m.lower) add_heap(*m.lower);
		if (m.higher) add_heap(*m.higher);
	}

public:
	// inclued a new element 
	void add_entry(const T & d)
	{
		if (n_entries)
		{
			Ordered_Heap_Member<T> * new_member = new Ordered_Heap_Member<T>(d);
			Ordered_Heap_Member<T> * new_lowest = new_member;
			Ordered_Heap_Member<T> * new_highest = new_member;
			for (Ordered_Heap_Member<T> * m = root;1;)
			{
				if (d < m->data)
				{
					++ m->num_lower;
					if (m->lower)
					{
						m = m->lower;
						new_highest = highest;
					}
					else
					{
						m->lower = new_member;
						new_member->parent = m;
						lowest = new_lowest;
						break;
					}
				}
				else
				{
					++ m->num_higher;
					if (m->higher)
					{
						m = m->higher;
						new_lowest = lowest;
					}
					else
					{
						m->higher = new_member;
						new_member->parent = m;
						highest = new_highest;
						break;
					}
				}
			}
		}
		else
		{
			lowest = highest = root = new Ordered_Heap_Member<T>(d);
		}
		++ n_entries;
	}

	// delete the element a in heap h such that for all b in h : !(b < a)
	void delete_lowest()
	{
		if (1==n_entries)
		{
			delete root;
			n_entries = 0;
			root = lowest = highest = 0;
		}
		else if (n_entries)
		{
			Ordered_Heap_Member<T> * new_lowest;
			if (lowest->parent)
			{
				for (Ordered_Heap_Member<T> * m = lowest; m->parent; m = m->parent)
				{
					-- m->parent->num_lower;
				}
				if (lowest->higher)
				{
					new_lowest = lowest->higher;
					lowest->parent->lower = new_lowest;
					new_lowest->parent = lowest->parent;
					delete lowest;
					while (new_lowest->lower) new_lowest = new_lowest->lower;
				}
				else
				{
					new_lowest = lowest->parent;
					new_lowest->lower = 0;
					delete lowest;
				}
			}
			else // lowest is root
			{
				new_lowest = lowest->higher;
				new_lowest->parent = 0;
				delete lowest;
				root = new_lowest;
				while (new_lowest->lower) new_lowest = new_lowest->lower;
			}
			lowest = new_lowest;
			-- n_entries;
		}
	}

	// delete the element a in heap h such that for all b in h : !(a < b)
	void delete_highest()
	{
		if (1==n_entries)
		{
			delete root;
			n_entries = 0;
			root = lowest = highest = 0;
		}
		else if (n_entries)
		{
			Ordered_Heap_Member<T> * new_highest;
			if (highest->parent)
			{
				for (Ordered_Heap_Member<T> * m = highest; m->parent; m = m->parent)
				{
					-- m->parent->num_higher;
				}
				if (highest->lower)
				{
					new_highest = highest->lower;
					highest->parent->higher = new_highest;
					new_highest->parent = highest->parent;
					delete highest;
					while (new_highest->higher) new_highest = new_highest->higher;
				}
				else
				{
					new_highest = highest->parent;
					new_highest->higher = 0;
					delete highest;
				}
			}
			else // highest is root
			{
				new_highest = highest->lower;
				new_highest->parent = 0;
				delete highest;
				root = new_highest;
				while (new_highest->higher) new_highest = new_highest->higher;
			}
			highest = new_highest;
			-- n_entries;
		}
	}

	Ordered_Heap_Member<T> * equivalent_entry(T const & d)
	{
		Ordered_Heap_Member<T> * m = root;
		
		while (m && (m->data<d || d<m->data))
			m = d<m->data ? m->lower : m->higher;
		
		return m;
	}
	
	void delete_entry_by_pointer(Ordered_Heap_Member<T> * m)
	{
		if (m == lowest) delete_lowest();
		else if (m == highest) delete_highest();
		else if (m->lower)
		{
			Ordered_Heap_Member<T> * new_node = m->lower;
			while (new_node->higher)
			{
				-- new_node->num_higher;
				new_node = new_node->higher;
			}
			if (new_node != m->lower)
			{
				new_node->parent->higher = new_node->lower;
				if (new_node->lower) new_node->lower->parent = new_node->parent;
			}
			else
			{
				m->lower = new_node->lower;
			}
			if (m->parent)
			{
				if (m->parent->higher == m)
				{
					m->parent->higher = new_node;
					-- m->parent->num_higher;
				}
				else
				{
					m->parent->lower = new_node;
					-- m->parent->num_lower;
				}
			}
			else root = new_node;
			if (m->lower) m->lower->parent = new_node;
			if (m->higher) m->higher->parent = new_node;
			new_node->parent = m->parent;
			new_node->lower = m->lower;
			new_node->higher = m->higher;
			new_node->num_lower = m->num_lower-1;
			new_node->num_higher = m->num_higher;
			delete m;
			-- n_entries;
		}
		else
		{
			if (m->parent)
			{
				if (m->parent->higher == m)
				{
					m->parent->higher = m->higher;
					-- m->parent->num_higher;
				}
				else
				{
					m->parent->lower = m->higher;
					-- m->parent->num_lower;
				}
			}
			else root = m->higher;
			if (m->higher) m->higher->parent = m->parent;
			delete m;
			-- n_entries;
		}
	}

	// return the ith element, when i=0 returns the lowest
	T const & operator [](int i) const
	{
		Ordered_Heap_Member<T> * m = root;
		while (m->num_lower != i)
		{
			if (m->num_lower > i) m = m->lower;
			else
			{
				i -= m->num_lower;
				m = m->higher;
				-- i;
			}
		}
		return m->data;
	}

	// return the element a in heap h such that for all b in h : !(b < a)
	T const & lowest_entry() const { return lowest->data; }
	// return the element a in heap h such that for all b in h : !(a < b)
	T const & highest_entry() const { return highest->data; }

	// return the number of elements
	int size() const { return n_entries; }

	friend class Ordered_Heap_Iterator_Forward<T>;
	friend class Ordered_Heap_Iterator_Backward<T>;
};


template<class T>
class Ordered_Heap_Iterator_Forward
{
private:
	Ordered_Heap_Member<T> * m;
	Ordered_Heap<T> const * h;
	Ordered_Heap<T> * hnc;

public:
	// construct with pointer to heap, start at lowest
	Ordered_Heap_Iterator_Forward() {}
	Ordered_Heap_Iterator_Forward(Ordered_Heap<T> const * const h) : h(h), hnc(0), m(h->lowest) {}
	Ordered_Heap_Iterator_Forward(Ordered_Heap<T> * const h) : h(h), hnc(h), m(h->lowest) {}

	int operator == (Ordered_Heap_Iterator_Forward const & i2) const { return m==i2.m; }
	int operator != (Ordered_Heap_Iterator_Forward const & i2) const { return m!=i2.m; }

	// move to next highest, sequentially
	void next()
	{
		if (m->higher)
		{
			m = m->higher;
			while (m->lower) m = m->lower;
		}
		else
		{
			while (m->parent)
			{
				if (m->parent->lower == m) break;
				m = m->parent;
			}
			if (m->parent) m = m->parent;
			else m = 0;
		}
	}

	// undo a next operation
	void un_next()
	{
		if (m->lower)
		{
			m = m->lower;
			while (m->higher) m = m->higher;
		}
		else
		{
			while (m->parent)
			{
				if (m->parent->higher == m) break;
				m = m->parent;
			}
			if (m->parent) m = m->parent;
			else m = 0;
		}
	}

	// return the current element
	T const & operator() () const { return m->data; }
	operator T const & () const { return m->data; }

	// delete the current element
	// unpredictable if iterator constructed from a constant heap
	void delete_current()
	{
		Ordered_Heap_Member<T> * old_m = m;
		next();
		hnc->delete_entry_by_pointer(old_m);
	}

	// change the current element, checking that the ordering is preserved
	int change_current(T const & new_val) // be very careful with this -- check the return value which is non-zero on success
	{
		Ordered_Heap_Member<T> * old_m = m;
		next();
		Ordered_Heap_Member<T> * higher_m = m;
		m = old_m;
		un_next();
		Ordered_Heap_Member<T> * lower_m = m;
		m = old_m;

		if (lower_m) if (new_val < lower_m->data) { return 0; }
		if (higher_m) if (higher_m->data < new_val) { return 0; }
		m->data = new_val;
		return 1;
	}

	// have we gone past the highest?
	int done() const { return m ? 0 : 1; }

	// start again at lowest
	void restart() { m = h->lowest; }
};

#define OHIF Ordered_Heap_Iterator_Forward


template<class T>
class Ordered_Heap_Iterator_Backward
{
private:
	Ordered_Heap_Member<T> * m;
	Ordered_Heap<T> const * h;
	Ordered_Heap<T> * hnc;

public:
	Ordered_Heap_Iterator_Backward() {}
	Ordered_Heap_Iterator_Backward(Ordered_Heap<T> const * const h) : h(h), hnc(0), m(h->highest) {}
	Ordered_Heap_Iterator_Backward(Ordered_Heap<T> * const h) : h(h), hnc(h), m(h->highest) {}

	int operator == (Ordered_Heap_Iterator_Backward const & i2) const { return m==i2.m; }
	int operator != (Ordered_Heap_Iterator_Backward const & i2) const { return m!=i2.m; }

	void next()
	{
		if (m->lower)
		{
			m = m->lower;
			while (m->higher) m = m->higher;
		}
		else
		{
			while (m->parent)
			{
				if (m->parent->higher == m) break;
				m = m->parent;
			}
			if (m->parent) m = m->parent;
			else m = 0;
		}
	}

	void un_next()
	{
		if (m->higher)
		{
			m = m->higher;
			while (m->lower) m = m->lower;
		}
		else
		{
			while (m->parent)
			{
				if (m->parent->lower == m) break;
				m = m->parent;
			}
			if (m->parent) m = m->parent;
			else m = 0;
		}
	}

	T const & operator() () const { return m->data; }
	operator T const & () const { return m->data; }

	void delete_current()
	{
		Ordered_Heap_Member<T> * old_m = m;
		next();
		hnc->delete_entry_by_pointer(old_m);
	}

	int change_current(T const & new_val) // be very careful with this -- check the return value which is non-zero on success
	{
		Ordered_Heap_Member<T> * old_m = m;
		next();
		Ordered_Heap_Member<T> * lower_m = m;
		m = old_m;
		un_next();
		Ordered_Heap_Member<T> * higher_m = m;
		m = old_m;

		if (lower_m) if (new_val < lower_m->data) { return 0; }
		if (higher_m) if (higher_m->data < new_val) { return 0; }
		m->data = new_val;
		return 1;
	}

	int done() const { return m ? 0 : 1; }
	void restart() { m = h->highest; }
};

#define OHIB Ordered_Heap_Iterator_Backward

#endif // !_included_heap_tem_hpp_
