/********************************************************/
/* Hash Table template class - v1.2						*/
/*														*/
/* Author: Jake Hotson									*/
/*														*/
/* construct a hash table for objects of type TYPE		*/
/*														*/
/* HashTable<TYPE>(unsigned initialTableSizeShift		*/
/*                     = HT_DEFAULTTABLESIZESHIFT);		*/
/*														*/
/* TYPE must have boolean operator == defined, and       */
/* there must be an overload of the function             */
/* HashFunction() which acts as a hash                   */
/* function for the TYPE, taking an argument of type     */
/* TYPE or a suitable conversion thereof, and returning  */
/* unsigned                                              */
/* Also, if a==b then HashFunction(a)==HashFunction(b)   */
/* Additionally, TYPE must have valid assignment         */
/* operator and copy constructor                         */
/****************************************************Jake*/

/************************************************************/
/*                                                          */
/* v1.0 public classes and functions and macros             */
/*                                                          */
/* HASH_TEMPLATE_VERSION                                    */
/*                                                          */
/* 	major version * 100 + minor version                     */
/* 	in this version (1.0) the value is 100                  */
/* 	                                                        */
/* HT_NODEFAULTFNS                                          */
/*                                                          */
/* 	#define to not compile default hash functions           */
/* 	                                                        */
/* HT_DEFAULTABLESIZESHIFT                                  */
/*                                                          */
/* 	#define to override default table size shift            */
/* 	of 6 (which gives a default table size of 64)           */
/* 	                                                        */
/* HT_FAIL(sP)                                              */
/*                                                          */
/* 	this macro should expand to a function (which           */
/* 	takes a char const * parameter) to be called            */
/* 	in the event of an error                                */
/* 	                                                        */
/* HashFunction(unsigned)                                   */
/* 	                                                        */
/* 	default hash function for integers                      */
/* 	                                                        */
/* HashFunction(void const *)                               */
/* 	                                                        */
/* 	default hash function for pointers                      */
/* 	                                                        */
/* HashFunction(char const *)                               */
/* 	                                                        */
/* 	default hash function for strings                       */
/* 	                                                        */
/* class HashTable<TYPE>                                    */
/*                                                          */
/* 	hash table template class                               */
/* 	TYPE should have == operator defined and an             */
/* 	overloaded HashFunction() defined which can             */
/* 	take an argument type TYPE or suitable                  */
/* 	conversion thereof                                      */
/*                                                          */
/* HashTable::HashTable()                                   */
/*                                                          */
/* 	constructs an empty table with default table            */
/* 	size                                                    */
/*                                                          */
/* HashTable::HashTable(unsigned)                           */
/*                                                          */
/* 	constructs an empty table with specific                 */
/* 	table size determined by 2^arg                          */
/* 	                                                        */
/* bool HashTable::AddChecked(TYPE)                         */
/*                                                          */
/* 	adds object to hash table unless it already             */
/* 	contains it. Returns non-zero if object was             */
/* 	added, and zero if it was already contained             */
/* 	                                                        */
/* void HashTable::AddRegardless(TYPE)                      */
/*                                                          */
/* 	adds object to hash table but does not check            */
/* 	if it already exists, so duplicates could               */
/* 	occur                                                   */
/* 	                                                        */
/* void HashTable::AddAsserted(TYPE)                        */
/*                                                          */
/* 	adds object to hash table but throws a failure if it    */
/*  already exists unless NDEBUG is defined in which case   */
/*  this function is identical to AddRegardless             */
/* 	                                                        */
/* TYPE const * HashTable::Contains(TYPE) const             */
/*                                                          */
/* 	returns pointer to equivalent entry in table,           */
/* 	or NULL, if none exists                                 */
/* 	                                                        */
/* bool HashTable::Remove(TYPE)                             */
/*                                                          */
/* 	removes object from table if it exists.                 */
/* 	Returns non-zero if object was removed, or              */
/* 	zero if object was not contained                        */
/* 	                                                        */
/* void HashTable::RemoveAll()                              */
/*                                                          */
/* 	empties the table                                       */
/* 	                                                        */
/* void HashTable::RemoveAsserted(TYPE)                     */
/*                                                          */
/* 	removes object from table, but throws a                 */
/* 	failure if object was not contained                     */
/* 	                                                        */
/* unsigned HashTable::Size() const                         */
/*                                                          */
/* 	returns number of objects in table                      */
/* 	                                                        */
/* class HashTable<TYPE>::ConstIterator                     */
/*                                                          */
/* 	class for iterating through objects contained           */
/* 	in hash table, without modifying the contents           */
/* 	of the table                                            */
/* 	                                                        */
/* HashTable::ConstIterator::ConstIterator(HashTable const) */
/*                                                          */
/* 	constructs an iterator linked to a table,               */
/* 	ready for a Get()                                       */
/* 	                                                        */
/* TYPE const & HashTable::ConstIterator::Get() const       */
/* HashTable::ConstIterator::operator TYPE const & () const */
/*                                                          */
/* 	gets a constant reference to the object in              */
/* 	the table pointed to by the iterator object             */
/*                                                          */
/* bool HashTable::ConstIterator::Done() const              */
/*                                                          */
/* 	returns non-zero only if there is no more               */
/* 	iterating to do                                         */
/* 	                                                        */
/* void HashTable::ConstIterator::Next()                    */
/*                                                          */
/* 	moves iterator to a fresh entry in the table            */
/* 	                                                        */
/* class HashTable<TYPE>::Iterator                          */
/*                                                          */
/* 	class for iterating through objects contained           */
/* 	in hash table, allowing their removal                   */
/* 	                                                        */
/* HashTable::Iterator::Iterator(HashTable)                 */
/*                                                          */
/* 	constructs an iterator linked to a table,               */
/* 	ready for a Get()                                       */
/* 	                                                        */
/* HashTable::Iterator::Remove()                            */
/*                                                          */
/* 	removes the current object pointed to by the            */
/* 	iterator from the table, and advances the               */
/* 	iterator to the next entry                              */
/* 	                                                        */
/*******************************************************Jake*/

