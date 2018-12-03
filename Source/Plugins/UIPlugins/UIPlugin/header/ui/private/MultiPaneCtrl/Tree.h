//==========================================================
// Author: Borodenko Oleg
// 05/12/2009 <oktamail@gmail.com>
//==========================================================
// 
#pragma once
/////////////////////////////////////////////////////////////////////////////
#pragma warning(push,3)
	#pragma warning(disable : 4702)	// unreachable code.
	#pragma warning(disable : 4786)	// too long identifiers.
	#pragma warning(disable : 4018)	// signed/unsigned mismatch.
#pragma warning(pop)
/////////////////////////////////////////////////////////////////////////////
// 
#pragma warning(push)
#pragma warning(disable : 4290)	// C++ exception specification ignored...
#pragma warning(disable : 4355)	// 'this' : used in base member initializer list
// 
typedef struct _HNODE {} *HNODE;
// 
template<typename T>
class tree
{public:
	tree() : m_NodeRoot(this)
	{	m_NodeRoot.parent = NULL;
	}
	tree(tree<T> const &src) : m_NodeRoot(this)
	{	m_NodeRoot.parent = NULL;
		*this = src;
	}
	~tree()
	{	DeleteAll();
	}

protected:
	struct Node;
	friend Node;
		// must be before 'struct Node' definition.
	Blade::Set<Node *> allnodes;		// all nodes except root node.
		// 
	struct Node : public _HNODE , public Blade::Allocatable
	{	Node(tree<T> *c)
		{	container = c;
		}
		~Node()
		{	for(Blade::List<Node *>::iterator i; nodes.empty()==false; )	// from bottom to top.
			{	i = --nodes.end();
				BLADE_DELETE *i;								// 1 !!!.
				container->allnodes.erase(*i);	// 2 !!!.
				nodes.erase(i);						// 3 !!!.
			}
		}
		tree<T> *container;
		Node *parent;
		T data;
		typename Blade::List<Node *>::iterator iterator;	// iterator of this node in the parent list.
		Blade::List<Node *> nodes;	// child nodes.
	} m_NodeRoot;
		// 
	typedef typename Blade::List<Node *>::iterator i_nodes;
	typedef typename Blade::List<Node *>::const_iterator ic_nodes;
	typedef typename Blade::List<Node *>::reverse_iterator ir_nodes;

public:
	void operator=(tree<T> const &src) throw(std::bad_alloc)
	{	Copy(&src);
	}
	void Copy(tree<T> const *src) throw(std::bad_alloc)
	{	DeleteAll();
		try
		{	Copy(GetRoot(),src,src->GetRoot());
		}
		catch(std::bad_alloc &)
		{	DeleteAll();
			throw;
		}
	}
private:
	void Copy(HNODE hDst, tree<T> const *treeSrc, HNODE hSrc) throw(std::bad_alloc)
	{	*GetData(hDst) = *treeSrc->GetData(hSrc);
			// 
		for(HNODE s=treeSrc->GetFirstChild(hSrc); s!=NULL; s=treeSrc->GetNextChild(s))
		{	HNODE d = Add(hDst);
			Copy(d,treeSrc,s);
		}
	}

public:
	HNODE Add(HNODE parent) throw(std::bad_alloc)
	{	_ASSERTE(parent==NULL || IsExist(parent)==true);
			// 
		Node *p = (parent!=NULL ? HNDLtoNODE(parent) : &m_NodeRoot);
			// 
		Node *n = BLADE_NEW Node(this);
		try
		{	allnodes.insert(n);
			p->nodes.push_back(n);
		}
		catch(std::bad_alloc &)
		{	if(allnodes.find(n)!=allnodes.end()) allnodes.erase(n);
			BLADE_DELETE n;
			throw;
		}
		n->parent = p;
		n->iterator = --p->nodes.end();
		return n;
	}
	HNODE Insert(HNODE before) throw(std::bad_alloc)
	{	_ASSERTE(IsExist(before)==true);
		_ASSERTE(before!=GetRoot());
			// 
		Node *b = HNDLtoNODE(before);
		Node *p = b->parent;
			// 
		Node *n = BLADE_NEW Node(this);
		try
		{	allnodes.insert(n);
			n->iterator = p->nodes.insert(b->iterator,n);
		}
		catch(std::bad_alloc &)
		{	if(allnodes.find(n)!=allnodes.end()) allnodes.erase(n);
			BLADE_DELETE n;
			throw;
		}
		n->parent = p;
		return n;
	}
	void Remove(HNODE before, HNODE src)
	{	_ASSERTE(before!=src);
		_ASSERTE(IsExist(before)==true && IsExist(src)==true);
		_ASSERTE(GetParent(before)==GetParent(src));
			// 
		Node *p = HNDLtoNODE(before)->parent;
		Blade::List<Node *> tmp;
			// 
		tmp.splice(tmp.end(),p->nodes,GetIterator(src));
			// 
		i_nodes i_before = GetIterator(before);
		p->nodes.splice(i_before,tmp);
			// 
		HNDLtoNODE(src)->iterator = --i_before;
	}
	void Delete(HNODE node)
	{	_ASSERTE(IsExist(node)==true);
		_ASSERTE(node!=GetRoot());
			// 
		Node *n = HNDLtoNODE(node);
		Node *p = n->parent;
			// 
		i_nodes i = n->iterator;
		BLADE_DELETE n;				// 1 !!!.
		allnodes.erase(n);	// 2 !!!.
		p->nodes.erase(i);	// 3 !!!.
	}
	void DeleteAll()
	{	for(i_nodes i; m_NodeRoot.nodes.empty()==false; )
		{	i = --m_NodeRoot.nodes.end();
			BLADE_DELETE *i;						// 1 !!!.
			allnodes.erase(*i);			// 2 !!!.
			m_NodeRoot.nodes.erase(i);	// 3 !!!.
		}
	}
		// 
		//////////////// 
	HNODE InsertIntermediate(HNODE parent) throw(std::bad_alloc)
	{	_ASSERTE(parent==NULL || IsExist(parent)==true);
		_ASSERTE(GetCount(parent)>0);
			// 
		Node *p = (parent!=NULL ? HNDLtoNODE(parent) : &m_NodeRoot);
			// 
		Node *n = BLADE_NEW Node(this);
		try
		{	allnodes.insert(n);
			p->nodes.push_back(n);
		}
		catch(std::bad_alloc &)
		{	if(allnodes.find(n)!=allnodes.end()) allnodes.erase(n);
			BLADE_DELETE n;
			throw;
		}
			// 
		Blade::List<Node *> tmp;
		tmp.splice(tmp.end(),p->nodes,p->nodes.begin(),--p->nodes.end());
		n->nodes.splice(n->nodes.end(),tmp);
			// 
		n->parent = p;
		n->iterator = --p->nodes.end();
		for(i_nodes i=n->nodes.begin(), e=n->nodes.end(); i!=e; ++i)
		{	(*i)->parent = n;
			(*i)->iterator = i;
		}
		return n;
	}
	void DeleteIntermediate(HNODE node)
	{	_ASSERTE(IsExist(node)==true);
		_ASSERTE(GetCount(node)>0);
		_ASSERTE(GetParent(node)!=NULL);
		_ASSERTE(GetCount(GetParent(node))==1);
			// 
		Node *n = HNDLtoNODE(node);
		Node *p = n->parent;
			// 
		p->nodes.swap( n->nodes );
		for(i_nodes i=p->nodes.begin(), e=p->nodes.end(); i!=e; ++i)
		{	(*i)->parent = p;
			(*i)->iterator = i;
		}
		n->nodes.clear();		// 1 !!! - exclude recursion in the ~Node.
		BLADE_DELETE n;				// 2 !!!.
		allnodes.erase(n);	// 3 !!!.
	}
		// 
		//////////////// 
	HNODE GetRoot() const
	{	return (Node *)&m_NodeRoot;
	}
	HNODE GetParent(HNODE node) const
	{	_ASSERTE(IsExist(node)==true);
			// 
		return HNDLtoNODE(node)->parent;
	}
	HNODE GetNode(HNODE parent, int idx) const
	{	_ASSERTE(parent==NULL || IsExist(parent)==true);
		_ASSERTE(idx>=0 && idx<GetCount(parent));
			// 
		Node *p = (parent!=NULL ? HNDLtoNODE(parent) : (Node *)&m_NodeRoot);
			// 
		ic_nodes i = p->nodes.begin();
		for(; idx>0; --idx) ++i;
		return *i;
	}
	T *GetData(HNODE node) const
	{	_ASSERTE(node==NULL || IsExist(node)==true);
			// 
		Node *p = (node!=NULL ? HNDLtoNODE(node) : (Node *)&m_NodeRoot);
		return &p->data;
	}
	int GetIndex(HNODE node) const
	{	_ASSERTE(IsExist(node)==true);
		_ASSERTE(node!=GetRoot());
			// 
		Node *n = HNDLtoNODE(node);
		Node *p = n->parent;
			// 
		int idx=0;
		for(ic_nodes i=p->nodes.begin(), e=p->nodes.end(); i!=e; ++i,++idx)
			if(*i==n) return idx;
		return -1;
	}
	int GetCount(HNODE parent) const
	{	_ASSERTE(parent==NULL || IsExist(parent)==true);
			// 
		Node *p = (parent!=NULL ? HNDLtoNODE(parent) : (Node *)&m_NodeRoot);
		return (int)p->nodes.size();
	}
	int GetCount() const
	{	return (int)allnodes.size() + 1/*root*/;
	}
	bool IsExist(HNODE node) const
	{	_ASSERTE(node!=NULL);
			// 
		if(node==GetRoot()) return true;
		return (allnodes.find( HNDLtoNODE(node) )!=allnodes.end());
	}
		// 
		////////////////
	HNODE GetFirstChild(HNODE parent) const
	{	_ASSERTE(parent==NULL || IsExist(parent)==true);
			// 
		Node *p = (parent!=NULL ? HNDLtoNODE(parent) : (Node *)&m_NodeRoot);
		if(p->nodes.empty()==true) return NULL;
		return p->nodes.front();
	}
	HNODE GetPrevChild(HNODE node) const
	{	_ASSERTE(IsExist(node)==true);
		_ASSERTE(node!=GetRoot());
			// 
		Node *n = HNDLtoNODE(node);
		Node *p = n->parent;
			// 
		Blade::List<Node *>::iterator i = n->iterator;
		if(i==p->nodes.begin()) return NULL;
		return *--i;
	}
	HNODE GetNextChild(HNODE node) const
	{	_ASSERTE(IsExist(node)==true);
		_ASSERTE(node!=GetRoot());
			// 
		Node *n = HNDLtoNODE(node);
		Node *p = n->parent;
			// 
		Blade::List<Node *>::iterator i = n->iterator;
		if(++i==p->nodes.end()) return NULL;
		return *i;
	}
	HNODE GetLastChild(HNODE parent) const
	{	_ASSERTE(parent==NULL || IsExist(parent)==true);
			// 
		Node *p = (parent!=NULL ? HNDLtoNODE(parent) : (Node *)&m_NodeRoot);
		if(p->nodes.empty()==true) return NULL;
		return *--p->nodes.end();
	}
		// 
		//////////////// 
	HNODE GetFirst() const
	{	return GetFirst((Node *)&m_NodeRoot);
	}
	HNODE GetFirst(HNODE parent) const
	{	_ASSERTE(IsExist(parent)==true);
			// 
		return parent;
	}
		// 
	HNODE GetPrev(HNODE node) const
	{	return GetPrev((Node *)&m_NodeRoot,node);
	}
	HNODE GetPrev(HNODE parent, HNODE node) const
	{	_ASSERTE(IsExist(parent)==true);
		_ASSERTE(IsExist(node)==true);
			// 
		if(node==parent) return NULL;
			// 
		Node *n = HNDLtoNODE(node);
			// 
		Blade::List<Node *> *list = &n->parent->nodes;
		ic_nodes i = n->iterator;
			// 
		if(i==list->begin())
			return (*i)->parent;
			// 
		for(; (*--i)->nodes.empty()==false; i=(*i)->nodes.end());
			// 
		return *i;
	}
		// 
	HNODE GetNext(HNODE node) const
	{	return GetNext((Node *)&m_NodeRoot,node);
	}
	HNODE GetNext(HNODE parent, HNODE node) const
	{	_ASSERTE(IsExist(parent)==true);
		_ASSERTE(IsExist(node)==true);
			// 
		Node *n = HNDLtoNODE(node);
			// 
		if(n->nodes.empty()==false)
			return n->nodes.front();
		if(node==parent) return NULL;
			// 
		Blade::List<Node *> *list = &n->parent->nodes;
		ic_nodes i = n->iterator;
			// 
		while(++i==list->end())
		{	Node *p = (*--i)->parent;
			if(p==parent) return NULL;
			list = &p->parent->nodes;
			i = p->iterator;
		}
			// 
		return *i;
	}
		// 
	HNODE GetLast() const
	{	return GetLast((Node *)&m_NodeRoot);
	}
	HNODE GetLast(HNODE parent) const
	{	_ASSERTE(IsExist(parent)==true);
			// 
		if(HNDLtoNODE(parent)->nodes.empty()==true) return parent;
			// 
		ic_nodes i;
		for(i = HNDLtoNODE(parent)->nodes.end(); (*--i)->nodes.empty()==false; i=(*i)->nodes.end());
		return *i;
	}

protected:
	Node *HNDLtoNODE(HNODE node) const
	{	return (Node *)node;
	}
	i_nodes GetIterator(HNODE node) const
	{	return HNDLtoNODE(node)->iterator;
	}
};
/////////////////////////////////////////////////////////////////////////////
#pragma warning(pop)
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

















