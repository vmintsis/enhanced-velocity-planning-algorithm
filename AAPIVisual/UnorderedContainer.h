#ifndef _UnorderedContainer_H_
#define _UnorderedContainer_H_

template<typename K, typename D>
class UnorderedContainer
{
public:
	typedef std::pair<K, D> Item;
	typedef std::vector<Item> Cont;
	typedef typename Cont::iterator iterator;

	UnorderedContainer() { }
	UnorderedContainer(const UnorderedContainer &uc) { *this = uc; }

	UnorderedContainer &operator=(const UnorderedContainer &uc)
	{
		if (&uc != this)
		{
		}
		return *this;
	}

	iterator begin() { return m_cont.begin(); }
	iterator end() { return m_cont.end(); }
	iterator find(const K &key) { iterator i = begin(); for( ;i != end(); i++) if (i->first == key) return i; return end(); }
	D &operator[](const K &key) { iterator i = find(key); if (i != end()) return i->second; m_cont.push_back(Item(key, D())); return m_cont[m_cont.size()-1].second; }
	void erase(iterator i) { m_cont.erase(i); }
	void erase(iterator s, iterator e) { m_cont.erase(s, e); }
	void clear() { erase(begin(), end()); }
	size_t size() { return m_cont.size(); }

private:
	Cont m_cont;
};

#endif