/************************************************************/
/*                                                          */
/* v1.1 - v1.11 extended functionality                      */
/*                                                          */
/* class HashTable<TYPE>::Node                              */
/* 	                                                        */
/*  this is the internal class for nodes in the table       */
/* 	                                                        */
/* TYPE HashTable::Node::d                                  */
/* 	                                                        */
/*  this is the value of a node's data                      */
/* 	                                                        */
/* Node * HashTable::NewNode()                              */
/* 	                                                        */
/*  allocates a node without inserting it into the table;   */
/*  you are expected to set Node::d to the required value   */
/*  to be added to the table, and call one of these methods */
/*  to link the node into the table                         */
/* 	                                                        */
/* void HashTable::AddAsserted(Node *)                      */
/* 	                                                        */
/*  like AddAsserted(TYPE const &) but takes a Node *       */
/*  created with NewNode(); do not use the Node * pointer   */
/*  after calling this method                               */
/* 	                                                        */
/* void HashTable::AddRegardless(Node *)                    */
/* 	                                                        */
/*  like AddRegardless(TYPE const &) but takes a Node *     */
/*  created with NewNode(); do not use the Node * pointer   */
/*  after calling this method                               */
/*                                                          */
/* bool HashTable::AddChecked(Node *)                       */
/* 	                                                        */
/*  like AddChecked(TYPE const &) but takes a Node *        */
/*  created with NewNode(); do not use the Node * pointer   */
/*  after calling this method if it returns true, but if it */
/*  returns false (i.e. the Node was not added, then you    */
/*  can re-use the Node or use the following function to    */
/*  delete it.                                              */
/* 	                                                        */
/* void HashTable::DeleteNode(Node *)                       */
/* 	                                                        */
/*  destroys a Node created with NewNode. Use this only if  */
/*  the node created was not added to the hash table        */
/* 	                                                        */
/*******************************************************Jake*/

