//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1991-2000 Microsoft Corporation
//
// @doc
//
// @module WIZARD.CPP
//
//-----------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////
#include "common.h"
#include "tablecopy.h"
#include "wizard.h"
#include "progress.h"



/////////////////////////////////////////////////////////////////////
// CDialogBase::CDialogBase
//
/////////////////////////////////////////////////////////////////////
CDialogBase::CDialogBase(HWND hWnd, HINSTANCE hInst)
{
	ASSERT(hInst);
	
	m_hWnd	= hWnd;
	m_hInst = hInst;
}

/////////////////////////////////////////////////////////////////////
// CDialogBase::~CDialogBase
//
/////////////////////////////////////////////////////////////////////
CDialogBase::~CDialogBase()
{
	Destroy();
}


/////////////////////////////////////////////////////////////////////
// ULONG CDialogBase::Destroy
//
/////////////////////////////////////////////////////////////////////
ULONG CDialogBase::Destroy()
{
	if(m_hWnd)
	{
		EndDialog(m_hWnd, 0);
		m_hWnd = NULL;
	}

	return 0;
}


////////////////////////////////////////////////////////////////
// CWizard::CWizard
//
/////////////////////////////////////////////////////////////////
CWizard::CWizard(HWND hWnd, HINSTANCE hInst)
	: CDialogBase(hWnd, hInst)
{
	m_pCTableCopy = new CTableCopy(this);
	m_pCProgress = new CProgress(hWnd, hInst);

	m_iPrevStep = WIZ_STEP1;
	m_rgDialogSteps[WIZ_STEP1] = new CS1Dialog(hWnd, hInst, m_pCTableCopy);
	m_rgDialogSteps[WIZ_STEP2] = new CS2Dialog(hWnd, hInst, m_pCTableCopy);
	m_rgDialogSteps[WIZ_STEP3] = new CS3Dialog(hWnd, hInst, m_pCTableCopy);
	m_rgDialogSteps[WIZ_STEP4] = new CS4Dialog(hWnd, hInst, m_pCTableCopy);
	m_rgDialogSteps[WIZ_TYPES] = new CTypesDialog(hWnd, hInst, m_pCTableCopy);
}


////////////////////////////////////////////////////////////////
// CWizard::~CWizard
//
/////////////////////////////////////////////////////////////////
CWizard::~CWizard()
{
	delete m_pCTableCopy;
	delete m_pCProgress;

	delete m_rgDialogSteps[WIZ_STEP1];
	delete m_rgDialogSteps[WIZ_STEP2];
	delete m_rgDialogSteps[WIZ_STEP3];
	delete m_rgDialogSteps[WIZ_STEP4];
	delete m_rgDialogSteps[WIZ_TYPES];
}


////////////////////////////////////////////////////////////////
// CWizard::Display
//
/////////////////////////////////////////////////////////////////
INT_PTR CWizard::Display()
{
	return DisplayStep(WIZ_STEP1);
}


////////////////////////////////////////////////////////////////
// ULONG CWizard::DisplayStep
//
/////////////////////////////////////////////////////////////////
INT_PTR CWizard::DisplayStep(ULONG iStep)
{
	ASSERT(iStep >= WIZ_STEP1 && iStep < END_WIZ);
	return m_rgDialogSteps[iStep]->Display();
}


////////////////////////////////////////////////////////////////
// ULONG CWizard::DestroyPrevStep
//
/////////////////////////////////////////////////////////////////
ULONG CWizard::DestroyPrevStep(ULONG iCurStep)
{
	if(iCurStep != m_iPrevStep)
		m_rgDialogSteps[m_iPrevStep]->Destroy();
	m_iPrevStep = iCurStep;
	return 0;
}
