#ifndef _Settings_H_
#define _Settings_H_

#include "ParameterGroup.h"
#include "VehicleTypes.h"
#include "UnorderedContainer.h"

#pragma warning(disable: 4996)

class Settings
{
public:

	struct paramGroup : public ParameterGroup
	{
		paramGroup() { }
		paramGroup(const paramGroup &pg) { *this = pg; }

		template<typename T>
		paramGroup &operator=(const T &pg)
		{
//			if (this != &pg)
//			{
				ParameterGroup::operator=(pg);
//			}
			return *this;
		}

		std::string read(const char *sec, const char *path)
		{
			char str[1024] = {0};
			GetPrivateProfileString(sec, "SectionID", "0", str, sizeof(str), path);
			sectionID(str);
			junctionID( GetPrivateProfileInt(sec, "JunctionID", 0, path) );
			signalGroup( GetPrivateProfileInt(sec, "SignalGroup", 0, path) );
			GetPrivateProfileString(sec, "StartPoint", "0.0", str, sizeof(str), path); 
			startPoint( atof(str) );
			GetPrivateProfileString(sec, "EndPoint", "0.0", str, sizeof(str), path); 
			endPoint( atof(str) );
			GetPrivateProfileString(sec, "AccelerationFactor", "0.0", str, sizeof(str), path); 
			accelFactor( atof(str) );
			GetPrivateProfileString(sec, "EnableCF_Rules", "false", str, sizeof(str), path); 
			enableCFR( stricmp(str, "true") == 0 );
			GetPrivateProfileString(sec, "Enforce_Min_Speed", "false", str, sizeof(str), path); 
			enforceMinSpeed( stricmp(str, "true") == 0 );
			GetPrivateProfileString(sec, "InfluenceZoneCalibrationParameter", "0.0", str, sizeof(str), path); 
			infZoneCalibParam( atof(str) );
			GetPrivateProfileString(sec, "MinimumSpeed", "20.0", str, sizeof(str), path); 
			minSpeed( atof(str) );
			GetPrivateProfileString(sec, "status", "(+)", str, sizeof(str), path); 
			if (str[strlen(str)-1] != ' ') strcat(str, " ");
			return str;
		}

		void write(const char *sec, const char *status, const char *path)
		{
			char bf[1024];
			WritePrivateProfileString(sec, "SectionID", sectionID(), path);
			sprintf(bf, "%d",junctionID()); WritePrivateProfileString(sec, "JunctionID", bf, path);
			sprintf(bf, "%d",signalGroup()); WritePrivateProfileString(sec, "SignalGroup", bf, path);
			sprintf(bf, "%f",startPoint()); WritePrivateProfileString(sec, "StartPoint", bf, path);
			sprintf(bf, "%f",endPoint()); WritePrivateProfileString(sec, "EndPoint", bf, path);
			sprintf(bf, "%f",accelFactor()); WritePrivateProfileString(sec, "AccelerationFactor", bf, path);
			WritePrivateProfileString(sec, "EnableCF_Rules", enableCFR() ? "true" : "false", path);
			sprintf(bf, "%f",infZoneCalibParam()); WritePrivateProfileString(sec, "InfluenceZoneCalibrationParameter", bf, path);
			sprintf(bf, "%f",minSpeed()); WritePrivateProfileString(sec, "MinimumSpeed", bf, path);
			WritePrivateProfileString(sec, "Enforce_Min_Speed", enforceMinSpeed() ? "true" : "false", path);
			WritePrivateProfileString(sec, "status", status, path);
		}
	};

	// use  UnorderedContainer here to avoid sorting!
//	typedef std::map<std::string, paramGroup> Groups;
	typedef UnorderedContainer<std::string, paramGroup> Groups;
	typedef Groups::iterator iterator;
	


	enum retcode
	{
		sr_ok,
		sr_empty_index_string,		// specified intersection section not found
		sr_duplicate_index,			// duplicate intersection title
		sr_empty_table,				// ni intersections defined
		sr_bad_vehicle_index,		// vehicle index is 0
		sr_bad_vehicle_entry,		// Equipped Vehicle Types section is incomplete
		sr_bad_vehicle_selection,	// selected vehicle not found in vehicle list

		sr_LAST

	};

	Settings() : m_LogLevels("Err,Msg")
	{
	}

	const char *logLevels() const { return m_LogLevels.c_str(); }
	void logLevels(const char *l) { m_LogLevels = l ? l : ""; }
	void replications(int n) { m_NumReplications = n; }
	int replications() { return m_NumReplications; }