/************************************************************/
/*															*/
/* v1.2														*/
/*                                                          */
/* HashTable::Iterator::Restart()							*/
/*  restarts the iterator									*/
/*															*/
/* HashTable::HashTable(HashTable<TYPE>)					*/
/*  copy constructor										*/
/*															*/
/* HashTable::ValueIterator(unsigned)						*/
/*  iterate through all entries of a specific hashvalue		*/
/*															*/
/*******************************************************Alex*/

#ifndef HASH_TEMPLATE_VERSION
#define HASH_TEMPLATE_VERSION 12 // v1.2

#include <stddef.h>
#include <ctype.h>// for toupper

// v1,0 Default Hash Functions defined:
// HashFunction(unsigned), HashFunction(void const *), HashFunction(char const *)
// you can disable the default hash functions by defining HT_NODEFAULTFNS

#ifndef HT_NODEFAULTFNS
	// a hash function for integral (unsigned) values
	inline unsigned HashFunction(unsigned const _i)
	{
		return _i ^ _i>>4 ^ _i>>9 ^ _i>>15 ^ _i>>22;
	}

	// a hash function for pointers
	inline unsigned HashFunction(void const * const _vP)
	{
		// treat as integer
		return HashFunction(reinterpret_cast<uintptr_t>(_vP));
	}

	// a hash function for strings
	inline unsigned HashFunction(char const * _sP)
	{
		unsigned rv = 0;
		while (*_sP) rv += toupper(*_sP++);
		return rv;
	}
#endif

// v1,0 Default (initial) table size (log2 of)
// Define this to another value if you like, 
// or just override in the constructor.
// in v1.0, the table is not self-expanding,
// but if this feature is implememted, then 
// this value will become the log2(initial table size),
// and if table becomes is self-contracting,
// this value will also give the minimum table size

#ifndef HT_DEFAULTTABLESIZESHIFT
	#define HT_DEFAULTTABLESIZESHIFT 6
#endif

// for asserted functions, define HT_FAIL to be your function
// to be triggered upon a failure, eg.
// #define HT_FAIL(strP) fprintf(stderr,"%s\n",strP)
#ifndef HT_FAIL
	#define HT_FAIL(strP) ((void)0)
#endif

template <class TYPE, class ARG_TYPE, class CMP_ARG_TYPE>
class _base_HashTable
{
	public:
		class Iterator;
		class ConstIterator;
		class ValueIterator;
		class Node;
		
	public:
		// V1.0 Functionality
		
		// empty constructor, with argument specifying initial table size (log2 of)
		_base_HashTable(unsigned = HT_DEFAULTTABLESIZESHIFT);
		
		// destructor
		virtual ~_base_HashTable();
		
		// copy constructor and assignment not provided in v1.0
		_base_HashTable(_base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE> const &);
		_base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE> & operator = (_base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE> const &);
		
		// add, checking that an equivalent entry does not already exist
		// returns non-zero if entry was added
		bool AddChecked(ARG_TYPE);
		
		// add, regardless of whether an equivalent entry already exists
		void AddRegardless(ARG_TYPE);
		
		// add, checking that an equivalent entry does not already exist
		// triggering fail function if one does
		void AddAsserted(ARG_TYPE);
		
		// see if entry exists, get pointer to it if it does
		TYPE const * Contains(CMP_ARG_TYPE) const;
		
		// remove an entry (once only in the case of equivalent entries listed multiple times)
		// returns non-zero if entry existed and was removed
		bool Remove(CMP_ARG_TYPE);
		
		// remove an entry (once only in the case of equivalent entries listed multiple times)
		// triggers fail function if no entry existed to remove
		void RemoveAsserted(CMP_ARG_TYPE);
		
		// empty the table
		void RemoveAll();
		
		// return num entries in table
		unsigned Size() const;

