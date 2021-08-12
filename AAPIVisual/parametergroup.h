#ifndef _ParameterGroup_H_
#define _ParameterGroup_H_

struct ParameterGroup
{
	ParameterGroup() :
			m_JunctionID(-1), 
			m_SignalGroup(-1), 
			m_StartPoint(-1),
			m_EndPoint(-1),
			m_accelFactor(ACCCEL_FACTOR),
			m_enableCFR(false),
			m_SectionID(""),
			m_infZoneCalibParam(INFLUENCE_ZONE_CALIBRATION_PARAM),
			m_minSpeed(MIN_SPEED),
			m_enforceMinSpeed(false)

//			m_idJunction(-1), m_signalGroup(-1)
//			m_distToJunction1(-1), m_distToJunction2(-1), 
//			m_accelFactor(ACCCEL_FACTOR), m_enableCFR(false), m_infl_zone_calib_param(INFLUENCE_ZONE_CALIBRATION_PARAM),
//			m_min_speed(MIN_SPEED)
	{ }

	ParameterGroup(const ParameterGroup &pg) :
			m_JunctionID(0), 
			m_SignalGroup(0), 
			m_StartPoint(0),
			m_EndPoint(0),
			m_accelFactor(0),
			m_enableCFR(false),
			m_SectionID(""),
			m_infZoneCalibParam(0),
			m_minSpeed(0.0),
			m_enforceMinSpeed(false)
	{
		*this = pg;
	}

	template<typename T>
	ParameterGroup &operator=(const T &pg)
	{
		if (this != &pg)
		{
			junctionID(pg.junctionID());
			signalGroup(pg.signalGroup());
			startPoint(pg.startPoint());
			endPoint(pg.endPoint());
			accelFactor(pg.accelFactor());
			enableCFR(pg.enableCFR());
			infZoneCalibParam(pg.infZoneCalibParam());
			minSpeed(pg.minSpeed());
			enforceMinSpeed(pg.enforceMinSpeed());
			sectionID(pg.sectionID());		
			printf("========== this is my T op= =============\n");
		}
		return *this;
	}

	ParameterGroup &operator=(const ParameterGroup &pg) { return this->operator=<ParameterGroup>(pg); }
/*
	ParameterGroup &operator=(const ParameterGroup &pg)
	{
		if (this != &pg)
		{
			junctionID(pg.junctionID());
			signalGroup(pg.signalGroup());
			startPoint(pg.startPoint());
			endPoint(pg.endPoint());
			accelFactor(pg.accelFactor());
			enableCFR(pg.enableCFR());
			infZoneCalibParam(pg.infZoneCalibParam());
			minSpeed(pg.minSpeed());
			sectionID(pg.sectionID());		
			printf("========== this is my op= =============\n");
		}
		return *this;
	}
*/
	bool isValid()
	{
		return	m_JunctionID > 0 && 
			m_SignalGroup > 0 && 
			atoi(sectionID()) > 0 && 
			m_infZoneCalibParam >0.01 &&
			m_StartPoint > 0.0 &&
			m_EndPoint > 0.0 && m_EndPoint < m_StartPoint &&
			m_accelFactor > 0.0 && m_minSpeed >= 0.0;
	}

	int junctionID() const { return m_JunctionID; }
	void junctionID(int jid) { m_JunctionID = jid; }
	int signalGroup() const { return m_SignalGroup; }
	void signalGroup(int sg) { m_SignalGroup = sg; }
	double startPoint() const { return m_StartPoint; }
	void startPoint(double sg) { m_StartPoint = sg; }
	double endPoint() const { return m_EndPoint; }
	void endPoint(double sg) { m_EndPoint = sg; }
	double accelFactor() const { return m_accelFactor; }
	void accelFactor(double a) { m_accelFactor = a; }
	bool enableCFR() const { return m_enableCFR; }
	void enableCFR(bool e) { m_enableCFR = e; }
	void infZoneCalibParam(double f) { m_infZoneCalibParam = f; }
	double infZoneCalibParam() const { return m_infZoneCalibParam;}
	double minSpeed() const { return m_minSpeed; }
	void minSpeed(double s) { REQUIRE( s >= 0.0); m_minSpeed = s; }
	bool enforceMinSpeed() const { return m_enforceMinSpeed; }
	void enforceMinSpeed(bool b) { m_enforceMinSpeed = b; }
	const char *sectionID() const { return m_SectionID.c_str(); }
	void sectionID(const char *sid) { REQUIRE(sid != 0); m_SectionID = sid; }

	int m_JunctionID;			// numerical ID of junction (def. -1 - invalid)
	int m_SignalGroup;			// signal group in junction (def. -1 - invalid)
	double m_StartPoint;	// vehicle distance to junction when comms are established between RSU and OBU (def. 200)
	double m_EndPoint;   // VagCode: vehicle distance to junction below which comms cannot be estab. between RSU and OBU
	double m_accelFactor;	// Acceleration factor (used in 's' calculation)
	bool m_enableCFR;		// enable handling car following conditions
	double m_infZoneCalibParam;	// Influence zone calibration parameter (see Velocity profile)
	double m_minSpeed;			// minimum allowable speed
	bool m_enforceMinSpeed;		// True if minimum allowable speed is enforced, False otherwise
	std::string m_SectionID;		// numerical ID of section (def. -1 - invalid)
};


#endif

