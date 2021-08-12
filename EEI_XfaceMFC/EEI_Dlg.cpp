// EEI_Dlg.cpp : implementation file
//

#include "Precompiled.h"
#include "EEI_XfaceMFC.h"
#include "EEI_Dlg.h"



// EEI_Dlg dialog

IMPLEMENT_DYNAMIC(EEI_Dlg, CDialog)

EEI_Dlg::EEI_Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(EEI_Dlg::IDD, pParent)
	, m_JunctionID(0)
	, m_SignalGroup(0)
	, m_StartPoint(0)
	, m_EndPoint(0)
	, m_filename(_T(""))
	, m_accelFactor(0)
	, m_enableCFR(FALSE)
	, m_sectionID(_T(""))
	, m_infZoneCalibParam(0)
	, m_XSectionName(_T(""))
	, m_minSpeed(0)
	, m_logAll(FALSE)
	, m_logErr(TRUE)
	, m_logMsg(TRUE)
	, m_EnforceMinSpeed(FALSE)
	, m_NumReplications(0)
{
	m_newMode = false;
	m_timer = 0;
}

EEI_Dlg::~EEI_Dlg()
{
}


// TODO: Replace MFC annoying DDX/DDV with custom DDX/DDV mechanism that is more user-friendly
//		 The previous timer-based workaround has the side-effect of not saving if the user clicks on another intersection or OK/Cancel 
//		 before the timer expires.
void EEI_Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, m_JunctionID);
	DDV_MinMaxInt(pDX, m_JunctionID, 1, 100000);
	DDX_Text(pDX, IDC_EDIT3, m_SignalGroup);
	DDV_MinMaxInt(pDX, m_SignalGroup, 1, 40);
	DDX_Text(pDX, IDC_EDIT4, m_StartPoint);
	DDV_MinMaxDouble(pDX, m_StartPoint, 10, 1000);
	DDX_Text(pDX, IDC_EDIT5, m_EndPoint);
	DDV_MinMaxDouble(pDX, m_EndPoint, 10, 1000);
	DDX_Control(pDX, IDC_EDIT7, m_filenameCtl);
	DDX_Text(pDX, IDC_EDIT7, m_filename);
	DDX_Text(pDX, IDC_EDIT6, m_accelFactor);
	DDX_Check(pDX, IDC_CHECK1, m_enableCFR);
	DDX_Text(pDX, IDC_EDIT1, m_sectionID);
	DDX_Text(pDX, IDC_EDIT8, m_infZoneCalibParam);
	DDV_MinMaxDouble(pDX, m_infZoneCalibParam, 0.01, 100.0);
	DDX_Text(pDX, IDC_EDIT9, m_XSectionName);
	DDX_Control(pDX, IDC_LIST1, m_XSectionLB);
	DDV_MaxChars(pDX, m_XSectionName, 50);
	DDX_Text(pDX, IDC_EDIT10, m_minSpeed);
	DDV_MinMaxDouble(pDX, m_minSpeed, 0.0, 250);
	DDX_Control(pDX, IDC_NEW_IND, m_newIndicator);
	DDX_Control(pDX, IDC_LIST2, m_VehicleTypes);
	DDX_Check(pDX, IDC_DEBUG_LOG, m_logAll);
	DDX_Check(pDX, IDC_DEBUG_LOG2, m_logErr);
	DDX_Check(pDX, IDC_DEBUG_LOG3, m_logMsg);
	DDX_Check(pDX, IDC_CHECK2, m_EnforceMinSpeed);
	DDX_Text(pDX, IDC_REPLICATIONS, m_NumReplications);
}