		// a _base_HashTable const iterator
		class ConstIterator
		{
				// Nested class functions apparently have to be declared here for MSVC compatability
			public:
				// construct from const hash table
				ConstIterator(_base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE> const & tableR)
					: chainPP(tableR.chainPA)
					, nChainsRemaining(tableR.tableSize)
					, nEntriesRemaining(tableR.nEntries)
				{
					if (nEntriesRemaining)
					{
						while (!*chainPP)
						{
							++ chainPP;
							-- nChainsRemaining;
						}
						nodePP = chainPP;
					}
				}

				// returns non-zero if there are no more entries to get
				inline bool Done() const
				{
					return ! nEntriesRemaining;
				}

				inline void Restart(_base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE> const & tableR)
				{
					chainPP = tableR.chainPA;
					nChainsRemaining = tableR.tableSize;
					nEntriesRemaining = tableR.nEntries;

					if (nEntriesRemaining)
					{
						while (!*chainPP)
						{
							++ chainPP;
							-- nChainsRemaining;
						}
						nodePP = chainPP;
					}
				}

				// get the current entry pointed to, either with Get() or cast operator
				inline operator ARG_TYPE () const
				{
					return Get();
				}
				inline ARG_TYPE Get() const
				{
					if( Done() )
					{
						HT_FAIL("HTT: Tried to Get() from an iterator which was Done()");
					}
					return (*nodePP)->d;
				}

				// advance to the next entry
				void Next()
				{
					if (!nEntriesRemaining)
					{
						HT_FAIL("HTT: Tried to do Next() on an iterator which was Done()");
					}
					if ((*nodePP)->nextP)
					{
						nodePP = &(*nodePP)->nextP;
					}
					else
					{
						do
						{
							++ chainPP;
							-- nChainsRemaining;
						}
						while (nChainsRemaining && !*chainPP);
						nodePP = chainPP;
					}
					-- nEntriesRemaining;
				}
				
			private:
			
				Node * * chainPP;
				Node * * nodePP;
				unsigned nChainsRemaining;
				unsigned nEntriesRemaining;
				
				friend class Iterator;
		};

		// a _base_HashTable non-const iterator - can remove entry pointed to
		class Iterator : public ConstIterator
		{
				// Nested class functions apparently have to be declared here for MSVC compatability
			public:
				// construct from hash table
				inline Iterator(_base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE> & tableR)
					: ConstIterator(tableR)
					, tableNEntriesP(&tableR.nEntries)
				{
				}
				
				// remove the current entry pointed to, advancing to the next
				void Remove()
				{
					if (!ConstIterator::nEntriesRemaining)
					{
						HT_FAIL("HTT: Tried to Remove() via an iterator which was Done()");
					}
					Node * oldP = *ConstIterator::nodePP;
					*ConstIterator::nodePP = oldP->nextP;
					delete oldP;
					if (!*ConstIterator::nodePP)
					{
						do
						{
							++ ConstIterator::chainPP;
							-- ConstIterator::nChainsRemaining;
						}
						while (ConstIterator::nChainsRemaining && !*ConstIterator::chainPP);
						ConstIterator::nodePP = ConstIterator::chainPP;
					}
					-- ConstIterator::nEntriesRemaining;
					-- *tableNEntriesP;
				}

			private:
				unsigned * tableNEntriesP;
		};

		// v1.2 extended functionality
		// a _base_HashTable iterator through a specific hash value
		class ValueIterator
		{
				// Nested class functions apparently have to be declared here for MSVC compatability
			public:
				// construct from const hash table
				ValueIterator(_base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE> & tableR, unsigned value)
					: chainPP(tableR.chainPA)
					, value(value)
					, tableNEntriesP(&tableR.nEntries)
				{
					chainPP += (value & tableR.tableSizeMask);
					nodePP = chainPP;
				}

				// returns non-zero if there are no more entries to get
				inline bool Done() const
				{
					return( !(*nodePP) );
				}

