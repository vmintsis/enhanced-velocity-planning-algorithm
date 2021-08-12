#ifndef _CLog_H_
#define _CLog_H_

#include "TSptr.h"

////////////////////////////////////////
//
// Aimsun-related generic logger class
//

#define LOG_BUF_SZ 8192

class CLog
{
public:
	CLog() : f(0), m_enabled(true) 
	{ 
//		LOG("CLOG CTOR %p\n", this);
	}
	CLog(const CLog &cl): f(0), m_enabled(true) { *this = cl; }
	virtual ~CLog() 
	{ 
		// TODO: add a log here before we do anything in this code in addition to the end log and check if we see both at end
		m_enabled = false;
		try{
			if (f) fclose(f); 
		}catch(std::exception e)
		{
			LOG("CLOG ***EXCEPTION DTOR %s (%p)\n", id().c_str(), this);
		}
		f = 0;

		LOG("CLOG DTOR %s (%p)\n", id().c_str(), this);
	}

	CLog &operator=(const CLog &st) 
	{
		if (this != &st)
		{
			LOG("CLOG op=(%s, %s)\n", m_logID.c_str(), ((CLog &)st).id().c_str());
			if (f) { fclose(f); f = 0; }
			m_fname = st.m_fname;
		}
		return *this;
	}

	typedef TSptr<CLog> RefClog;
	typedef std::map<std::string, RefClog> LogMap;

	static RefClog log(const char *log_name=0)
	{
//		static CLog _cl;
		static LogMap _lm;

		if (log_name == 0) log_name = "default";

		RefClog pLog;
		LogMap::iterator li = _lm.find(log_name);
		if (li == _lm.end())
		{
			pLog = _lm[log_name] = RefClog().newObj();
			pLog->id() = log_name;
			LOG("CLOG CTOR %s (%p)\n", log_name, pLog);
		}
		else
			pLog = li->second;

		return pLog;
	}

	void show(const char *format, ...)
	{
		if (enabled())
		{
			va_list va;
			va_start(va, format);
			vsprintf_s(m_buf, LOG_BUF_SZ, format, va);
			AKIPrintString(m_buf);
			_flog(m_buf);
		}
	}

	void fshow(const char *format, ...)
	{
		if (enabled())
		{
			va_list va;
			va_start(va, format);
			vsprintf_s(m_buf, LOG_BUF_SZ, format, va);
			_flog(m_buf);
		}
	}

	void init(const char *fname)
	{
		REQUIRE(fname != 0);
		m_fname = fname;
		f = fopen(m_fname.c_str(), "wb");
		if (f != 0)
		{
			fclose(f);
			f = 0;
		}
	}

	bool enabled() { return m_enabled; }
	void setEnabled(bool t) { m_enabled = t; }

	std::string& id() { return m_logID; }
protected:

	void _flog(const char *buff)
	{
		try{
			if (!m_fname.empty())
			{
				if (f == 0)
					f = fopen(m_fname.c_str(), "a+");
				if (f != 0)
				{
					fwrite(buff, 1, strlen(buff), f);
					fclose(f);	f = 0;
				}
			}
		}
		catch(std::exception e) // EXCEPTION_CONTINUE_EXECUTION)
		{
			LOG("CLOG _flog EXCEPTION %p\n", this);
		}
	}

private:
	char m_buf[LOG_BUF_SZ];
	std::string m_fname;
	std::string m_logID;
	FILE *f;
	bool m_enabled;
};
#endif