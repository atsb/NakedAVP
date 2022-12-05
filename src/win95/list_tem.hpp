// Doubly linked list class.
//
// Inserts new entries at the *end* of the list -- so add_entry()
// followed by delete_last_entry() will do nothing.
//
//
// Usage:
//
// List<Thing> l;               creates an empty list.
// Thing a; List<Thing> l(a);   creates a list containing a.
//      
// void l.add_entry(Thing a);        adds an entry.
// void l.delete_entry(Thing a);     removes `a' from the list if it's there, 
//                                   aborts if it's not.
// void l.delete_{first,last}_entry  deletes the first/last entry.
// Thing l.{first,last}_entry        returns the first/last entry.
// bool l.contains(Thing a)          is a in the list?
#ifndef list_template_hpp
#define list_template_hpp

//#pragma once

#include <stdio.h>

#ifdef _CPPRTTI // run time type information available
	#include <typeinfo.h>
	#define LIST_TEM_TYPEID_THIS typeid(*this).name()
#else
	#define LIST_TEM_TYPEID_THIS "?"
#endif

#include "mem3dc.h"

#ifdef NDEBUG
	static void fail(...) {}
	#define list_fail_get_data_from_sentinel NULL
	#define list_fail_add_entry_after NULL
	#define list_fail_add_entry_before NULL
	#define list_fail_delete_entry NULL
	#define list_fail_delete_entry_by_pointer NULL
	#define list_fail_alter_entry NULL
	#define list_fail_next_entry_nonexist NULL
	#define list_fail_next_entry_sentinel NULL
	#define list_fail_prev_entry_nonexist NULL
	#define list_fail_prev_entry_sentinel NULL
	#define list_fail_last_entry NULL
	#define list_fail_first_entry NULL
	#define list_fail_similar_entry NULL
	#define list_fail_delete_last_entry NULL
	#define list_fail_delete_first_entry NULL
	#define list_fail_operator NULL
	#define lit_fail_next NULL
	#define lit_fail_operator NULL
	#define lit_fail_delete_current NULL
	#define lit_fail_change_current NULL
#else
	#include "fail.h"
	extern char const * list_fail_get_data_from_sentinel;
	extern char const * list_fail_add_entry_after;
	extern char const * list_fail_add_entry_before;
	extern char const * list_fail_delete_entry;
	extern char const * list_fail_delete_entry_by_pointer;
	extern char const * list_fail_alter_entry;
	extern char const * list_fail_next_entry_nonexist;
	extern char const * list_fail_next_entry_sentinel;
	extern char const * list_fail_prev_entry_nonexist;
	extern char const * list_fail_prev_entry_sentinel;
	extern char const * list_fail_last_entry;
	extern char const * list_fail_first_entry;
	extern char const * list_fail_similar_entry;
	extern char const * list_fail_delete_last_entry;
	extern char const * list_fail_delete_first_entry;
	extern char const * list_fail_operator;
	extern char const * lit_fail_next;
	extern char const * lit_fail_operator;
	extern char const * lit_fail_delete_current;
	extern char const * lit_fail_change_current;
#endif

// The first declaration of these class templates was previously as friends
// of List. However, Visual C++ 5 can't parse them unless we give a 
// forward declaration first - I think this is a compiler bug - Garry.
template<class T>
class List_Iterator_Forward;
template<class T>
class ConstList_Iterator_Forward;
template<class T>
class List_Iterator_Backward;
template<class T>
class ConstList_Iterator_Backward;
template<class T>
class List_Iterator_Loop;

template<class T>
struct List_Member;

template<class T>
struct List_Member_Base
{
	union
	{
		List_Member_Base<T> *prev;
	   	List_Member<T> *prev_debug; // encourage the debugger to display the list members data
		                            // hopefully casting from base to derived class would not
		                            // cause the actual value of the ptr to change, so the debugger
		                            // will display the information correctly, and this union
		                            // won't cause any kind of performance hit
	};
	union
	{
		List_Member_Base<T> *next;
		List_Member<T> *next_debug;
	};
	virtual ~List_Member_Base() {}
};

template<class T>
struct List_Member : public List_Member_Base<T>
{
  T data;
  List_Member<T>(const T& n) : data(n) {}
};