	retcode read()
	{
		retcode ret = sr_ok; // no errors
		int n = GetPrivateProfileInt("Index", "Intersections", 0, path());
		char str[1024] = {0};
		char ib[20];

		GetPrivateProfileString("Index", "LogLevels", "", str, sizeof(str), path());
		if (str[0] != 0)
			logLevels(str);

		int repl = GetPrivateProfileInt("Index", "Replications", 0, path());
		replications(repl);

		m_paramGroups.clear();
		for (int i=0; ret == sr_ok && i< n; i++)
		{
			sprintf(ib, "%d", i+1);
			GetPrivateProfileString("Index", ib, "", str, sizeof(str), path());
			if (str[0] == 0)
				ret = sr_empty_index_string;  // empty index string
			else
			{
				bool found = false;
				for (Groups::iterator it = m_paramGroups.begin(); !found && it != m_paramGroups.end(); it++)
				{
					size_t pos = it->first.find_first_of(") ");
					if (pos != std::string::npos)
					{
						found = it->first.substr(pos+2) == str;
					}
				}
				if (!found)
				{
					paramGroup pg;
					std::string status = pg.read(str, path());
					m_paramGroups[status + str] = pg;
					fLog("READ: [%s]\n", (status+str).c_str());
				}
				else
				{
					ret = sr_duplicate_index; // duplicate
					sLogErr("SETTINGS: Duplicate index %s\n", str);
				}
			}
		}

		vehTypes().clear();
		n = GetPrivateProfileInt("Equipped Vehicle Types", "NumTypes", 0, path());
		for (int i=0; ret == sr_ok && i<n; i++)
		{
			sprintf(ib, "%d", i+1);
			GetPrivateProfileString("Equipped Vehicle Types", ib, "", str, sizeof(str), path()); 
			char *p = strchr(str, ',');
			if (p)
			{
				*p=0;
				int x = atoi(p+1);
				if (x == 0) 
					ret = sr_bad_vehicle_index;
				else
					vehTypes().add(str, atoi(p+1));
			}
			else
				ret = sr_bad_vehicle_entry;
		}

		vehTypesSel().clear();
		GetPrivateProfileString("Equipped Vehicle Types", "Selected", "", str, sizeof(str), path()); 
		char *p = strtok(str, ",");
		while (p && ret == sr_ok)
		{
			eei::VehicleTypes::iterator is = vehTypes().find(atoi(p));
			if (is != vehTypes().end())
			{
				sLog("SETTINGS: Adding selected %s - %d\n", is->first.c_str(), is->second);
				vehTypesSel().add(is->first.c_str(), is->second);
			}
			else
			{
				sLogErr("SETTINGS: *** Vehicle with type %d not found\n", atoi(p));
				ret = sr_bad_vehicle_selection;
			}
			p = strtok(0, ",");
		}

		return ret;
	}

	retcode write()
	{
		retcode r = sr_ok;  // no errors
		WritePrivateProfileString("Index", "LogLevels", logLevels(), path());
		char bf[100];
		sprintf(bf, "%d", replications()); WritePrivateProfileString("Index", "Replications", bf, path());
		size_t n = m_paramGroups.size();
		if (n)
		{
			sprintf(bf, "%d", n); WritePrivateProfileString("Index", "Intersections", bf, path());
			int i=1;
			for (Groups::iterator it = m_paramGroups.begin(); it!= m_paramGroups.end(); it++, i++)
			{
				size_t pos = it->first.find_first_of(") ");
				Assert(pos != std::string::npos);

				std::string name = it->first.substr(pos+2);
				std::string status = it->first.substr(0,pos+2);
				sprintf(bf, "%d", i); WritePrivateProfileString("Index", bf, name.c_str(), path());

				it->second.write(name.c_str(), status.c_str(), path());
			}

			size_t nveh = vehTypes().size();
			sprintf(bf, "%d", nveh); WritePrivateProfileString("Equipped Vehicle Types", "NumTypes", bf, path());
			
			bf[0] = 0;
			for (eei::VehicleTypes::iterator is = vehTypesSel().begin(); is != vehTypesSel().end(); is++)
				sprintf(bf+strlen(bf), "%d,", is->second);
			if (bf[0] != 0)
				WritePrivateProfileString("Equipped Vehicle Types", "Selected", bf, path());

			char val[255];
			i=1;
			for (eei::VehicleTypes::iterator it = vehTypes().begin(); it!= vehTypes().end(); it++, i++)
			{
				sprintf(val, "%s,%d", it->first.c_str(), it->second);  // name,type
				sprintf(bf, "%d", i);   // index number
				WritePrivateProfileString("Equipped Vehicle Types", bf, val, path());
			}

		}
		else
			r = sr_empty_table;

		return r;
	}


	const char *path() { return m_path.c_str(); }
	void path(const char *p) { m_path = p; }

	iterator begin() { return m_paramGroups.begin(); }
	iterator end() { return m_paramGroups.end(); }
	iterator find(const char *key) { return m_paramGroups.find(key); }

	Groups &params() {return m_paramGroups; }
	eei::VehicleTypes &vehTypes() {return m_vehTypes; }
	eei::VehicleTypes &vehTypesSel() {return m_vehTypesSel; }

private:
	std::string m_path;				// settingfile path
	std::string m_LogLevels;
	int m_NumReplications;			// number of replications left to run (def. 0)

	Groups m_paramGroups;			// parameter groups per intersection
	eei::VehicleTypes m_vehTypes;		// global equipped vehicles types
	eei::VehicleTypes m_vehTypesSel;	// global equipped vehicles types selected by user
};

#endif