				inline operator ARG_TYPE () const
				{
					return Get();
				}
				inline ARG_TYPE Get() const
				{
					if( Done() )
					{
						HT_FAIL("HTT: Tried to Get() from an iterator which was Done()");
					}
					return (*nodePP)->d;
				}

				inline void Restart(_base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE> const & tableR)
				{
					chainPP = tableR.chainPA + (value & tableR.tableSizeMask);
					nodePP = chainPP;
				}

				// advance to the next entry
				void Next()
				{
					if( *nodePP )
					{
						nodePP = &(*nodePP)->nextP;	// even if it's NULL for ValueIterator
					}
					else
					{
						HT_FAIL("HTT: Tried to do Next() on a Value iterator which was Done()");
					}
				}

				// remove the current entry pointed to, advancing to the next
				void Remove()
				{
					if( *nodePP )
					{
						Node * oldP = *nodePP;
						*nodePP = oldP->nextP;
						delete oldP;
						-- *tableNEntriesP;
					}
					else
					{
						HT_FAIL("HTT: Tried to Remove() via an iterator which was Done()");
					}
				}

			private:

				Node * * chainPP;
				Node * * nodePP;

				unsigned value;
				unsigned * tableNEntriesP;
		};

		// V1.1 extended functionality
		// allow user to create nodes, change
		// the data using this pointer,
		// then insert the node into the
		// correct chain, without having
		// to create a copy of the data to
		// be added on the stack
		virtual Node * NewNode();
		// add, checking that an equivalent entry does not already exist
		// triggering fail function if one does
		void AddAsserted(Node *);
		// add, regardless of whether an equivalent entry already exists
		void AddRegardless(Node *);
		// V1.11 allows AddChecked for the Node-adding interface
		bool AddChecked(Node *);
		// if add checked fails, you should avoid the memory leak with this function
		virtual void DeleteNode(Node *);

		class Node
		{
			public:
				TYPE d;
			private:
				Node * nextP;
				// Nested class functions apparently have to be declared here for MSVC compatability
				inline Node(ARG_TYPE _dataR,Node * _nextP)
					: d(_dataR)
					, nextP(_nextP)
				{
				}
				inline Node()
				{
				}
				inline ~Node()
				{
				}
				void DeleteChain()
				{
					if (nextP) nextP->DeleteChain();
					delete this;
				}

				friend class ConstIterator;
				friend class Iterator;
				friend class ValueIterator;
				friend class _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>;
		};
		
	private:
		// virtual functions for future expansion
		virtual Node * NewNode(ARG_TYPE,Node *);
		
		unsigned nEntries;
		unsigned tableSize;
		unsigned tableSizeMask;
		Node * * chainPA;

		friend class ConstIterator;
		friend class Iterator;
		friend class ValueIterator;
		
		inline void Xx(){}
};

/*******************/
/* Defined to Fail */
/**************Jake*/

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
inline _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE> & _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::operator = (_base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE> const &)
{
  	HT_FAIL("HTT: assignment operator not allowed in this version");
  	return *this;
}

/*******************************/
/* Inline Function Definitions */
/**************************Jake*/

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
inline void _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::AddRegardless(ARG_TYPE _dataR)
{
	Node * & chainPR = chainPA[HashFunction(_dataR) & tableSizeMask];
	chainPR = new Node(_dataR,chainPR);
	++ nEntries;
}

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
inline void _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::AddRegardless(Node * _nodeP)
{
	Node * & chainPR = chainPA[HashFunction(_nodeP->d) & tableSizeMask];
	_nodeP->nextP = chainPR;
	chainPR = _nodeP;
	++ nEntries;
}

