#pragma once
#include "afxwin.h"


// COV495ISP dialog

class COV495ISP : public CDialogEx
{
	DECLARE_DYNAMIC(COV495ISP)

public:
	COV495ISP(CWnd* pParent = NULL);   // standard constructor
	virtual ~COV495ISP();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_OV495 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	void erase_sf(void);
	void burn_sf(void);
	void verify_sf();
	UINT static WorkerThread_Erase(LPVOID);
	UINT static WorkerThread_Burn(LPVOID);

public:
	CDialog *m_pMianDlg;
	void set_dlg(CDialog * parnet_dlg);
	void ServiceMessages();

	CEdit m_ISP_ID;
	afx_msg void OnBnClickedButtonIdcheck();
	afx_msg void OnBnClickedButtonSetid();
	afx_msg void OnBnClickedButtonSpiinit();
	afx_msg void OnBnClickedButtonEraseflash();
	afx_msg void OnBnClickedButtonReadflash();
	afx_msg void OnBnClickedButtonBurnflash();
	CButton m_Eraseflash;
	CButton m_Readflash;
	CButton m_Burnflash;
	CButton m_Spiinit;

	unsigned int FileLength;	
	unsigned char * fw_buf;
	unsigned long boot_bin_file_len;
	unsigned int BOOTFLAG_ADDR;
	unsigned short CHIP_VER;
	FILE * burnfile;
	unsigned int m_nFileLength;
	unsigned char * m_pFWBuf;

	
	CButton m_check_verify;
	CStatic m_IDs;

	CEdit m_SVal;
	CEdit m_SAddr;
	CEdit m_SId;
	CButton m_ReadSNR;
	CButton m_WriteSNR;
	CString s_SId;
	CString s_SAddr;
	CString s_SVal;

	afx_msg void OnBnClickedButtonReadsnr();
	afx_msg void OnBnClickedButtonWritesnr();
	afx_msg void OnBnClickedButtonIsp0();
	afx_msg void OnBnClickedButtonIsp1();
};
