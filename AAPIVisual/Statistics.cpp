#include "Precompiled.h"

#include "EEIObject.h"
#include "Statistics.h"

namespace eei
{

//----------------------------------------------
//
//----------------------------------------------
Statistics::Statistics(EEIObject *parent)
	: m_pParent(parent), m_enabled(false)
{
}

//----------------------------------------------
//
//----------------------------------------------
void Statistics::OnEnteredZone(int vehType, double veh_speed)
{
	if (!m_enabled) return;
	VehStat &stat = m_map[vehType];
//	stat.avgSpeed = veh_speed + (stat.lastSpeed - stat.avgSpeed)/(stat.vehInZone+1);

	// Wikipedia, Cumulative Moving Average: https://en.wikipedia.org/?title=Moving_average
	stat.avgSpeed = stat.avgSpeed + (veh_speed - stat.avgSpeed)/(stat.vehInZone+1);
	stat.vehInZone++;
}

void Statistics::OnAdvised(int vehType, ADV_REASON reason, double veh_speed)
{
	if (!m_enabled) return;
	VehStat &stat = m_map[vehType];
	if (reason & AR_ACCELERATE) 
		stat.accelAdv++;
	else
		stat.declAdv++;

	stat.speedAdv++;
}


void Statistics::OnUnadvised(int vehType)
{
	if (!m_enabled) return;
	m_map[vehType].numUntracked++;
}


std::string Statistics::toHTML()
{
//	return "NO HTML GENERATED";

	EEIObject *par = parent();
	if (!par->enabled())
	{
		return "Intersection processing is disabled";
	}

	std::string js = 
"	// set inner html to element with specified id\n"
"	function at(oname, text)\n"
"	{\n"
"		document.getElementById(oname).innerHTML = text;\n"
"	}\n"

"	// create statistics table header \n"
"	function stat_hdr(oname, a_hdr)\n"
"	{\n"
"		var i, t_hdr='<td></td><td class=\"tbl_spc\"></td>';\n"
"		for (i=0; i<a_hdr.length; i++)\n"
"			t_hdr += '<td class=\"stat_col\">' + a_hdr[i] + \"</td>\";\n"
"		at(oname, t_hdr);\n"
"	}\n"
"\n"
"	// create one row in the statistics table\n"
"	function stat_line(oname, lbl_name, a_data)\n"
"	{\n"
"		var tbl = document.getElementById(oname);\n"
"		var row = tbl.tBodies[0].insertRow(-1);\n"
"\n"
"		row.innerHTML = '<td class=\"tbl_lbl\">' + lbl_name + '</td><td class=\"tbl_spc\"></td>';\n"
"		for (i=0; i<a_data.length; i++)\n"
"			row.innerHTML += '<td class=\"tbl_lbl\">' + a_data[i] + \"</td>\";\n"
"	}\n"
"\n"
"	// install a window onload event handler to set up all report elements\n"
"	window.onload = function()\n"
"	{\n"
"\n";

		js += 
"		at('SCENARIO_NAME', '" + par->getScenarioName() + "');\n"
"		at('INTERSECTION_NAME', '" + par->intersectionName() + "');\n"
"		at('INTERSECTION_ID', '" + toStr(par->getJunctionID()) + "');\n"
"		at('ROAD_NAME', '" + par->roadName() + "');\n"
"		at('ROAD_ID', '" + par->getSectionIDstr() + "');\n"
"		at('ZONE_START', '" + toStr(par->getRangeToJunction().first) + "');\n"
"		at('ZONE_END', '" + toStr(par->getRangeToJunction().second) + "');\n"
"		at('ACCEL_FACTOR', '"+ toStr(par->accelFactor()) + "');\n"
"		at('INFL_ZONE_CALIB_PARAM', '"+ toStr(par->infZoneCalibParam()) +"');\n"
"		at('MIN_SPEED', '" + toStr(par->minSpeed()) + "');\n"
"		at('CF_RULE', '" + (par->enableCFR() ? "Enabled" : "Disabled") + "');\n"
"		at('CF_MINSPEED', '" + (par->enforceMinSpeed() ? "Enabled" : "Disabled") + "');\n"
"		at('CYCLE_DURATION', '" + toStr(par->getCycleDuration()) + "');\n"
"		at('GREEN_TIME_THRU', '"+ toStr(par->getGreenTime()) +"');\n"
"		at('GREEN_TIME_PERCENT', '" + toStr(100.0 *(par->getGreenTime() / par->getCycleDuration())) + "');\n";

		js +=
"		stat_hdr('TR_VEH_HDR', [";
		EEIObject::VehicleVector vt = par->vehicles("'");
		for (size_t i=0; i<vt.size(); i++)
		{
			js += vt[i].first;
			if (i<vt.size()-1) js += ",";
		}
		js += "] );\n";

		js += "		stat_line('STAT_TBL', 'Vehicles within Zone:', [";
		for (size_t i=0; i< vt.size(); i++)
		{
			VehStat vs = m_map[vt[i].second];
			js += "'" + toStr(vs.vehInZone) + "'";
			if (i < vt.size() -1 ) js += ",";
		}
		js += "]);\n";

		js += "		stat_line('STAT_TBL', 'Speed Advice:', [";
		for (size_t i=0; i< vt.size(); i++)
		{
			VehStat vs = m_map[vt[i].second];
			js += "'" + toStr(vs.speedAdv) + "'";
			if (i < vt.size() -1 ) js += ",";
		}
		js += "]);\n";

		js += "		stat_line('STAT_TBL', 'Acceleration Advice:', [";
		for (size_t i=0; i< vt.size(); i++)
		{
			VehStat vs = m_map[vt[i].second];
			js += "'" + toStr(vs.accelAdv) + "'";
			if (i < vt.size() -1 ) js += ",";
		}
		js += "]);\n";

		js += "		stat_line('STAT_TBL', 'Deceleration Advice:', [";
		for (size_t i=0; i< vt.size(); i++)
		{
			VehStat vs = m_map[vt[i].second];
			js += "'" + toStr(vs.declAdv) + "'";
			if (i < vt.size() -1 ) js += ",";
		}
		js += "]);\n";

		js += "		stat_line('STAT_TBL', 'Average Speed:', [";
		for (size_t i=0; i< vt.size(); i++)
		{
			VehStat vs = m_map[vt[i].second];
			js += "'" + toStr(vs.avgSpeed) + "'";
			if (i < vt.size() -1 ) js += ",";
		}
		js += "]);\n";

		js += "		stat_line('STAT_TBL', 'Unadvised due to Car-Following:', [";
		for (size_t i=0; i< vt.size(); i++)
		{
			VehStat vs = m_map[vt[i].second];
			js += "'" + toStr(vs.numUntracked) + "'";
			if (i < vt.size() -1 ) js += ",";
		}
		js += "]);\n";


		js += 
"\n"
"	}\n";

	std::string html = populate_MFC_HTMLresource(js);
	return html;
}

//=========================================
//  MFC-specific function that obtains the HTML report resource of this module
//  and inserts into it the javascript passed.
//
std::string Statistics::populate_MFC_HTMLresource(std::string &js)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	std::string html="*** NO HTML ***\n";

	HRSRC hRes = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_EEI_REP2), "EEI_REP");
	if (hRes)
	{
		HGLOBAL hMem = LoadResource(AfxGetResourceHandle(), hRes);
		if (hMem != NULL)
		{
			DWORD sz = SizeofResource(AfxGetResourceHandle(), hRes);
			char * pRes = (char *)LockResource(hMem);
			if (pRes)
			{
				pflogMsg("Size of HTML: %lu bytes\n", sz);
				std::vector<char> buff(sz+10);
				memcpy(&buff[0], pRes, sz);
				memcpy(&buff[sz], "\n\n\0", 3); 
				html = (char *)&buff[0];
				UnlockResource(hMem);

				size_t pos = html.find("//REPORT_JS");
				if (pos != std::string::npos)
				{
					html.insert(pos+11, "\n\n"+js);
				}
			}
			else
				pflogErr("*** Cannot Lock HTML Resource\n");
		}
		else
			pflogErr("*** Cannot Load HTML Resource\n");
	}
	else
		pflogErr("*** Report HTML Resource not found!\n");
	return html;
}


} // namespace

