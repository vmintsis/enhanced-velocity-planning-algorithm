#ifndef _EEI_Errors_H_
#define _EEI_Errors_H_

namespace eei
{


// message severity levels
enum ERtype
{
	ET_None,
	ET_Warning,
	ET_Error,

	ET_Last
};

// severity level code to string functor
struct severity
{
	severity(ERtype er) : _er(er) { }
	operator const char *() 
	{
		switch(_er)
		{
			case ET_None : return "Message";
			case ET_Warning : return "Warning";
			case ET_Error : return "ERROR";
			default:
				return "***";
		}
	}

	ERtype _er;
};

enum ERcode
{
	EC_OK,
	EC_InvalidVehParameter,
	EC_UnknownUnits,
	EC_NegativeNumberOfVehicles,
	EC_UnknownJunction,
	EC_NotFixedControl,
	EC_CannotGetDurations,
	EC_InvalidSectionID,
	EC_GetSectionInfFailed,

	EC_Last
};

struct ERinfo
{
	ERcode err;
	ERtype type;
	const char *errStr;
};

//////////////////////////////////////////
//
//
class EEI_Error
{
	EEI_Error() 
	{ 
	}
public:

	static EEI_Error *err() 
	{
		static EEI_Error _er;
		return &_er;
	}

	const ERinfo &info(ERcode ernum)
	{
		int i;
		for (i=0; m_errTable[i].err != EC_Last; i++)
		{
			if (m_errTable[i].err == ernum) break;
		}
		return m_errTable[i];
	}

	void log_error(ERcode ernum, const char *pMsg="\n");

private:
	static ERinfo m_errTable[];
};

} // namespace

#endif