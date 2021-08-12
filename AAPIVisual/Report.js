
// set inner html to element with specified id
function at(oname, text)
{
	document.getElementById(oname).innerHTML = text;
}

// create statistics table header
function stat_hdr(oname, a_hdr)
{
	var i, t_hdr='<td></td><td class="tbl_spc"></td>';
	for (i=0; i<a_hdr.length; i++)
		t_hdr += '<td class="stat_col">' + a_hdr[i] + "</td>";
	at(oname, t_hdr);
}

// create one row in the statistics table
function stat_line(oname, lbl_name, a_data)
{
	var tbl = document.getElementById(oname);
	var row = tbl.tBodies[0].insertRow(-1);
	
	row.innerHTML = '<td class="tbl_lbl">' + lbl_name + '</td><td class="tbl_spc"></td>';
	for (i=0; i<a_data.length; i++)
		row.innerHTML += '<td class="tbl_lbl">' + a_data[i] + "</td>";
}

// install a window onload event handler to set up all report elements
window.onload = function()
{
	// todo: start of generated code 
	at("SCENARIO_NAME", "Test Scenario");
	at("INTERSECTION_NAME", "Pl. Chanth");
	at("INTERSECTION_ID", "1112");
	at("ROAD_NAME", "Tsimiski St.");
	at("ROAD_ID", "525,635,248");
	
	at("ZONE_START", "360");
	at("ZONE_END", "340");
	
	at("ACCEL_FACTOR", "0.15");
	at("INFL_ZONE_CALIB_PARAM", "0.05");
	at("MIN_SPEED", "20");
	at("CF_RULE", "Enabled");
	
	at("CYCLE_DURATION", "120");
	at("GREEN_TIME_THRU", "45");
	at("GREEN_TIME_PERCENT", "37.5");
	
	stat_hdr("TR_VEH_HDR", ['Car', 'Bus', 'Taxi', 'Total'] );
	stat_line("STAT_TBL", "Vehicles within Zone:", ["250", "34", "105", "678"]);
	stat_line("STAT_TBL", "Speed Advice:", ["150", "34", "105", "678"]);
	stat_line("STAT_TBL", "Acceleration Advice:", ["250", "34", "105", "678"]);
	stat_line("STAT_TBL", "Deceleration Advice:", ["250", "34", "105", "678"]);
	stat_line("STAT_TBL", "Average Speed:", ["250", "34", "105", "678"]);
	stat_line("STAT_TBL", "Car-Following/Untracked:", ["250", "34", "105", "678"]);
}

