#ifndef _EEIObject_H_
#define _EEIObject_H_

#include "CLog.h"

namespace eei
{

class EEIObject
{
public:
	typedef std::pair<double, double> Range;
	typedef std::vector<std::pair<std::string, int>> VehicleVector;

	EEIObject()
	{
	}

	virtual const char *title() { return "";}
	virtual void setTitle(const char *t) { }

	virtual const char *intersectionName() { return "";}
	virtual const char *roadName() { return ""; }

	virtual int getScenarioID() = 0;
	virtual std::string getScenarioName() = 0;
	virtual int getJunctionID() = 0;
	virtual std::string getSectionIDstr() const = 0;
	virtual Range getRangeToJunction() = 0;
	virtual double accelFactor() = 0;
	virtual void infZoneCalibParam(double f) = 0;
	virtual double infZoneCalibParam()  = 0;

	virtual double minSpeed() = 0;
	virtual bool enableCFR()  = 0;
	virtual bool enforceMinSpeed()  = 0;
	virtual double getCycleDuration() = 0;
	virtual double getGreenTime() = 0;

	virtual VehicleVector vehicles(char *quote="'") = 0;
	virtual bool enabled() = 0;
	virtual void enable() = 0;
	virtual void disable() = 0;

	CLog::RefClog logD() { return m_logD; }
	CLog::RefClog logMsg() { return m_logMsg; }
	CLog::RefClog logErr() { return m_logErr; }

	void setLogD(CLog::RefClog l) { m_logD = l; }
	void setLogMsg(CLog::RefClog l) { m_logMsg = l; }
	void setLogErr(CLog::RefClog l) { m_logErr = l; }

	void initLogging(const char *logFolder, const char *logLevels);

	std::string logFolder() { return m_logFolder; }
	std::string baseName() { return m_baseName; }

private:
	std::string m_logFolder;
	std::string m_baseName;
	CLog::RefClog m_logD;
	CLog::RefClog m_logMsg;
	CLog::RefClog m_logErr;
};

}

#endif