// with NDEBUG on these functions evaluate to be identical to AddRegardless
#ifdef NDEBUG
template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
inline void _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::AddAsserted(ARG_TYPE _dataR)
{
	AddRegardless(_dataR);
}

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
inline void _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::AddAsserted(Node * _nodeP)
{
	AddRegardless(_nodeP);
}
#endif

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
inline TYPE const * _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::Contains(CMP_ARG_TYPE _dataR) const
{
	for (Node const * nodeP = chainPA[HashFunction(_dataR) & tableSizeMask]; nodeP; nodeP = nodeP->nextP)
	{
		if (nodeP->d == _dataR) return &nodeP->d;
	}
	return NULL;
}

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
inline unsigned _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::Size() const
{
	return nEntries;
}

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
inline void _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::RemoveAll()
{
	for (unsigned i=0; i<tableSize; ++i)
		if (chainPA[i])
		{
			chainPA[i]->DeleteChain();
			chainPA[i] = NULL;
		}
	nEntries = 0;
}

/*************************************************************/
/* Non inlines declared here since neither Watcom nor        */
/* MS Visual C will link if they're in their own source file */
/********************************************************Jake*/

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
_base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::_base_HashTable(unsigned _initialTableSizeShift)
	: nEntries(0)
	, tableSize(1<<_initialTableSizeShift)
	, tableSizeMask(tableSize-1)
	, chainPA(new Node * [tableSize])
{
	for (unsigned i=0; i<tableSize; ++i)
		chainPA[i] = NULL;
}

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
inline _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::_base_HashTable(_base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE> const & ht)
	: nEntries(0)
	, tableSize(ht.tableSize)
	, tableSizeMask(tableSize-1)
	, chainPA(new Node * [tableSize])
{
	for (unsigned i=0; i<tableSize; ++i) { chainPA[i] = NULL; }

	
//	for(HashTable<TYPE>::ConstIterator it(ht); !it.Done(); it.Next() )
	for (typename _base_HashTable::ConstIterator it(ht); !it.Done(); it.Next() )
	{
		AddRegardless( it.Get() );
	}
}

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
_base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::~_base_HashTable()
{
	for (unsigned i=0; i<tableSize; ++i)
		if (chainPA[i])
			chainPA[i]->DeleteChain();
	delete[] chainPA;
}

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
bool _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::AddChecked(ARG_TYPE _dataR)
{
	Node * & chainPR = chainPA[HashFunction(_dataR) & tableSizeMask];
	for (Node const * nodeP = chainPR; nodeP; nodeP = nodeP->nextP)
	{
		if (nodeP->d == _dataR) return false;
	}
	chainPR = new Node(_dataR,chainPR);
	++ nEntries;
	return true;
}

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
bool _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::AddChecked(Node * _nodeP)
{
	Node * & chainPR = chainPA[HashFunction(_nodeP->d) & tableSizeMask];
	for (Node const * nodeP = chainPR; nodeP; nodeP = nodeP->nextP)
	{
		if (nodeP->d == _nodeP->d) return false;
	}
	_nodeP->nextP = chainPR;
	chainPR = _nodeP;
	++ nEntries;
	return true;
}

