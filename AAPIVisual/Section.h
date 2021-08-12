#ifndef _Section_H_
#define _Section_H_

#include "EEIObject.h"

namespace eei
{


//////////////////////////////////////////////
//
//
class Section
{
// construction/destruction
public:

	struct sectionInfo
	{
		enum si_type
		{
			si_invalid,

			si_section,
			si_junction,

			si_last
		};

		sectionInfo() : _sid(-1), _length(0), _numVeh(-1), _numLanes(0), _speedLimit(0.0), _type(si_invalid) { }
		sectionInfo(int sid, int nv=-1) : _sid(sid), _length(0), _numVeh(nv), _numLanes(0), _speedLimit(0.0), _type(si_invalid) { }
		sectionInfo(const sectionInfo &si) : _sid(-1), _length(0), _numVeh(-1), _numLanes(0), _speedLimit(0.0), _type(si_invalid) { *this = si; }

		sectionInfo &operator=(const sectionInfo &si)
		{
			if (this != &si)
			{
				setID(si.id()).setLength(si.length()).setNumVeh(si.numVeh());
				setNumLanes(si.numLanes()).setSpeedLimit(si.speedLimit());\
				setType(si.type());
			}
			return *this;
		}

		sectionInfo &setID(int id) { _sid = id; return *this; }
		sectionInfo &setLength(double l) { _length = l; return *this; }
		sectionInfo &setNumVeh(int n) { _numVeh = n; return *this; }
		sectionInfo &setNumLanes(int n) { _numLanes = n; return *this; }
		sectionInfo &setSpeedLimit(double sl) { _speedLimit = sl; return *this; }
		sectionInfo &setType(si_type t) { _type = t; return *this; }
		int id() const { return _sid; }
		double length() const { return _length; }
		int numVeh() const { return _numVeh;}
		int numLanes() const { return _numLanes;}
		double speedLimit() const { return _speedLimit;}
		si_type type() const { return _type;}

		std::string toString() { char bf[300]; sprintf(bf, "type=%s, id:%d, length:%f, nVeh:%d, lanes:%d, spLim:%f", 
			type() == si_section ? "section" :
			type() == si_junction ? "junction" : "<invalid>",
			id(), length(), numVeh(), numLanes(), speedLimit()); return bf; }

		bool validType() { return _type > si_invalid && _type < si_last; }
		bool isvalid() { return _sid > 0 && _length > 0.0 && _numVeh >=0 && _numLanes >=1 && _speedLimit > 0.0 && validType(); }

	private:
		int _sid;
		double _length;
		int _numLanes;
		int _numVeh;
		double _speedLimit;
		si_type _type;
	};

	// auto-incrementing key: used to order map elements in the order given and not by section/junction ID
	struct key 
	{
		enum key_index
		{
			ki_none,
			ki_auto_inc
		};

		// default: will not auto_increment
		key(key_index inc=ki_none) : _index(inc==ki_auto_inc ? key::indval()++ : key::indval() ), _id(0) {}
		key(const key &k) : _index(k.index()), _id(k.id()) {  }

		key &operator=(const key &k) 
		{
			if (this != &k)
				index(k.index()).id(k.id());
			return *this;
		}

		static int &indval()
		{
			static int _ind=0;
			return _ind;
		}

		key &index(int x) { _index=x; return *this; }
		key &id(int x) { _id=x; return *this; }
		int index() const { return _index; }
		int id() const { return _id; }

		bool operator<(const key &k) 
		{ 
			return _index < k.index(); 
		}
		bool operator<(const key &k) const 
		{ 
			return _index < k.index(); 
		}
		operator int() const { return _id; }

		int _index;
		int _id;
	};
	typedef std::map<key, sectionInfo> sectionMap_t;  // pair<section id, section info>

	Section(EEIObject *parent=0) : m_vsid(-1), m_speedLimitsCalculated(false), m_pParent(parent) { }
	virtual ~Section() { }

// operations
public:

