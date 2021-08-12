#include "Precompiled.h"

#include "EEIObject.h"

namespace eei
{

void EEIObject::initLogging(const char *logFolder, const char *logLevels)
{
	if (logFolder && logLevels && title()[0] != 0)
	{
		std::string base = title();
		if (base[0] == '(')
		{
			if (base[1] == '-')
				base = base.substr(5);
			else
				base = base.substr(4);
			size_t pos = base.find(":");
			if (pos != std::string::npos)
				base = base.substr(0, pos);
			for (std::string::iterator it = base.begin(); it != base.end(); it++)
				if (*it == ' ') *it = '_';
		}
		m_logFolder = logFolder;
		m_baseName = base;
		char bf[400];
		CLog::RefClog log = CLog::log(title());
		log->init((std::string(logFolder)+"\\" + base + "_log.txt").c_str());
		
		strcpy(bf, logLevels);
		char *p=strtok(bf, ", ");
		while (p)
		{
			if (!strcmp(p, "All"))
				setLogD(log);
			if (!strcmp(p, "Msg"))
				setLogMsg(log);
			if (!strcmp(p, "Err"))
				setLogErr(log);

			p = strtok(0, ", ");
		}
	}
}

} // end namespace