template<class T>
class List {
private:
  List_Member_Base<T> *sentinel;
  int n_entries;
  mutable T **entry_pointers;
  mutable bool calculated_indices;

  T& data(List_Member_Base<T>* e) const
  {
    if (e == sentinel)
      fail(list_fail_get_data_from_sentinel,LIST_TEM_TYPEID_THIS);
    return ((List_Member<T>*)e)->data;
  }

public:
  List() {
    sentinel = new List_Member_Base<T>;

    sentinel->next = sentinel;
    sentinel->prev = sentinel;
        
    n_entries = 0;
    entry_pointers = 0;
    calculated_indices = false;
  }

  List(const T& n) {
    sentinel = new List_Member_Base<T>;
    sentinel->next = sentinel;
    sentinel->prev = sentinel;
    n_entries = 0;       
    entry_pointers = 0;
    calculated_indices = false;

    add_entry(n);
  }

  List(const List<T>& l) {
    sentinel = new List_Member_Base<T>;

    sentinel->next = sentinel;
    sentinel->prev = sentinel;
    n_entries = 0;

    entry_pointers = 0;
    calculated_indices = false;

    List_Member_Base<T>* m = l.sentinel->next;
    while (m != l.sentinel) {
      add_entry(data(m));
      m = m->next;
    }

  }  

  List<T>& operator= (const List<T>& l) {
    while(n_entries != 0) delete_last_entry();
    if (entry_pointers != 0)
      delete[] entry_pointers;

    List_Member_Base<T>* m = l.sentinel->next;
    while (m != l.sentinel) {
      add_entry(data(m));
      m = m->next;
    }
    calculated_indices = false;
    entry_pointers = 0;
    return *this;
  }
    
  ~List() {
    while (n_entries != 0) delete_last_entry();
    delete sentinel;
    delete[] entry_pointers; 
  }

  void add_entry(const T& n) {
    add_entry_end(n);
  }

  void add_entry_end(const T& n) {
    List_Member<T> *e = new List_Member<T>(n);
    e->next = sentinel;
    e->prev = sentinel->prev;
    sentinel->prev->next = e;
    sentinel->prev = e;
    n_entries++;
    cleanup();
  }

  void add_entry_start(const T& n) {
    List_Member<T> *e = new List_Member<T>(n);
    e->prev = sentinel;
    e->next = sentinel->next;
    sentinel->next->prev = e;
    sentinel->next = e;
    n_entries++;
    cleanup();
  }

  void add_entry_after(const T& n, const T& d) {
    List_Member_Base<T> *f = sentinel->next;
    while (f != sentinel && data(f) != d) {
      f = f->next;
    }
    if (f == sentinel) {
      fail(list_fail_add_entry_after,LIST_TEM_TYPEID_THIS);
    } else {
      List_Member<T> *e = new List_Member<T>(n);
      e->next = f->next;
      e->prev = f;
      e->next->prev = e;
      f->next = e;
      n_entries++;
    }
    cleanup();
  }

  void add_entry_before(const T& n, const T& d) {
    List_Member_Base<T> *f = sentinel->next;
    while (f != sentinel && data(f) != d) {
      f = f->next;
    }
    if (f == sentinel) {
      fail(list_fail_add_entry_before,LIST_TEM_TYPEID_THIS);
    } else {
      List_Member<T> *e = new List_Member<T>(n);
      e->prev = f->prev;
      e->next = f;
      e->prev->next = e;
      f->prev = e;
      n_entries++;
    }
    cleanup();
  }

  void delete_entry(const T& d) {
    List_Member_Base<T> *e = sentinel->next;
    while (e != sentinel && data(e) != d && e != sentinel) {
      e = e->next;
    }
    if (e == sentinel) {
      fail(list_fail_delete_entry,LIST_TEM_TYPEID_THIS);
    } else {
      e->prev->next = e->next;
      e->next->prev = e->prev;
      delete e;
    }
    n_entries--;
    cleanup();
  }