	void setParent(EEIObject *parent) { m_pParent = parent; }
	EEIObject *parent() { return m_pParent; }
	
	void addSections(std::vector<int> &ids) 
	{  
		REQUIRE(ids.size() > 0);

		for (size_t i=0; i<ids.size(); i++)
		{
			int id = ids[i];
			key k(key::ki_auto_inc);  // create an auto-incrementing key
			k.id(id);
			if (m_sid.find(k) == m_sid.end())
			{
				A2KSectionInf si = SP.f_AKIInfNetGetSectionANGInf(id);
				if (si.report != 0)
				{
					m_sid[k] = sectionInfo(id).setType(sectionInfo::si_junction); // must add the junction first so getTurnFromJunction can find it later!
				}
				else
				{
					m_sid[k] = sectionInfo(id).setType(sectionInfo::si_section).setLength(si.length).setNumLanes(si.nbCentralLanes);
				}
			}
		}

		for (sectionMap_t::iterator it = m_sid.begin(); it != m_sid.end(); it++)
		{
			// only junction are not valid, since we need all the surrounding sections to fill in their data
			if (it->second.type() == sectionInfo::si_junction) 
			{
				A2KTurnInf ti = getTurnFromJunction(it->first);
				if (ti.report == 0)
				{
					pflog("junction %d, turn:%d, nextSec:%d len:%f spl:%f\n", it->first.id(), ti.id, ti.destinationSectionId, ti.length, ti.speedLimit);
					// CAUTION: simplification is made by using as number of virtual lanes the number of lanes of the previous section
					A2KSectionInf si = SP.f_AKIInfNetGetSectionANGInf(ti.originSectionId);
					it->second.setLength(ti.length).setNumLanes(si.nbCentralLanes); // now fill in rest of data
				}
				else
				{
					pflogErr("*** Given junction %d is NOT a junction, invalidating node.\n", it->first.id());
					it->second.setType(sectionInfo::si_invalid);
				}
			}
		}
	}

	void removeSection(int id) 
	{
		for (sectionMap_t::iterator it = m_sid.begin(); it != m_sid.end(); it++)
			if (it->first.id() == id)
			{
				m_sid.erase(it);
				break;
			}
	}

	void updateSectionInfo() // call this on every time step
	{
		if (!m_speedLimitsCalculated)  // calculate speed limits once
		{
			calcSpeedLimits();
		}

		for (sectionMap_t::iterator it = m_sid.begin(); it != m_sid.end(); it++)
		{
			int n = 0;
			if (it->second.type() == sectionInfo::si_section)
				n = SP.f_AKIVehStateGetNbVehiclesSection(it->first, true);
			else
				n = SP.f_AKIVehStateGetNbVehiclesJunction(it->first);
			if (n >= 0)
				it->second.setNumVeh(n);
			else
			{
				Assert("*** Failed to update number of vehicles(no section or junction specified)"==0);
			}
		}
	}
protected:
	// In this version, all speed limits in the sections/junctions of the virtual section must be the same.
	// No signal junction and no stop signs can be included in an intermediate junction.
	// All sections and junctions in the virtual section set must form a continguous sequence.
	void calcSpeedLimits()
	{
		m_speedLimitsCalculated = true;

		for (sectionMap_t::iterator it = m_sid.begin(); it != m_sid.end(); it++)
		{
			A2KSectionInf si = SP.f_AKIInfNetGetSectionANGInf(it->first);
			double sl = si.speedLimit;
			if (si.report != 0)
			{
				// if it's not a section, then find which turn leads to the next sub-section in the virtual section, 
				// and use that turn's speed limit
				sectionMap_t::iterator itNext = it;
				itNext++;
				if (itNext == m_sid.end())
				{
					pflogErr("***Virtual section ends in a junction (%d) immediately before signal junction!\n", it->first.id());
					return;
				}

				// Find the turn that connects our sections and use its speed limit
				// The junction may have several turns with different speed limits that refer to different section connections
				A2KTurnInf turn = getTurnFromJunction(it->first.id());
				if (turn.report == 0)
				{
					sl = turn.speedLimit;
					si.report = 0;
				}
				else
					break;
			}

			if (sl <= 0.0)
				pflogErr("***Zero speed limit for subsection %d.\n", it->first.id());
			it->second.setSpeedLimit(sl);
		}
	}

