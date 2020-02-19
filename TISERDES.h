#pragma once


// CTISERDES dialog

class CTISERDES : public CDialogEx
{
	DECLARE_DYNAMIC(CTISERDES)

public:
	CTISERDES(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTISERDES();

// Dialog Data
	enum { IDD = IDD_DIALOG_TI_SERDES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CDialogEx *m_pMianDlg;
};
