#pragma once


// CX1FSOC dialog

class CX1FSOC : public CDialogEx
{
	DECLARE_DYNAMIC(CX1FSOC)

public:
	CX1FSOC(CWnd* pParent = NULL);   // standard constructor
	virtual ~CX1FSOC();
	virtual BOOL OnInitDialog();
// Dialog Data
	enum { IDD = IDD_DIALOG_X1F };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	void init_sf();
	void burn_sf();
	void ChipErase();
	void BlockErase();
	void SectorErase();
	UINT static WorkerThread_Erase(LPVOID);
	UINT static WorkerThread_Burn(LPVOID);
	UINT static WorkerThread_BootLoader(LPVOID);

	void ReadDataFromDevice(unsigned char nID, unsigned char *pBuf, unsigned int data_start, unsigned int data_size, unsigned char mode);
	int WriteDataToDevice(unsigned char nID, unsigned char *pBuf, unsigned int data_start, unsigned int data_size, unsigned char mode);

public:
	CDialog *m_pMianDlg;
	void set_dlg(CDialog * parnet_dlg);
	void ServiceMessages();

	afx_msg void OnBnClickedButtonIdcheck();
	afx_msg void OnBnClickedButtonSetid();
	afx_msg void OnBnClickedButtonSpiinit();
	afx_msg void OnBnClickedButtonEraseflash();
	afx_msg void OnBnClickedButtonReadflash();
	afx_msg void OnBnClickedButtonBurnflash();
	afx_msg void OnBnClickedCheckSectorerase();
	afx_msg void OnBnClickedCheckVerify();

	CButton m_Eraseflash;
	CButton m_Readflash;
	CButton m_Burnflash;
	CButton m_Spiinit;	
	CButton m_check_verify;
	CStatic m_IDs;	
	CEdit m_ISP_ID;
	CButton m_SectorErase;


};