	// find which turn of the given junction leads to our next section
	A2KTurnInf getTurnFromJunction(int jid)
	{
		A2KTurnInf ti;
		ti.report = -1;

		sectionMap_t::iterator it;  // find this junction in the vsection
		sectionMap_t::iterator it_prev;  // find section before this junction in the vsection
		for (it = m_sid.begin(); it != m_sid.end() && it->first.id() != jid ; it++)
			it_prev = it;

		Assert( it != m_sid.end() || "***getTurnFromJunction: jid not in virtual section!"==0);
		int OrgSID = it_prev->second.id();
		it++;  // move to next - MUST be section: cannot end in a junction since the final node is always a junction where the signals are

		if (it != m_sid.end())
		{
			int nTurns = SP.f_AKIInfNetGetNbTurnsInNode(jid);
			int i;
			pflog("Junction %d -> %d turns\n",OrgSID, nTurns); 
			for (i=0; i<nTurns; ++i)
			{
				A2KTurnInf ti1 = SP.f_AKIInfNetGetTurnInfo(jid, i);
				int idDestSec = ti1.destinationSectionId;
				int idOrgSec = ti1.originSectionId;
//				int idDestSec = SP.f_AKIInfNetGetDestinationSectionInTurn(jid, i);
//				int idOrgSec = SP.f_AKIInfNetGetOriginSectionInTurn(jid, i);

				pflog("Junction %d turn=%d (id=%d) Org=%d Dest=%d (need: org=%d dest=%d)\n",OrgSID, i, ti1.id, idOrgSec, idDestSec, OrgSID, it->second.id()); 
//				pflog("Junction %d turn=%d Org=%d Dest=%d (need: org=%d dest=%d)\n",OrgSID, i, idOrgSec, idDestSec, OrgSID, it->second.id()); 
				if (idOrgSec == OrgSID && idDestSec == it->second.id())
					break;
			}
			if (i < nTurns)
				ti = SP.f_AKIInfNetGetTurnInfo(jid, i);
			else
				pflogErr("***No turn in sub-junction %d of virtual section leads to next sub-section!\n", jid);
		}
		else
			pflogErr("***Sub-junction %d of virtual section is last in sequence!\n", jid);
		return ti;
	}

// accessors
public:
	sectionMap_t &smap() { return m_sid; }

	int numVehicles()
	{
//		if (m_sid.begin()->second.numVeh() == 0)
//			updateSectionInfo();

		int nVehicles = 0;
		for (sectionMap_t::iterator it = m_sid.begin(); it != m_sid.end(); it++)
			nVehicles += it->second.numVeh();
		return nVehicles;
	}