BEGIN_MESSAGE_MAP(EEI_Dlg, CDialog)
	ON_BN_CLICKED(IDOK, &EEI_Dlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &EEI_Dlg::OnBnClickedBrowse)
	ON_BN_CLICKED(IDC_ADD_BTN, &EEI_Dlg::OnBnClickedAddBtn)
	ON_BN_CLICKED(IDC_REMOVE_BTN, &EEI_Dlg::OnBnClickedRemoveBtn)
	ON_LBN_SELCHANGE(IDC_LIST1, &EEI_Dlg::OnLbnSelchangeList1)
	ON_BN_CLICKED(IDC_NEW_BTN, &EEI_Dlg::OnBnClickedNewBtn)
	ON_WM_CTLCOLOR()
	ON_EN_CHANGE(IDC_EDIT2, &EEI_Dlg::OnEnChangeEdit2)
	ON_EN_CHANGE(IDC_EDIT1, &EEI_Dlg::OnEnChangeEdit1)
	ON_EN_CHANGE(IDC_EDIT3, &EEI_Dlg::OnEnChangeEdit3)
	ON_EN_CHANGE(IDC_EDIT4, &EEI_Dlg::OnEnChangeEdit4)
	ON_EN_CHANGE(IDC_EDIT5, &EEI_Dlg::OnEnChangeEdit5)
	ON_EN_CHANGE(IDC_EDIT6, &EEI_Dlg::OnEnChangeEdit6)
	ON_EN_CHANGE(IDC_EDIT8, &EEI_Dlg::OnEnChangeEdit8)
	ON_EN_CHANGE(IDC_EDIT10, &EEI_Dlg::OnEnChangeEdit10)
	ON_BN_CLICKED(IDC_CHECK1, &EEI_Dlg::OnBnClickedCheck1)
	ON_WM_TIMER()
	ON_LBN_SELCHANGE(IDC_LIST2, &EEI_Dlg::OnLbnSelchangeVehicles)
	ON_BN_CLICKED(IDC_BUTTON_USE, &EEI_Dlg::OnBnClickedButtonUse)
	ON_BN_CLICKED(IDC_CHECK2, &EEI_Dlg::OnBnClickedCheck2)
END_MESSAGE_MAP()




// NEW' button clears the gui on the right.
// When in 'NEW' mode, we don't update the map.
// We exit 'NEW' mode when we click in the list to select an entry, or when we click 'ADD'
// During 'NORMAL' mode all notification handlers update the current selection in the map with the user-modified data
BOOL EEI_Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_redBR = CreateSolidBrush(RGB(0,0,255));
//	m_fontCourier.CreatePointFont(80, "Courier New");
//	m_XSectionLB.SetFont(&m_fontCourier);

	updateVehTypesLB();
	updateXSectionLB();
	UpdateData(FALSE);
	if (m_XSectionLB.GetCount() > 0)
	{
		m_XSectionLB.SetCurSel(0);
		OnLbnSelchangeList1();
	}

	return TRUE;
}

// EEI_Dlg message handlers

void EEI_Dlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	// TODO: Add your control notification handler code here
	OnOK();
}

// Callable even when dialog is not created yet.
// It will setup the data, but not update the listbox.
// The listbox will be updated automatically *if* data have been setup at OnInitDialog time
bool EEI_Dlg::SetupParameters(const char *path, const eei::VehicleTypes &vt)
{
	REQUIRE( path != 0);

	bool ret = true;
	
	vehTypes() = vt;
	m_filename = path;

	Settings st;
	st.path(m_filename);

	Settings::retcode r = st.read();
	if (r == Settings::sr_ok )
	{
		if (st.vehTypes() == vehTypes())
		{
			vehTypesSel() = st.vehTypesSel();
		}
		else
		{
			char *pMsg = "***Warning: Vehicle types of this file do not match current map. Using all vehicles.\n";
			sLogErr(pMsg);
			fLogErr(pMsg);
		}

		setLogLevels(st.logLevels());
		setReplications( st.replications() );

		EEI_GUI gui;
		for (Settings::iterator it = st.begin(); it != st.end(); it++)
		{
			gui << it->second;
			addXSection(it->first.c_str(), gui);
		}
	}
	else
	{
		sLogErr("***Error %d setting up parameters\n", r);
		fLogErr("***Error %d setting up parameters\n", r);
		ret = false;
	}

	return ret;
}

