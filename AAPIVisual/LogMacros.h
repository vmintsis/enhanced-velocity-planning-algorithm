#ifndef _LogMacros_H_
#define _LogMacros_H_

// Independent debug only logging

inline std::string logs_location(const char *log_relative_path=0)
{
	std::string loc =  getenv("USERPROFILE"); // has no terminating '\\'
	loc += "\\EEI_Files";
	if (log_relative_path)
		loc += log_relative_path;
	if (loc.find('.') != std::string::npos)
		MakeSureDirectoryPathExists(loc.c_str());
	else
		MakeSureDirectoryPathExists((loc+"\\").c_str());
	return loc;
}


#define LOG(x, ...) do { FILE *f = fopen( logs_location("\\CDlog.txt").c_str(), "a+"); \
					if (f){ fprintf(f, x, __VA_ARGS__); \
					fclose(f); } } while(0);

//#define LOG(x, ...) 


// logging enabler macros

#define _sLog_
#define _sLogD_
#define _sLogMsg_
#define _sLogErr_

#define _flog_
#define _flogMsg_
#define _flogErr_

// ============ GLOBAL LOGGING ===============
// informative (debug) log
#ifdef _sLog_
#define sLog(x, ...) CLog::log()->show(x, __VA_ARGS__)
#define fLog(x, ...) CLog::log()->fshow(x, __VA_ARGS__)
#else
#define sLog(x, ...)
#define fLog(x, ...)
#endif

// debug log
#ifdef _sLogD_
#define sLogD(x, ...) CLog::log()->show(x, __VA_ARGS__)
#define fLogD(x, ...) CLog::log()->fshow(x, __VA_ARGS__)
#else
#define sLogD(x, ...)
#define fLogD(x, ...)
#endif

// message log
#ifdef _sLogMsg_
#define sLogMsg(x, ...) CLog::log()->show(x, __VA_ARGS__)
#define fLogMsg(x, ...) CLog::log()->fshow(x, __VA_ARGS__)
#else
#define sLogMsg(x, ...)
#define fLogMsg(x, ...)
#endif

// error log
#ifdef _sLogErr_
#define sLogErr(x, ...) CLog::log()->show(x, __VA_ARGS__)
#define fLogErr(x, ...) CLog::log()->fshow(x, __VA_ARGS__)
#else
#define sLogErr(x, ...)
#define fLogErr(x, ...)
#endif

// ============ EEI LOGGING ===============
#ifdef _flog_
#define flog(x, ...) if (!logD().empty()) logD()->fshow("%s: "x, title(), __VA_ARGS__)
#define slog(x, ...) if (!logD().empty()) logD()->show("%s: "x, title(), __VA_ARGS__)
#define pflog(x, ...) if (m_pParent && !m_pParent->logD().empty()) m_pParent->logD()->fshow("%s: "x, m_pParent->title(), __VA_ARGS__)
#define pslog(x, ...) if (m_pParent && !m_pParent->logD().empty()) m_pParent->logD()->show("%s: "x, m_pParent->title(), __VA_ARGS__)
#else
#define flog(x, ...)
#define slog(x, ...)
#define pflog(x, ...)
#define pslog(x, ...)
#endif

#ifdef _flogMsg_
#define flogMsg(x, ...) if (!logMsg().empty()) logMsg()->fshow("%s: "x, title(), __VA_ARGS__)
#define slogMsg(x, ...) if (!logMsg().empty()) logMsg()->show("%s: "x, title(), __VA_ARGS__)
#define pflogMsg(x, ...) if (m_pParent && !m_pParent->logMsg().empty()) m_pParent->logMsg()->fshow("%s: "x, m_pParent->title(), __VA_ARGS__)
#define pslogMsg(x, ...) if (m_pParent && !m_pParent->logMsg().empty()) m_pParent->logMsg()->show("%s: "x, m_pParent->title(), __VA_ARGS__)
#else
#define flogMsg(x, ...)
#define slogMsg(x, ...)
#define pflogMsg(x, ...)
#define pslogMsg(x, ...)
#endif

#ifdef _flogErr_
#define flogErr(x, ...) if (!logErr().empty()) logErr()->fshow("%s: "x, title(), __VA_ARGS__)
#define slogErr(x, ...) if (!logErr().empty()) logErr()->show("%s: "x, title(), __VA_ARGS__)
#define pflogErr(x, ...) if (m_pParent && !m_pParent->logErr().empty()) m_pParent->logErr()->fshow("%s: "x, m_pParent->title(), __VA_ARGS__)
#define pslogErr(x, ...) if (m_pParent && !m_pParent->logErr().empty()) m_pParent->logErr()->show("%s: "x, m_pParent->title(), __VA_ARGS__)
#else
#define flogErr(x, ...)
#define slogErr(x, ...)
#endif


#endif