  void delete_entry_backward(const T& d) {
    List_Member_Base<T> *e = sentinel->prev;
    while (e != sentinel && data(e) != d && e != sentinel) {
      e = e->prev;
    }
    if (e == sentinel) {
      fail(list_fail_delete_entry,LIST_TEM_TYPEID_THIS);
    } else {
      e->prev->next = e->next;
      e->next->prev = e->prev;
      delete e;
    }
    n_entries--;
    cleanup();
  }

  void delete_entry_by_pointer(List_Member_Base<T>* l) {
    if (l == sentinel)
      fail(list_fail_delete_entry_by_pointer,LIST_TEM_TYPEID_THIS);
    l->next->prev = l->prev;
    l->prev->next = l->next;
    delete l;
    l = 0; // so we get a seg if we try and reuse it
    n_entries--;
    cleanup();
  }

  void alter_entry(const T& od, const T& nd) {
    List_Member_Base<T> *e = sentinel->next;
    while (e != sentinel && data(e) != od) {
      e = e->next;
    }
    if (e == sentinel) {
      fail(list_fail_alter_entry,LIST_TEM_TYPEID_THIS);
    } else {
      // Remove this entry, and put a new one in it's place. We can't
      // just do e->data = nd, because that's assignment, and we don't
      // want to require an assignment operator to be defined for
      // every thing that we put on a list.
      List_Member<T> * n = new List_Member<T>(nd);
      e->prev->next = n;
      e->next->prev = n;
      n->next = e->next;
      n->prev = e->prev;
      delete e;
    }
    cleanup();
  }

  T next_entry(const T& d) const {
    List_Member_Base<T> *e = sentinel->next;
    while (e != sentinel && data(e) != d && e != sentinel) {
      e = e->next;
    }
    if (e == sentinel) {
      fail(list_fail_next_entry_nonexist,LIST_TEM_TYPEID_THIS);
    } else { 
      if (e->next == sentinel)
        fail(list_fail_next_entry_sentinel,LIST_TEM_TYPEID_THIS);
      return data(e->next);
    }
	  return data(e->next);
  }

  T prev_entry(const T& d) const {
    List_Member_Base<T> *e = sentinel->next;
    while (e!= sentinel && data(e) != d) {
      e = e->next;
    }
    if (e == sentinel) {
      fail(list_fail_prev_entry_nonexist,LIST_TEM_TYPEID_THIS);
    } else { 
      if (e->prev == sentinel)
        fail(list_fail_prev_entry_sentinel,LIST_TEM_TYPEID_THIS);
      return data(e->prev);
    }
      return data(e->prev);
  }

  T const & similar_entry(T const& d) const
  {
      List_Member_Base<T> *e = sentinel->next;
      while (e != sentinel)
      {
          if (data(e) == d)
          	break;
          e = e->next;
		  
      }
      if (e == sentinel) 
      {
          fail(list_fail_similar_entry,LIST_TEM_TYPEID_THIS);
      } 
      return data(e);
  }
  
  bool contains(const T& d) {
    List_Member_Base<T> *e = sentinel->next;
    while (e != sentinel && data(e) != d) {
      e = e->next;
    }
    if (e == sentinel) return false;
    else               return true;
  }

  void delete_last_entry() {
    if (sentinel->prev == sentinel) {
      fail(list_fail_delete_last_entry,LIST_TEM_TYPEID_THIS);
    } else {
      // aiee. These lines work, but are a bit hairy.

      sentinel->prev = sentinel->prev->prev;
      delete sentinel->prev->next;
      sentinel->prev->next = sentinel;
      n_entries--;
    }
    cleanup();
  }

  void delete_first_entry() {
    if (sentinel->next == sentinel) {
      fail(list_fail_delete_last_entry,LIST_TEM_TYPEID_THIS);
    } else {
      sentinel->next = sentinel->next->next;
      delete sentinel->next->prev;
      sentinel->next->prev = sentinel;
      n_entries--;
    }
    cleanup();
  }

  T const & operator[](int i) const {
    if (i < 0 || i >= n_entries)
      fail(list_fail_operator,LIST_TEM_TYPEID_THIS, i+1, n_entries);

    if (!calculated_indices) {
      if (entry_pointers != 0)
        delete[] entry_pointers;
      entry_pointers = new T*[n_entries+1];
      List_Member_Base<T>*e = sentinel->next;
      int j = 0;
      while (e != sentinel) {
        entry_pointers[j] = &data(e);
        e = e->next;
        j++;
      }
      calculated_indices = true;
    }
    return *entry_pointers[i];
  }
    