void EEI_Dlg::setReplications(int n)
{
	m_NumReplications = n;
}

int EEI_Dlg::getReplications()
{
	return m_NumReplications;
}


void EEI_Dlg::setLogLevels(const char *logLevels)
{
	if (logLevels)
	{
		char bf[400];
		strcpy(bf, logLevels);
		char *p = strtok(bf, ", ");
		m_logAll = m_logErr = m_logMsg = FALSE;
		while (p)
		{
			if (!strcmp(p, "All"))
				m_logAll = TRUE;
			else
			if (!strcmp(p, "Err"))
				m_logErr = TRUE;
			else
			if (!strcmp(p, "Msg"))
				m_logMsg = TRUE;

			p = strtok(0, ", ");
		}
	}
}

std::string EEI_Dlg::getLogLevels()
{
	std::string r = "";
	if (m_logAll) r += "All,";
	if (m_logErr) r += "Err,";
	if (m_logMsg) r += "Msg,";

	return r;
}


void EEI_Dlg::OnBnClickedBrowse()
{
	// TODO: Add your control notification handler code here
	CString path;
	UpdateData(TRUE);
	if (!m_filename.IsEmpty())
		path = m_filename;
	CFileDialog dlg(TRUE, "eei", path, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "EEI Files (*.eei)|*.eei|All Files (*.*)|*.*||");
	if (dlg.DoModal() == IDOK)
	{
		if (!dlg.GetPathName().IsEmpty())
		{
			if (strcmp(path, dlg.GetPathName()) )
			{
				m_XSectionLB.ResetContent();
				m_guiMap.clear();
			}

			SetupParameters(dlg.GetPathName(), vehTypes());

			if (m_XSectionLB.GetCount() > 0)
			{
				m_XSectionLB.SetCurSel(0);
				OnLbnSelchangeList1();
			}
			UpdateData(FALSE);
		}
	}

}

void EEI_Dlg::updateXSectionLB()
{
	if ((HWND)m_XSectionLB)
	{
		m_XSectionLB.ResetContent();
		for (guiMap::iterator it = m_guiMap.begin(); it != m_guiMap.end(); it++)
			m_XSectionLB.AddString(it->first.c_str());
	}
}

void EEI_Dlg::updateVehTypesLB()
{
	if ((HWND)m_VehicleTypes)
	{
		m_VehicleTypes.ResetContent();
		for (eei::VehicleTypes::iterator it = m_vehTypes.begin(); it != m_vehTypes.end(); it++)
		{
			if (m_VehicleTypes.GetCount() < it->second)
			{
				int r = m_VehicleTypes.AddString(it->first.c_str() );
				m_VehicleTypes.SetItemData(r, (DWORD_PTR)it->second);
			}
			else
			{
				int r = m_VehicleTypes.InsertString( it->second-1, it->first.c_str() );
				m_VehicleTypes.SetItemData(r, (DWORD_PTR)it->second);
			}
		}

		if (vehTypesSel().size() > 0)
		{
			for (eei::VehicleTypes::iterator it = vehTypesSel().begin(); it != vehTypesSel().end(); it++)
			{
				int n = m_VehicleTypes.FindString(-1, it->first.c_str());
				if (n >=0)
					m_VehicleTypes.SetSel(n, TRUE);
			}
		}
	}
}

EEI_Dlg &EEI_Dlg::addXSection(const char *key, EEI_GUI &x)
{
	REQUIRE(key != 0);

	bool found = false;
	for (guiMap::iterator it = m_guiMap.begin(); !found && it != m_guiMap.end(); it++)
	{
		found = it->second.junctionID() == x.junctionID() && it->second.sectionID() == x.sectionID();
	}

	if (!found && find(key) == end())
	{
		add(key, x);
		if ((HWND)m_XSectionLB)
			m_XSectionLB.AddString(key);
	}
	return *this;
}

