#include "Precompiled.h"

#include "EEI_Errors.h"

namespace eei
{


ERinfo EEI_Error::m_errTable[] = 
{
	{EC_OK,							ET_None,	"ok"},
	{EC_InvalidVehParameter,		ET_Error,	"Invalid parameter"},
	{EC_UnknownUnits,				ET_Error,	"Units must be either Metric or Imperial"},
	{EC_NegativeNumberOfVehicles,	ET_Error,	"Negative number of vehicles"},
	{EC_UnknownJunction,			ET_Error,	"Unknown Junction"},
	{EC_NotFixedControl,			ET_Error,	"Fixed junction control expected"},
	{EC_CannotGetDurations,			ET_Error,	"Cannot obtain phase durations"},
	{EC_InvalidSectionID,			ET_Error,	"Invalid Section ID"},
	{EC_GetSectionInfFailed,		ET_Error,	"Failed to obtain section information"},

	{ EC_Last, ET_Last, ""}
};


//----------------------------------------------------------
//
//----------------------------------------------------------
void EEI_Error::log_error(ERcode ernum, const char *pMsg/*="\n"*/)
{
	const ERinfo &inf = info(ernum);
	sLogErr("EEI %s: %s%s", (const char *)severity(inf.type), inf.errStr, pMsg);
}

} // namespace