  T & last_entry() 
  {
    if (n_entries == 0)
      fail(list_fail_last_entry,LIST_TEM_TYPEID_THIS);
    return data(sentinel->prev);
  }

  T const & last_entry() const
  {
    if (n_entries == 0)
      fail(list_fail_last_entry,LIST_TEM_TYPEID_THIS);
    return data(sentinel->prev);
  }

  T & first_entry() 
  {
    if (n_entries == 0)
      fail(list_fail_first_entry,LIST_TEM_TYPEID_THIS);
    return data(sentinel->next);
  }

  T const & first_entry() const 
  {
    if (n_entries == 0)
      fail(list_fail_first_entry,LIST_TEM_TYPEID_THIS);
    return data(sentinel->next);
  }
    
  int size() const { return n_entries; }

  void cleanup() { calculated_indices = false; if (entry_pointers != 0) delete[] entry_pointers; entry_pointers = 0;}

  bool operator==(const List <T> &l1) const
  {
    if (n_entries != l1.n_entries) return false;
    for (List_Member_Base<T> * e = sentinel->next, *e1 = l1.sentinel->next; e != sentinel; e = e->next, e1 = e1->next)
    {
      if (((List_Member<T> *)e)->data != ((List_Member<T> *)e1)->data) return false;
    }
    return true;
  }

  bool operator!=(const List <T> &l1) const
  {
    if (n_entries != l1.n_entries) return true;
    for (List_Member_Base<T> * e = sentinel->next, *e1 = l1.sentinel->next; e != sentinel; e = e->next, e1 = e1->next)
    {
      if (((List_Member<T> *)e)->data != ((List_Member<T> *)e1)->data) return true;
    }
    return false;
  }

    friend class List_Iterator_Forward<T>;
    friend class ConstList_Iterator_Forward<T>;
    friend class List_Iterator_Backward<T>;
    friend class ConstList_Iterator_Backward<T>;
    friend class List_Iterator_Loop<T>;
};

// Use List_Iterator_{Forward,Backward} as follows:
//
//     for(List_Iterator_Forward<T*> oi(&(List_of_T*)); // a _pointer_ 
//                                                      // to the list.
//           !oi.done();
//           oi.next()    )    {
//          do_something( oi() ) ;
//     }
//
// First entry is the one past the sentinel, and next() and
// operator() fail if you try and iterate too far; check if it's
// done() before doing anything else, basically.
//
// As it's a doubly-linked list, we can go backwards and
// forwards. next() takes us to the entry that the type of iterator
// suggests; un_next() takes us in the other direction. un_next() is
// an ugly name, but next() and prev() are too suggestive of a
// particular direction.

template<class T>
class List_Iterator_Forward
{
private:
  union
  {
    List_Member_Base<T> *m;
    List_Member<T> *m_debug; // encourage the debugger to display the list members data
                             // hopefully casting from base to derived class would not
                             // cause the actual value of the ptr to change, so the debugger
                             // will display the information correctly, and this union
                             // won't cause any kind of performance hit
  };
  List<T> *l;

public:
  List_Iterator_Forward() {}
  List_Iterator_Forward(List<T> *list) { l = list; m = l->sentinel->next; }
   
  void next() {
    if (m != l->sentinel) {
      m = m->next;
    } else
      fail(lit_fail_next,LIST_TEM_TYPEID_THIS);
  }
    
  void un_next() {
    if (m != l->sentinel) {
      m = m->prev;
    } else 
      fail(lit_fail_next,LIST_TEM_TYPEID_THIS);
  }
  
  T & operator() () const {
      if (m == l->sentinel)
          fail(lit_fail_operator,LIST_TEM_TYPEID_THIS);
      return l->data(m);
  }

  void delete_current() {
    if (m != l->sentinel) {
      m = m->next;
      l->delete_entry_by_pointer(m->prev);
    } else
      fail(lit_fail_delete_current,LIST_TEM_TYPEID_THIS);
  }