EEI_Dlg &EEI_Dlg::removeXSection(const char *key)
{
	REQUIRE(key != 0);

	guiMap::iterator it = find(key);
	if (it != m_guiMap.end())
	{
		if ((HWND)m_XSectionLB)
		{
			int n = m_XSectionLB.FindString(0L, key);
			if (n >=0)
				m_XSectionLB.DeleteString(n);
		}
		m_guiMap.erase(it);
	}
	return *this;
}

EEI_GUI EEI_Dlg::toStruct()
{
	UpdateData(TRUE);

	EEI_GUI x;
	x.junctionID(m_JunctionID);
	x.signalGroup(m_SignalGroup); 
	x.startPoint(m_StartPoint);
	x.endPoint(m_EndPoint);
	x.accelFactor(m_accelFactor);
	x.enableCFR( m_enableCFR == TRUE);
	x.sectionID( m_sectionID );
	x.infZoneCalibParam(m_infZoneCalibParam);
	x.minSpeed(m_minSpeed);
	x.enforceMinSpeed(m_EnforceMinSpeed == TRUE);

	CString cs;
	for (int i=0; i<m_VehicleTypes.GetCount(); i++)
	{
		if (m_VehicleTypes.GetSel(i))
			x.m_vehTypesSelected.push_back(i);
	}
	return x;
}

void EEI_Dlg::fromStruct(EEI_GUI &x)
{
	m_JunctionID = x.junctionID();
	m_SignalGroup = x.signalGroup(); 
	m_StartPoint = x.startPoint();
	m_EndPoint = x.endPoint();
	m_accelFactor = x.accelFactor();
	m_enableCFR = x.enableCFR();
	m_sectionID = x.sectionID();
	m_infZoneCalibParam = x.infZoneCalibParam();
	m_minSpeed = x.minSpeed();
	m_EnforceMinSpeed = x.enforceMinSpeed();

	for (size_t i=0; i < x.m_vehTypesSelected.size(); i++)
	{
		if (x.m_vehTypesSelected[i] < m_VehicleTypes.GetCount())
			m_VehicleTypes.SetSel(x.m_vehTypesSelected[i]);
	}

	UpdateData(FALSE);
}

void EEI_Dlg::newMode(bool b)
{
	m_newMode = b;
	Invalidate();
}

HBRUSH EEI_Dlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if (pWnd->GetDlgCtrlID() == IDC_NEW_IND)
    {
        if (m_newMode)
			hbr = m_redBR;
		else
			hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
    }
    return hbr;
}

void EEI_Dlg::OnBnClickedAddBtn()
{
	CancelTimer();
	EEI_GUI data = toStruct();
	if (data.isValid())
	{
		char bf[500];
		std::vector<int> ids = section_str(data.sectionID()).toIDs();
		sprintf(bf, "(+) EEI %d - %d", data.m_JunctionID, ids[ids.size()-1]);
		if (!m_XSectionName.IsEmpty())
		{
			strcat(bf, ": ");
			strcat(bf, m_XSectionName);
		}
		addXSection(bf, data);
		newMode(false);
	}
	else
		MessageBox("Please enter correct data for this intersection", "Attention", MB_OK);
}

void EEI_Dlg::OnBnClickedRemoveBtn()
{
	CancelTimer();
	int n = m_XSectionLB.GetCurSel();
	if (n >=0)
	{
		CString str;
		m_XSectionLB.GetText(n, str);
		removeXSection(str);
	}
}

