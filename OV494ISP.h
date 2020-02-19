#pragma once
#include "afxwin.h"


// COV494ISP dialog

class COV494ISP : public CDialogEx
{
	DECLARE_DYNAMIC(COV494ISP)

public:
	COV494ISP(CWnd* pParent = NULL);   // standard constructor
	virtual ~COV494ISP();
	virtual BOOL OnInitDialog();

	// Dialog Data
	enum { IDD = IDD_DIALOG_OV494 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	
	void SendBootloader(void);
	void ChipErase(void);
	void Flash_Burn(void);
	UINT static WorkerThread_FlashBurn( LPVOID pParam);
	UINT static WorkerThread_Erase(LPVOID);
	UINT static WorkerThread_BootLoader(LPVOID);

public:
	CDialog *m_pMianDlg;
	void set_dlg(CDialog * parnet_dlg);
	void ServiceMessages();
	afx_msg void OnBnClickedButtonReadflash();
	afx_msg void OnBnClickedButtonBurnflash();
	afx_msg void OnBnClickedButtonCheckid();
	afx_msg void OnBnClickedButtonSetid();
	CEdit m_ISP_ID;
	
	afx_msg BOOL WriteDataToDevice(unsigned char nID, unsigned char *pBuf, unsigned int data_start, unsigned int data_size, unsigned char mode);
	afx_msg void ReadDataFromDevice(unsigned char nID, unsigned char *pBuf, unsigned int data_start, unsigned int data_size, unsigned char mode);
	CButton m_ReadFlash;
	CButton m_BurnFlash;
	CStatic m_IDs;
	afx_msg void OnBnClickedButtonReadsnr();
	afx_msg void OnBnClickedButtonWritesnr();
	CEdit m_SId;
	CEdit m_SAddr;
	CEdit m_SVal;
	CButton m_ReadSNR;
	CButton m_WriteSNR;
	CString s_SId;
	CString s_SAddr;
	CString s_SVal;
	
	CButton m_Spiinit;	
	CButton m_check_verify;
	CButton m_Eraseflash;
	CButton m_SectorErase;
	afx_msg void OnBnClickedButtonSpiinit();
	afx_msg void OnBnClickedButtonEraseflash();
	afx_msg void OnBnClickedCheckSectorerase();
};
