#pragma once


// CMAXIMSERDES dialog

class CMAXIMSERDES : public CDialogEx
{
	DECLARE_DYNAMIC(CMAXIMSERDES)

public:
	CMAXIMSERDES(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMAXIMSERDES();

// Dialog Data
	enum { IDD = IDD_DIALOG_MAXIM_SERDES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CDialogEx *m_pMianDlg;
};
