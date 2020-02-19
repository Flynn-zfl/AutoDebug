#pragma once
#include "afxwin.h"


// COV480ISP dialog

class COV480ISP : public CDialogEx
{
	DECLARE_DYNAMIC(COV480ISP)

public:
	COV480ISP(CWnd* pParent = NULL);   // standard constructor
	virtual ~COV480ISP();

// Dialog Data
	enum { IDD = IDD_DIALOG_OV480 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()	
	void Chiperase(void);
	void Spiburn(void);
	void Spiread(void);
	UINT static WorkerThread_Chiperase( LPVOID pParam);
	UINT static WorkerThread_Spiburn( LPVOID pParam);
	UINT static WorkerThread_Spiread( LPVOID pParam);

public:
	CDialog *m_pMianDlg;

	afx_msg void OnOK();
	afx_msg void OnClose();
	void set_dlg(CDialog * parnet_dlg);
	void ServiceMessages();


	afx_msg void OnBnClickedButtonIdcheck();
	CStatic m_IDs;
	afx_msg void OnBnClickedButtonEraseflash();

	//CButton m_Spiinit;
	CButton m_Eraseflash;
	CButton m_Readflash;
	CButton m_Burnflash;
	CButton m_check_verify;

	afx_msg void OnBnClickedButtonReadflash();
	afx_msg void OnBnClickedButtonBurnflash();
	afx_msg void OnBnClickedCheckVerify();

	CEdit m_SVal;
	CEdit m_SAddr;
	CEdit m_SId;
	CButton m_ReadSNR;
	CButton m_WriteSNR;
	afx_msg void OnBnClickedButtonReadsnr();
	afx_msg void OnBnClickedButtonWritesnr();

	CString s_SId;
	CString s_SAddr;
	CString s_SVal;
};