  void change_current(T const &new_val) const {
    if (m != l->sentinel) {
      // Delete the current member out of the list, put a new one in.

      List_Member<T> * n = new List_Member<T>(new_val);
      m->prev->next = n;
      m->next->prev = n;
      n->next = m->next;
      n->prev = m->prev;
      delete m;
      *(List_Member_Base<T> **)&m =n; // or we're pointing at the thing we just deleted.
      l->cleanup(); // because it's changed, but the List doesn't know that.
    } else
      fail(lit_fail_change_current,LIST_TEM_TYPEID_THIS);
  }

  bool done() { if (m == l->sentinel) return true; else return false; }
  void restart() { m = l->sentinel->next; }       
  // Go to the end of the list.
  void end() { m = l->sentinel->prev; }
};

#define LIF List_Iterator_Forward

template<class T>
class ConstList_Iterator_Forward
{
  private:
  union
  {
    List_Member_Base<T> *m;
    List_Member<T> *m_debug; // encourage the debugger to display the list members data
  };
    List<T> const *l;

  public:
    ConstList_Iterator_Forward(List<T> const *list) { l = list; m = l->sentinel->next; }
    ConstList_Iterator_Forward(){}
   
    void next() {
        if (m == l->sentinel) {
            fail(lit_fail_next,LIST_TEM_TYPEID_THIS);
        }
        m = m->next;
    }
    
    void un_next() {
        if (m == l->sentinel) {
            fail(lit_fail_next,LIST_TEM_TYPEID_THIS);
        }
        m = m->prev;
    }

    T const & operator() () const {
        if (m == l->sentinel)
            fail(lit_fail_operator,LIST_TEM_TYPEID_THIS);
        return l->data(m);
    }

    #if 0 // shouldn't really be available on a const list
    void change_current(T const & new_val) const {
        if (m != l->sentinel) {
            m->data = new_val;
        } else
            fail(lit_fail_change_current,LIST_TEM_TYPEID_THIS);
    }
    #endif

    bool done() const { if (m == l->sentinel) return true; else return false; }
    void restart() { m = l->sentinel->next; }       
	// Go to the end of the list.
	void end() { m = l->sentinel->prev; }
};

#define CLIF ConstList_Iterator_Forward

template<class T>
class List_Iterator_Backward
{
private:
  union
  {
    List_Member_Base<T> *m;
    List_Member<T> *m_debug; // encourage the debugger to display the list members data
  };
  List<T> *l;

public:
  List_Iterator_Backward() {}
  List_Iterator_Backward(List<T> *list) { l = list; m = l->sentinel->prev; }
   
  void next() {
    if (m != l->sentinel) {
      m = m->prev;
    } else
      fail(lit_fail_next,LIST_TEM_TYPEID_THIS);
  }

  void un_next() {
    if (m != l->sentinel) {
      m = m->next;
    } else
      fail(lit_fail_next,LIST_TEM_TYPEID_THIS);
  }

  T & operator() () const {
      if (m == l->sentinel)
          fail(lit_fail_operator,LIST_TEM_TYPEID_THIS);
      return l->data(m);
  }

  void delete_current() {
    if (m != l->sentinel) {
      m = m->prev;
      l->delete_entry_by_pointer(m->next);
    } else
      fail(lit_fail_delete_current,LIST_TEM_TYPEID_THIS);
  }

  void change_current(T const & new_val)  {
    if (m != l->sentinel) {
      // Delete the current member out of the list, put a new one in.

      List_Member<T> * n = new List_Member<T>(new_val);
      m->prev->next = n;
      m->next->prev = n;
      n->next = m->next;
      n->prev = m->prev;
      delete m;
      m = n; // or we're pointing at the thing we just deleted.
      l->cleanup(); // because it's changed, but the List doesn't know that.
    } else
      fail(lit_fail_change_current,LIST_TEM_TYPEID_THIS);
  }

  bool done() { if (m == l->sentinel) return true; else return false; }
  void restart() { m = l->sentinel->prev; }
  // Go to the end of the list.
  void end() { m = l->sentinel->prev; }
};

#define LIB List_Iterator_Backward