void EEI_Dlg::OnLbnSelchangeList1()
{
	CancelTimer();

	int n = m_XSectionLB.GetCurSel();
	if (n>=0)
	{
		CString str;
		m_XSectionLB.GetText(n, str);
		int n = str.Find(':');
		if (n >= 0) 
		{
			n += 2;
			m_XSectionName = (const char *)str+n;
		}
		else
			m_XSectionName = "";
		guiMap::iterator it = find((const char *)str);
		if (it != m_guiMap.end())
		{
			fromStruct(it->second);
			newMode(false);
		}
	}
}

void EEI_Dlg::OnBnClickedNewBtn()
{
	if (!m_newMode)
	{
		CancelTimer();
		newMode(true);
		EEI_GUI x;
		fromStruct(x);
	}
}

void EEI_Dlg::OnTimer(UINT_PTR nIDEvent)
{
	updateMap(true);
}

void EEI_Dlg::updateMap(bool fromTimer/*=false*/)
{
//	if (!fromTimer)
//	{
//		CancelTimer();
//		m_timer = SetTimer(1, 2000, 0);
//		sLog("Timer ON\n");
//	}
//	else
	{
		CancelTimer();
//		sLog("Timer OFF\n");
		if (!newMode() && (HWND)m_XSectionLB)
		{
			int n = m_XSectionLB.GetCurSel();
			if (n >=0)
			{
				EEI_GUI data = toStruct();
				CString str;
				m_XSectionLB.GetText(n, str);

				Assert( find(str) != end() );

				add((const char *)str, data);
			}
		}
	}
}


void EEI_Dlg::OnEnChangeEdit2()
{
	updateMap();
}

void EEI_Dlg::OnEnChangeEdit1()
{
	updateMap();
}

void EEI_Dlg::OnEnChangeEdit3()
{
	updateMap();
}

void EEI_Dlg::OnEnChangeEdit4()
{
	updateMap();
}

void EEI_Dlg::OnEnChangeEdit5()
{
	updateMap();
}

void EEI_Dlg::OnEnChangeEdit6()
{
	updateMap();
}

void EEI_Dlg::OnEnChangeEdit8()
{
	updateMap();
}

void EEI_Dlg::OnEnChangeEdit10()
{
	updateMap();
}

void EEI_Dlg::OnBnClickedCheck1()  // enableCFR
{
	updateMap();
}

void EEI_Dlg::OnBnClickedCheck2()  // enforce_min_speed
{
	updateMap();
}

void EEI_Dlg::OnLbnSelchangeVehicles()
{
	char buff[300];
//	eei::VehicleTypes vehTypesSel;
	vehTypesSel().clear();
	for (int i=0; i<m_VehicleTypes.GetCount(); i++)
	{
		if(m_VehicleTypes.GetSel(i))
		{
			m_VehicleTypes.GetText(i, buff);
			vehTypesSel().add(buff, (int)m_VehicleTypes.GetItemData(i));
		}
	}
//	for (guiMap::iterator i = m_guiMap.begin(); i!= m_guiMap.end(); i++)
//		i->second. = vehTypesSel;
}


// To avoid std::map ordering of the list after +/- changes, 
// Section::key has been made into a standalone templated class that can also 
// use a string 'id' and is used as guiMap key here too.
void EEI_Dlg::OnBnClickedButtonUse()
{
	for (int i=0; i <m_XSectionLB.GetCount(); i++)
	{
		if (m_XSectionLB.GetSel(i))
		{
			CString s;
			m_XSectionLB.GetText(i, s);
			m_XSectionLB.DeleteString(i);
			guiMap::iterator it = find((const char *)s);
			Assert(it != m_guiMap.end());
			EEI_GUI x = it->second;
//			m_guiMap.erase(it);
			if (s.Left(3) == "(+)")
				s.Replace("(+)", "(--)");
			else
				s.Replace("(--)", "(+)");
//			add((const char *)s, x);
			it->first = (const char *)s;
			m_XSectionLB.InsertString(i, s);
			m_XSectionLB.SetSel(i);
		}
	}
}