// with NDEBUG on these functions evaluate to be identical to AddRegardless
#ifndef NDEBUG
template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
void _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::AddAsserted(ARG_TYPE _dataR)
{
	Node * & chainPR = chainPA[HashFunction(_dataR) & tableSizeMask];
	for (Node const * nodeP = chainPR; nodeP; nodeP = nodeP->nextP)
	{
		if (nodeP->d == _dataR)
		{
			HT_FAIL("HTT: Tried to add entry which was already contained in table");
		}
	}
	chainPR = new Node(_dataR,chainPR);
	++ nEntries;
}

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
void _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::AddAsserted(Node * _nodeP)
{
	Node * & chainPR = chainPA[HashFunction(_nodeP->d) & tableSizeMask];
	for (Node const * nodeP = chainPR; nodeP; nodeP = nodeP->nextP)
	{
		if (nodeP->d == _nodeP->d)
		{
			HT_FAIL("HTT: Tried to add entry which was already contained in table");
		}
	}
	_nodeP->nextP = chainPR;
	chainPR = _nodeP;
	++ nEntries;
}
#endif

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
bool _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::Remove(CMP_ARG_TYPE _dataR)
{
	for (Node * * nodePP = &chainPA[HashFunction(_dataR) & tableSizeMask]; (*nodePP); nodePP = &(*nodePP)->nextP)
	{
		if ((*nodePP)->d == _dataR)
		{
			Node * oldP = *nodePP;
			*nodePP = oldP->nextP;
			delete oldP;
			-- nEntries;
			return true;
		}
	}
	return false;
}

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
void _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::RemoveAsserted(CMP_ARG_TYPE _dataR)
{
	for (Node * * nodePP = &chainPA[HashFunction(_dataR) & tableSizeMask]; (*nodePP); nodePP = &(*nodePP)->nextP)
	{
		if ((*nodePP)->d == _dataR)
		{
			Node * oldP = *nodePP;
			*nodePP = oldP->nextP;
			delete oldP;
			-- nEntries;
			return;
		}
	}
	HT_FAIL("HTT: Tried to remove entry which was not contained in table");
}

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
typename _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::Node * _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::NewNode(ARG_TYPE _dataR,Node * _nextP)
{
	return new Node(_dataR,_nextP);
}

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
typename _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::Node * _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::NewNode()
{
	return new Node;
}

template <class TYPE,class ARG_TYPE,class CMP_ARG_TYPE>
void _base_HashTable<TYPE,ARG_TYPE,CMP_ARG_TYPE>::DeleteNode(Node * _nodeP)
{
	delete _nodeP;
}

template <class TYPE> class HashTable;

#define HT_DEFINITION(T1,T2,T3) \
	: public _base_HashTable<T1,T2,T3> { public: HashTable(unsigned _initialTableSizeShift = HT_DEFAULTTABLESIZESHIFT) : _base_HashTable<T1,T2,T3>(_initialTableSizeShift){} };

// for simple types
#define HT_WATCOM_DEFINE_FOR_SIMPLE_TYPE(TYPE) \
	class HashTable<TYPE> HT_DEFINITION(TYPE,TYPE,TYPE)
	
#define HT_DEFINE_FOR_SIMPLE_TYPE(SIMPLE_TYPE) template<> HT_WATCOM_DEFINE_FOR_SIMPLE_TYPE(SIMPLE_TYPE)

HT_DEFINE_FOR_SIMPLE_TYPE(unsigned long)
HT_DEFINE_FOR_SIMPLE_TYPE(signed long)
HT_DEFINE_FOR_SIMPLE_TYPE(unsigned)
HT_DEFINE_FOR_SIMPLE_TYPE(signed)
HT_DEFINE_FOR_SIMPLE_TYPE(unsigned short)
HT_DEFINE_FOR_SIMPLE_TYPE(signed short)
HT_DEFINE_FOR_SIMPLE_TYPE(unsigned char)
HT_DEFINE_FOR_SIMPLE_TYPE(signed char)
HT_DEFINE_FOR_SIMPLE_TYPE(char)
HT_DEFINE_FOR_SIMPLE_TYPE(float)

#undef HT_DEFINE_FOR_SIMPLE_TYPE
#undef HT_WATCOM_DEFINE_FOR_SIMPLE_TYPE

// for pointer types
#if 0 // doesnt't compile!!
template <class TYPE>
class HashTable<TYPE *> HT_DEFINITION(TYPE *, TYPE *, TYPE const *)

template <class TYPE>
class HashTable<TYPE const *> HT_DEFINITION(TYPE const *, TYPE const *, TYPE const *)
#endif

// for other types
template <class TYPE>
class HashTable HT_DEFINITION(TYPE,TYPE const &, TYPE const &)

//template <class TYPE *> class HashTable : public _base_HashTable<TYPE *,TYPE *,TYPE const *> {};

#undef HT_DEFINITION

#endif // ! HASH_TEMPLATE_VERSION