template<class T>
class ConstList_Iterator_Backward
{
  private:
  union
  {
    List_Member_Base<T> *m;
    List_Member<T> *m_debug; // encourage the debugger to display the list members data
  };
    List<T> const *l;

  public:
    ConstList_Iterator_Backward(List<T> const *list) { l = list; m = l->sentinel->prev; }
    ConstList_Iterator_Backward(){}
   
    void next() {
        if (m == l->sentinel) {
            fail(lit_fail_next,LIST_TEM_TYPEID_THIS);
        }
        m = m->prev;
    }

    void un_next() {
        if (m == l->sentinel) {
            fail(lit_fail_next,LIST_TEM_TYPEID_THIS);
        }
        m = m->next;
    }

    T const & operator() () const {
        if (m == l->sentinel)
            fail(lit_fail_operator,LIST_TEM_TYPEID_THIS);
        return l->data(m);
    }

    #if 0 // shouldn't really be available on a const list
    void change_current(T const & new_val) const {
        if (m != l->sentinel) {
            m->data = new_val;
        } else
            fail(lit_fail_change_current,LIST_TEM_TYPEID_THIS);
    }
    #endif

    bool done() const { if (m == l->sentinel) return true; else return false; }
    void restart() { m = l->sentinel->prev; }
	// Go to the end of the list.
	void end() { m = l->sentinel->prev; }
};

#define CLIB ConstList_Iterator_Backward

/*
	A looping list iterator class :
		next from the last member will go to the first
		previous from the first member will go to the last
*/

template<class T>
class List_Iterator_Loop
{
  private:
  union
  {
    List_Member_Base<T> *m;
    List_Member<T> *m_debug; // encourage the debugger to display the list members data
  };
    List<T> *l;

  public:
    List_Iterator_Loop(List<T> *list) { l = list; m = l->sentinel->next; }
   
    void next() {
        m=m->next;
        if (m == l->sentinel) {
            m = m->next;
        } 
    }
    
    void previous() {
        m=m->prev;
        if (m == l->sentinel) {
            m = m->prev;
        }  
    }

    T const & operator() ()  {
        if (m == l->sentinel)
		{
			m=m->next;//needed in case iterator was created before anything was added to the list
	        if (m == l->sentinel)
			{
				fail(lit_fail_operator,LIST_TEM_TYPEID_THIS);
			}
		}
        return l->data(m);
    }

    void delete_current() {
        if (m == l->sentinel)
        {
			m=m->next;//needed in case iterator was created before anything was added to the list
			if (m == l->sentinel)
			{
			    fail(lit_fail_delete_current,LIST_TEM_TYPEID_THIS);
			}
        }
        m = m->next;
        l->delete_entry_by_pointer(m->prev);
    }


    void change_current(T const & new_val) 
    {
    	if (m == l->sentinel)
       		m=m->next;//needed in case iterator was created before anything was added to the list
      
      	if (m != l->sentinel) 
      	{
        	// Delete the current member out of the list, put a new one in.
			List_Member<T> * n = new List_Member<T>(new_val);
        	m->prev->next = n;
        	m->next->prev = n;
        	n->next = m->next;
        	n->prev = m->prev;
        	delete m;
        	m = n; // or we're pointing at the thing we just deleted.
        	l->cleanup(); // because it's changed, but the List doesn't know that.
      	}
      	else
        	fail(lit_fail_change_current,LIST_TEM_TYPEID_THIS);
    }

	// Go to the start of the list.
    void restart() { m = l->sentinel->next; }       
	
	// Go to the end of the list.
	void end() { m = l->sentinel->prev; }

	// Is it on the last entry.
    bool at_last() const { if (m == l->sentinel->prev) return true; else return false; }

	// Get the next entry but done move the pointer.
    T const & get_next()  {
        if (m->next == l->sentinel)
		{
			if (m->next->next == l->sentinel)
			{
				fail(lit_fail_operator,LIST_TEM_TYPEID_THIS);
			}
	        return l->data(m->next->next);
		}
        return l->data(m->next);
    }
};

#define LIL List_Iterator_Loop

#ifdef NDEBUG
	#undef fail // allow other code to have local variables called or differently scoped 'fail'
#endif

#endif
