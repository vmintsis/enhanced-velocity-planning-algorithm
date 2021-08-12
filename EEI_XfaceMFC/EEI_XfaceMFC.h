// EEI_XfaceMFC.h : main header file for the EEI_XfaceMFC DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CEEI_XfaceMFCApp
// See EEI_XfaceMFC.cpp for the implementation of this class
//

class CEEI_XfaceMFCApp : public CWinApp
{
public:
	CEEI_XfaceMFCApp();
	~CEEI_XfaceMFCApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	DECLARE_MESSAGE_MAP()
};
