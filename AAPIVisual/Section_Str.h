#ifndef _Section_Str_H_
#define _Section_Str_H_

struct section_str
{
	section_str() : _s("") { }
	section_str(const char *s) { REQUIRE(s != 0); _s = s;} 

	std::vector<int> toIDs()
	{
		std::vector<int> ids;
		for (char *p = (char *)_s.c_str(); *p; )
		{
			ids.push_back( atoi(p));
			while(*p != ',' && *p) p++;
			if (*p == ',') p++;
		}
		return ids;
	}

	std::string toStr(std::vector<int> &ids)
	{
		_s="";
		char bf[80];
		for (size_t i=0; i<ids.size(); i++)
		{
			sprintf(bf, "%d", ids[i]);
			_s += bf;
			if (i<ids.size() - 1) _s+= ",";
		}
		return _s;
	}

	std::string _s;
};


#endif