	InfVeh GetVehicleInfSection(int ithVeh)
	{
		InfVeh iv;
		iv.report = -1;  // AimSun: not available - TODO: sync all error info everywhere
		int nv = 0, np=0;
		double from_start = 0.0;

//		CLog::log("VS")->fshow("--------------------------------\n");

		// [       ][     ][ 3  ][   5    ][     ]
		for (sectionMap_t::iterator it = m_sid.begin(); it != m_sid.end(); it++)
		{
			nv += it->second.numVeh();
			if (ithVeh < nv && nv > 0)
			{
				if (it->second.type() == sectionInfo::si_section)
				{
					SP.f_AKIVehStateGetNbVehiclesSection(it->first, true); // ???? AimSun requirement before any calls to vehicle information!
					iv = SP.f_AKIVehStateGetVehicleInfSection(it->first, ithVeh - np);
				}
				else
				{
					SP.f_AKIVehStateGetNbVehiclesJunction(it->first); // ???? AimSun requirement before any calls to vehicle information!
					iv = SP.f_AKIVehStateGetVehicleInfJunction(it->first, ithVeh - np);
				}

				// AimSun doc says that iv.idLaneFrom is 'number of lanes of the origin section where the vehicle enters the junction'
				// However, in version 8.0.3(R26609), it appears that idLaneFrom is the index of the lane where the vehicle was.
				// TODO: clarify this!
				if (it->second.type() == sectionInfo::si_junction)
				{
					iv.numberLane = iv.idLaneFrom;  // use orign lane as current when in a junction 
//					CLog::log("VS")->fshow("GetVehInfSec: >>>IN JUNCTION ith=%d fixing lane: (from=%d, to=%d)", ithVeh, iv.numberLane, iv.idLaneTo );
				}

//				CLog::log("VS")->fshow("GetVehInfSec: ith=%d, nv=%d, np=%d, sec/jun=%d status=%d\n", ithVeh, nv, np, it->first.id(), iv.report);

				iv.CurrentPos += from_start;  // correct CurrentPos to include all previous section/junction lengths
				sectionMap_t::iterator it1 = it;  // fix distance2End to include all lenghts after the current
				it1++;
				while (it1 != m_sid.end()) 
				{
					iv.distance2End += it1->second.length();
					it1++;
				}

//				CLog::log("VS")->fshow("GetVehInfSec: ith=%d in {%s} curpos=%f dist2End=%f \n", ithVeh, it->second.toString().c_str(), iv.CurrentPos, iv.distance2End);

				break;
			}
			from_start += it->second.length(); // keep all previous distances so we can correct iv.CurrentPos
			np = nv;
		}
//		CLog::log("VS")->fshow("GetVehInfSec: ith=%d, DONE nv=%d, np=%d, status=%d\n", ithVeh, nv, np, iv.report);
		return iv;
	}

	// Return speed limit of first section (see calcSpeedLimits function)
	double speedLimit()
	{
		double sl = 0.0;
		if (!m_sid.empty())
		{
			sl = m_sid.begin()->second.speedLimit();
/*
			A2KTurnInf ti = SP.f_AKIInfNetGetTurnInf(m_sid.begin()->first);
			double sl = ti.speedLimit;
			if (ti.report != 0)
			{
				A2KSectionInf si = SP.f_AKIInfNetGetSectionANGInf(m_sid.begin()->first);
				sl = si.speedLimit;
			}
*/
		}
		else
			pflogErr("***Speed limit requested for empty virtual section.\n");
		return sl;
	}

	bool isvalid() 
	{
		bool r = false;
		if (m_speedLimitsCalculated) 
		{
			bool ok = true;
			for (sectionMap_t::iterator it = m_sid.begin(); ok && it != m_sid.end(); it++)
				r = ok = it->second.isvalid();
		}

		return r;
	}

	bool contains(int idSection)
	{
		for (sectionMap_t::iterator it = m_sid.begin(); it != m_sid.end(); it++)
			if (it->second.id() == idSection)
				return true;
		return false;
	}
	bool isLastSection(int idSection)
	{
		return m_sid.rbegin()->second.id() == idSection;
	}

	std::vector<int> getSectionIDs()
	{
		std::vector<int> ids;
		for (sectionMap_t::iterator it = m_sid.begin(); it != m_sid.end(); it++)
			ids.push_back(it->first);
		return ids;
	}

//	void setSectionID(int id) { REQUIRE(id >0); m_vsid = id; }
//	int getSectionID() { return m_vsid; }


// data
private:
	int m_vsid;						// virtual section id (identifies all real section ids managed by this class)
	sectionMap_t m_sid;				// real section ids - must be consecutive to form a contiguous virtual section
	bool m_speedLimitsCalculated;	// true if speed limits in virtual section have been calculated(def. false)
	EEIObject *m_pParent;
};

} // namespace

#endif

