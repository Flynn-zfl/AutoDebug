#pragma once
#include "afxwin.h"
//#include "resource.h"

// COV491ISP dialog

class COV491ISP : public CDialogEx
{
	DECLARE_DYNAMIC(COV491ISP)

public:
	COV491ISP(CWnd* pParent = NULL);   // standard constructor
	virtual ~COV491ISP();	
	virtual BOOL OnInitDialog();
// Dialog Data
	enum { IDD = IDD_DIALOG_OV491 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	

	DECLARE_MESSAGE_MAP()

	bool SCCB_Download_Bootloader(BYTE* buf, DWORD size);
	bool DownloadMainFW(BYTE* buf, DWORD size);
	bool DownloadBootloader();
	void OV49X_Reboot();
	void burn_sf();
	void ChipErase();
	void BlockErase();
	void SectorErase();
	UINT static WorkerThread_Erase(LPVOID);
	UINT static WorkerThread_Burn(LPVOID);
	UINT static WorkerThread_BootLoader(LPVOID);

public:
	CDialogEx *m_pMianDlg;
	
	afx_msg void OnBnClickedButtonIdcheck();
	afx_msg void OnBnClickedButtonSetid();
	afx_msg void OnBnClickedButtonIsp0();
	afx_msg void OnBnClickedButtonIsp1();
	afx_msg void OnBnClickedButtonReadsnr();
	afx_msg void OnBnClickedButtonWritesnr();
	afx_msg void OnBnClickedButtonSpiinit();
	afx_msg void OnBnClickedButtonEraseflash();
	afx_msg void OnBnClickedButtonBurnflash();	

	CButton m_Eraseflash;
	CButton m_Readflash;
	CButton m_Burnflash;
	CButton m_Spiinit;	
	CButton m_check_verify;
	CStatic m_IDs;	

	CEdit m_ISP_ID;
	CEdit m_SVal;
	CEdit m_SAddr;
	CEdit m_SId;
	CButton m_ReadSNR;
	CButton m_WriteSNR;
	CString s_SId;
	CString s_SAddr;
	CString s_SVal;
	afx_msg void OnBnClickedCheckSectorerase();
	CButton m_SectorErase;
};
