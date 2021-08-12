#ifndef _Statistics_H_
#define _Statistics_H_


namespace eei
{

class Statistics
{
public:
	enum ADV_REASON
	{
		AR_ACCELERATE,
		AR_DECELERATE
	};

	struct VehStat
	{
		int vehInZone;				// sum of vehicles in comms zone
		int speedAdv;				// number of vehicles that received speed advice
		int accelAdv;				// number of vehicles advised to accelerate
		int declAdv;				// number of vehicles advised to decelerate
		double avgSpeed;			// avg speed of all vehicles that entered comms zone
		double lastSpeed;			// used to calculate moving average
		int numUntracked;			// number of vehicles that entered car-following and were released from advise

		//TODO: rename numUntracked to numUnadvised since we only unadvise but we dont release from tracking
		//		add counters for unadvised vehicles due to non-compliance (possibly per-xform type)

		VehStat() : vehInZone(0), speedAdv(0), accelAdv(0), declAdv(0), lastSpeed(0.0), avgSpeed(0.0), numUntracked(0)
		{
		}
	};
	typedef std::map<int, VehStat> VehStatMap;  // map of statistics keyed on vehicle type ID

	std::string toStr(int x) { char bf[100]; sprintf(bf, "%d", x); return bf; }
	std::string toStr(double x) { char bf[100]; sprintf(bf, "%3.2f", x); return bf; }

// Construtor / destructor
public:
	Statistics(EEIObject *parent=0);

// Methods/accessors
public:
	void setParent(EEIObject *parent) {m_pParent = parent; }
	EEIObject *parent() { return m_pParent; }
	void enable() { m_enabled = true;}
	void disable() { m_enabled = false;}
	BOOL isEnabled() { return m_enabled; }
	std::string toHTML();

// Events
public:
	void OnEnteredZone(int vehType, double veh_speed);
	void OnAdvised(int vehType, ADV_REASON reason, double veh_speed);
	void OnUnadvised(int vehType);

// MFC-specific
private:
	std::string populate_MFC_HTMLresource(std::string &js);

private:
	EEIObject *m_pParent;
	VehStatMap m_map;
	BOOL m_enabled;
};

} // namespace


#endif

