#ifndef _VehicleTypes_H_
#define _VehicleTypes_H_

namespace eei 
{

class VehicleTypes
{
public:
	typedef std::map<std::string, int> VehMap;
	typedef VehMap::iterator iterator;
	typedef VehMap::const_iterator const_iterator;
	typedef std::vector<std::pair<std::string, int>> VehicleVector;

	VehicleTypes() { }
	VehicleTypes(const VehicleTypes &vt) { *this = vt; }
	~VehicleTypes() { }

	VehicleTypes &operator=(const VehicleTypes &vt)
	{
		if (this != &vt)
		{
			m_vmap = vt.m_vmap;
		}
		return *this;
	}

	bool empty() { return begin() == end(); }
	void clear() { m_vmap.clear(); }
	iterator begin() { return m_vmap.begin(); }
	const_iterator begin() const { return m_vmap.begin(); }
	iterator end() { return m_vmap.end(); }
	const_iterator end() const { return m_vmap.end(); }

	iterator find(const char *name) { return m_vmap.find(name); }
	const_iterator find(const char *name) const { return m_vmap.find(name); }
	iterator find(int vt) { for (iterator it = begin(); it != end(); it++) if (it->second == vt) return it; return end(); }

	bool contains(const char *name) { return find(name) != end(); }
	bool contains(int vt) { return find(vt) != end(); }
	VehicleTypes &add(const char *name, int id) { if (!contains(name)) m_vmap[name] = id; return *this; }
	VehicleTypes &remove(const char *name) { iterator it = find(name); if (it != end()) m_vmap.erase(it); return *this;}
	size_t size() const { return m_vmap.size(); }

	bool operator==(const VehicleTypes &vt)
	{
		bool b = size() == vt.size();
		for (iterator i=begin(); b && i!= end(); i++)
		{
			const_iterator vti = vt.find(i->first.c_str());
			b = vti != vt.end() && vti->second == i->second;
		}
		return b;
	}
	bool operator!=(const VehicleTypes &vt) { return !operator==(vt); }

	VehicleVector toVector(char *quote="'")
	{
		VehicleVector vec;
		for (iterator i = begin(); i!= end(); i++)
			vec.push_back( std::pair<std::string, int>( quote + i->first + quote, i->second) );

		return vec;
	}

private:
	VehMap m_vmap;
};

} // namespace

#endif

