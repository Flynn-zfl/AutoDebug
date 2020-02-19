// OV495 ISP.cpp : implementation file
//

#include "stdafx.h"
#include "AutoDebug.h"
#include "afxdialogex.h"
#include "AutoDebugDlg.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <time.h>

#include ".\\I2C\\i2c.h"

#include ".\\OV495\\ov495_spi_operation.h"
#include ".\\OV495\\LIB_SPI.h"
#include "OV495ISP.h"

t_sf g_sf;
t_sf * rom_g_sf = NULL;

//#define BOSCH_ARCH

// COV495ISP dialog
unsigned int m_ov495id=00;

#ifdef BOSCH_ARCH
unsigned char ov495_ReadSensor(unsigned char id, unsigned int addr)
{
	WriteReg16(m_ov495id, 0x3516, 0x00); //
	WriteReg16(m_ov495id, 0x354d, 0x10); //
	WriteReg16(m_ov495id, 0x354a, 0x1d); //

	if(id<2)
	{
		WriteReg16(m_ov495id, 0x0500, id+0x01); //Other Device
		WriteReg16(m_ov495id, 0x0501, (addr>>8)&0xff); //addrh
		WriteReg16(m_ov495id, 0x0502, addr&0xff); //addrl
		WriteReg16(m_ov495id, 0x0503, 0x00); //
		if(addr>0xff)WriteReg16(m_ov495id, 0x0504, 0x01); //16bit addr
		else WriteReg16(m_ov495id, 0x0504, 0x00); //8bit addr

		WriteReg16(m_ov495id, 0x3004, 0x00); //para num
		WriteReg16(m_ov495id, 0x3005, 0x05);
		WriteReg16(m_ov495id, 0x31a6, 0x00); //not do crc
		WriteReg16(m_ov495id, 0x31a7, 0x00);

		WriteReg16(m_ov495id, 0x30c0, 0xc1); //
	}	
	else
	{
		WriteReg16(m_ov495id, 0x0500, id+0x01); //Other Device

		if(addr>0xff)WriteReg16(m_ov495id, 0x0501, 0x00); //16bit addr
		else WriteReg16(m_ov495id, 0x0501, 0x10); //8bit addr

		WriteReg16(m_ov495id, 0x0502, (addr>>8)&0xff); //addrh
		WriteReg16(m_ov495id, 0x0503, addr&0xff); //addrl
		WriteReg16(m_ov495id, 0x0504, 0x00); //		

		WriteReg16(m_ov495id, 0x3004, 0x00); //para num
		WriteReg16(m_ov495id, 0x3005, 0x05);
		WriteReg16(m_ov495id, 0x31a6, 0x00); //not do crc
		WriteReg16(m_ov495id, 0x31a7, 0x00);

		WriteReg16(m_ov495id, 0x30c0, 0x3e); //	
	}

	int i=10;
	while(i){
		if(ReadReg16(m_ov495id, 0x0ffc) == 0x99) break;
		Sleep(10);
		i--;
	}

	return ReadReg16(m_ov495id, 0x0500);
}

unsigned char ov495_WriteSensor(unsigned char id, unsigned int addr, unsigned char pdata)
{

	WriteReg16(m_ov495id, 0x3516, 0x00); //
	WriteReg16(m_ov495id, 0x354d, 0x10); //
	WriteReg16(m_ov495id, 0x354a, 0x1d); //

	if(id<2)
	{
		WriteReg16(m_ov495id, 0x0500, id+0x00); //Other Device

		WriteReg16(m_ov495id, 0x0501, (addr>>8)&0xff); //addrh
		WriteReg16(m_ov495id, 0x0502, addr&0xff); //addrl
		WriteReg16(m_ov495id, 0x0503, pdata&0xff); //
	
		if(addr>0xff)WriteReg16(m_ov495id, 0x0504, 0x01); //16bit addr
		else WriteReg16(m_ov495id, 0x0504, 0x00); //8bit addr

		WriteReg16(m_ov495id, 0x3004, 0x00); //para num
		WriteReg16(m_ov495id, 0x3005, 0x05); 
		WriteReg16(m_ov495id, 0x31a6, 0x00); //not do CRC
		WriteReg16(m_ov495id, 0x31a7, 0x00);

		WriteReg16(m_ov495id, 0x30c0, 0xc1); //
	}
	else
	{
		WriteReg16(m_ov495id, 0x0500, id+0x00); //Other Device

		if(addr>0xff)WriteReg16(m_ov495id, 0x0501, 0x00); //16bit addr
		else WriteReg16(m_ov495id, 0x0501, 0x10); //8bit addr

		WriteReg16(m_ov495id, 0x0502, (addr>>8)&0xff); //addrh
		WriteReg16(m_ov495id, 0x0503, addr&0xff); //addrl
		WriteReg16(m_ov495id, 0x0504, pdata&0xff); //		

		WriteReg16(m_ov495id, 0x3004, 0x00); //para num
		WriteReg16(m_ov495id, 0x3005, 0x05);
		WriteReg16(m_ov495id, 0x31a6, 0x00); //not do crc
		WriteReg16(m_ov495id, 0x31a7, 0x00);

		WriteReg16(m_ov495id, 0x30c0, 0x3e); //	
	}

	int i=10;
	while(i){
		if(ReadReg16(m_ov495id, 0x0ffc) == 0x99) break;
		Sleep(10);
		i--;
	}

	return 1;
}

#else
unsigned char ov495_ReadSensor(unsigned char id, unsigned int addr)
{
	WriteReg16(m_ov495id, 0x3516, 0x00); //
	WriteReg16(m_ov495id, 0x354d, 0x10); //
	WriteReg16(m_ov495id, 0x354a, 0x1d); //


	WriteReg16(m_ov495id, 0x0500, id+0x01); //Other Device
	WriteReg16(m_ov495id, 0x0501, (addr>>8)&0xff); //addrh
	WriteReg16(m_ov495id, 0x0502, addr&0xff); //addrl
	WriteReg16(m_ov495id, 0x0503, 0x00); //
	if(addr>0xff)WriteReg16(m_ov495id, 0x0504, 0x01); //16bit addr
	else WriteReg16(m_ov495id, 0x0504, 0x00); //8bit addr

	WriteReg16(m_ov495id, 0x3004, 0x00); //para num
	WriteReg16(m_ov495id, 0x3005, 0x05);
	WriteReg16(m_ov495id, 0x31a6, 0x00); //not do crc
	WriteReg16(m_ov495id, 0x31a7, 0x00);

	WriteReg16(m_ov495id, 0x30c0, 0xc1); //


	int i=10;
	while(i){
		if(ReadReg16(m_ov495id, 0x0ffc) == 0x99) break;
		Sleep(10);
		i--;
	}

	return ReadReg16(m_ov495id, 0x0500);
}

unsigned char ov495_WriteSensor(unsigned char id, unsigned int addr, unsigned char pdata)
{

	WriteReg16(m_ov495id, 0x3516, 0x00); //
	WriteReg16(m_ov495id, 0x354d, 0x10); //
	WriteReg16(m_ov495id, 0x354a, 0x1d); //

	WriteReg16(m_ov495id, 0x0500, id+0x00); //Other Device

	WriteReg16(m_ov495id, 0x0501, (addr>>8)&0xff); //addrh
	WriteReg16(m_ov495id, 0x0502, addr&0xff); //addrl
	WriteReg16(m_ov495id, 0x0503, pdata&0xff); //
	
	if(addr>0xff)WriteReg16(m_ov495id, 0x0504, 0x01); //16bit addr
	else WriteReg16(m_ov495id, 0x0504, 0x00); //8bit addr

	WriteReg16(m_ov495id, 0x3004, 0x00); //para num
	WriteReg16(m_ov495id, 0x3005, 0x05); 
	WriteReg16(m_ov495id, 0x31a6, 0x00); //not do CRC
	WriteReg16(m_ov495id, 0x31a7, 0x00);

	WriteReg16(m_ov495id, 0x30c0, 0xc1); //


	int i=10;
	while(i){
		if(ReadReg16(m_ov495id, 0x0ffc) == 0x99) break;
		Sleep(10);
		i--;
	}

	return 1;
}

#endif

IMPLEMENT_DYNAMIC(COV495ISP, CDialogEx)

	COV495ISP::COV495ISP(CWnd* pParent /*=NULL*/)
	: CDialogEx(COV495ISP::IDD, pParent)
{

}

COV495ISP::~COV495ISP()
{
}

void COV495ISP::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ID, m_ISP_ID);
	DDX_Control(pDX, IDC_BUTTON_ERASEFLASH, m_Eraseflash);
	DDX_Control(pDX, IDC_BUTTON_READFLASH, m_Readflash);
	DDX_Control(pDX, IDC_BUTTON_BURNFLASH, m_Burnflash);
	DDX_Control(pDX, IDC_BUTTON_SPIINIT, m_Spiinit);
	DDX_Control(pDX, IDC_CHECK_VERIFY, m_check_verify);
	DDX_Control(pDX, IDC_STATIC_ID, m_IDs);

	DDX_Control(pDX, IDC_EDIT_SVAL, m_SVal);
	DDX_Control(pDX, IDC_EDIT_SADDR, m_SAddr);
	DDX_Control(pDX, IDC_EDIT_SID, m_SId);
	DDX_Control(pDX, IDC_BUTTON_READSNR, m_ReadSNR);
	DDX_Control(pDX, IDC_BUTTON_WRITESNR, m_WriteSNR);
	DDX_Text(pDX, IDC_EDIT_SID, s_SId);
	DDX_Text(pDX, IDC_EDIT_SADDR, s_SAddr);
	DDX_Text(pDX, IDC_EDIT_SVAL, s_SVal);
}


BEGIN_MESSAGE_MAP(COV495ISP, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_IDCHECK, &COV495ISP::OnBnClickedButtonIdcheck)
	ON_BN_CLICKED(IDC_BUTTON_SETID, &COV495ISP::OnBnClickedButtonSetid)
	ON_BN_CLICKED(IDC_BUTTON_SPIINIT, &COV495ISP::OnBnClickedButtonSpiinit)
	ON_BN_CLICKED(IDC_BUTTON_ERASEFLASH, &COV495ISP::OnBnClickedButtonEraseflash)
	ON_BN_CLICKED(IDC_BUTTON_READFLASH, &COV495ISP::OnBnClickedButtonReadflash)
	ON_BN_CLICKED(IDC_BUTTON_BURNFLASH, &COV495ISP::OnBnClickedButtonBurnflash)
	ON_BN_CLICKED(IDC_BUTTON_READSNR, &COV495ISP::OnBnClickedButtonReadsnr)
	ON_BN_CLICKED(IDC_BUTTON_WRITESNR, &COV495ISP::OnBnClickedButtonWritesnr)
	ON_BN_CLICKED(IDC_BUTTON_ISP0, &COV495ISP::OnBnClickedButtonIsp0)
	ON_BN_CLICKED(IDC_BUTTON_ISP1, &COV495ISP::OnBnClickedButtonIsp1)
END_MESSAGE_MAP()


// COV495ISP message handlers
BOOL COV495ISP::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_Spiinit.EnableWindow(FALSE);
	m_Eraseflash.EnableWindow(FALSE);
	m_Readflash.EnableWindow(FALSE);
	m_Burnflash.EnableWindow(FALSE);

	m_SVal.EnableWindow(FALSE);
	m_SAddr.EnableWindow(FALSE);
	m_SId.EnableWindow(FALSE);
	m_ReadSNR.EnableWindow(FALSE);
	m_WriteSNR.EnableWindow(FALSE);

	m_ISP_ID.SetWindowTextA("48");

	return TRUE;
}

void COV495ISP::set_dlg(CDialog * parnet_dlg)
{
	m_pMianDlg = parnet_dlg;
}

void COV495ISP::ServiceMessages()
{
	MSG message;
	while (PeekMessage((LPMSG)&message,NULL,NULL,NULL,PM_REMOVE))
	{
		TranslateMessage((LPMSG)&message);
		DispatchMessage((LPMSG)&message);
	}
}

void COV495ISP::erase_sf(void)
{
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	CString Strtxt;

	parent->m_Status.SetWindowTextA("Erase Flash ...");
	ov495_spi_erase();
	//Delay some seconds after sent CHIP_ERASE command to flash
	//delay is depended on flash type,normal case,we delay 8 seconds(512K BYTES flash)
	int time = 0;	
	while(( ov495_RD_SPI_STATUS()&(unsigned char)BIT0) == (unsigned char)BIT0){
		Sleep(1000);
		time++;
		Strtxt.Format("Erasing... %d", time);
		parent->m_Status.SetWindowTextA(Strtxt);
		if(time>20) 
			parent->Log("Erase Flash failed!\r\n");
			parent->m_Status.SetWindowTextA("Erase Flash failed!\r\n");
			return;
	}


	//for(int i=0;i<8000;i++)
	//{
	//	Sleep(100);
	//	i = i + 100;
	//	Strtxt.Format("Erasing... %d%%", i * 100 / 8000);
	//	parent->m_Status.SetWindowTextA(Strtxt);
	//}	

	CString msg;
	msg.Format("Erase time=%dsec\r\n", time);
	parent->Log(msg);
	parent->m_Status.SetWindowTextA("Erase Flash done!");
}

void COV495ISP::burn_sf(void)
{
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	CString Strtxt;
	/////////////////////////////////////////////////////////
	CFileDialog dlg(TRUE,".bin",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"ISP firmware(*.bin)|*.bin; *.bin|all bin files (*.*)|*.*||");

	if(dlg.DoModal()!=IDOK)    return;

	FILE *fp;
	if((fopen_s(&fp,dlg.GetPathName(),"rb+")) != 0)
	{
		parent->m_Status.SetWindowTextA("Open firmwre file wrong");
		return;
	}

	time_t	st_t, end_t;
	double	write_t;
	st_t=time(0);

	fseek(fp,0L,SEEK_END);
	long file_len = ftell(fp);  
	fseek(fp, 0, SEEK_SET);

	BYTE* buf = new BYTE [file_len];
	memset(buf, 0, file_len);
	fread(buf, 1, file_len, fp);
	m_nFileLength = file_len;
	fclose(fp);    
	/////////////////////////////////////////////////////////////
	unsigned int n_sfaddr = 0, llp = 0;
	int nFileLenTemp = m_nFileLength;
	unsigned char* tmp = buf;
	m_pFWBuf = buf;
	
	parent->m_Status.SetWindowTextA("Write Flash ...");


	while(nFileLenTemp > 0)
	{
		int len = g_sf.sector_size - (n_sfaddr & (g_sf.sector_size - 1));
		len &= ~(g_sf.page_size - 1);
		
		if(len > nFileLenTemp) 
			len = nFileLenTemp;

		for(int i = 0; i< len; i += g_sf.page_size)
		{
			unsigned int dest_flash_addr = n_sfaddr & ~(g_sf.page_size - 1);
			Sleep(5);

			ov495_spi_page_write(tmp, dest_flash_addr);
			Sleep(5);

			n_sfaddr += g_sf.page_size;
			tmp += g_sf.page_size;
			llp+=g_sf.page_size;
			
			Strtxt.Format("Burning... %d%%", (m_nFileLength - nFileLenTemp+i)*100/m_nFileLength);
			parent->m_Status.SetWindowTextA(Strtxt);	
		}
		nFileLenTemp -= len;
	}
	parent->m_Status.SetWindowTextA("Download Flash done!");

	end_t	=	time(0);
	write_t	=	difftime(end_t, st_t);
	CString msg;
	msg.Format("Burn time=%.1fsec\r\n", write_t);
	parent->Log(msg);
}

void COV495ISP::verify_sf()
{
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	CString Strtxt;
	unsigned int sf_addr = 0, buffer_count = 0;
	unsigned char buf[MAX_READ_SIZE] = {0};

	time_t	st_t, end_t;
	double	verify_t;
	st_t=time(0);
	
	parent->m_Status.SetWindowTextA("Verify Flash ...this step might be spent more time.");	
	bool bVerifyEnd = false;
	while(1)
	{
		Sleep(100);
			
		ov495_spi_page_read(sf_addr, buf);
		//compare
		for(int j=0;j<MAX_READ_SIZE;j++)
		{
			if(*(m_pFWBuf + buffer_count) != buf[j])
			{			

				Strtxt.Format("Verify Failed. Address(%d) 0x%x != 0x%x", j, *(m_pFWBuf + buffer_count), buf[j]);
				parent->m_Status.SetWindowTextA(Strtxt);
				bVerifyEnd = true;
				break;
			}
			else
			{
				buffer_count++;
				if(buffer_count == m_nFileLength)
				{
					parent->m_Status.SetWindowTextA("Verify OK.");
					bVerifyEnd = true;
					break;
				}
				Strtxt.Format("Verifying... %d%%", sf_addr * 100 / m_nFileLength);
				parent->m_Status.SetWindowTextA(Strtxt);
			}
		}
		if(bVerifyEnd)
			break;
		sf_addr += MAX_READ_SIZE;		
	}

	end_t	=	time(0);
	verify_t	=	difftime(end_t, st_t);
	CString msg;
	msg.Format("verify time=%.1fsec\r\n", verify_t);
	parent->Log(msg);
}

void COV495ISP::OnBnClickedButtonIdcheck()
{
	// TODO: Add your control notification handler code here
	unsigned char id[4]={0x48,0x46,0x4a,0x4c};	
	unsigned int id_tmp=0;
	CString strtxt;
	
	for(int i = 0; i<4; i++)
	{
		if((ReadReg16(id[i], 0x300b)&0xff) == 0x95)
			id_tmp = id_tmp*256 + id[i];
			//id_tmp = id[i];
	}	

	strtxt.Format("%x %x %x %x",id_tmp&0xff, (id_tmp>>8)&0xff, (id_tmp>>16)&0xff, (id_tmp>>24)&0xff);
	m_IDs.SetWindowTextA(strtxt);

}

void COV495ISP::OnBnClickedButtonSetid()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	CString strtxt;

	unsigned char mId;
	unsigned int version;

	m_ISP_ID.GetWindowTextA(strtxt);
	sscanf_s(strtxt,_T("%x"),&mId);

	WriteReg16(mId, 0x3516, 0x00); //unlock
	version=(ReadReg16(mId, 0x3001)<<8)&0xff00;
	version+=(ReadReg16(mId, 0x3000))&0xff;

	if(version==0x4951)
	{
		m_ov495id=mId&0xff; 
		m_Spiinit.EnableWindow(TRUE);

		m_SId.EnableWindow(TRUE);
		m_SVal.EnableWindow(TRUE);
		m_SAddr.EnableWindow(TRUE);
		m_ReadSNR.EnableWindow(TRUE);
		m_WriteSNR.EnableWindow(TRUE);


		strtxt.Format("ov491/493/495 is existed", version);
		parent->m_Status.SetWindowTextA(strtxt);
		strtxt.Format("BootFlag=0x%02x\r\n", ReadReg16(m_ov495id, 0x31a4));
		parent->Log(strtxt);
		strtxt.Format("FirmwareVer=%d(0x%x)\r\n", ReadReg16(m_ov495id, 0x31a2)<<8|ReadReg16(m_ov495id, 0x31a3), ReadReg16(m_ov495id, 0x31a2)<<8|ReadReg16(m_ov495id, 0x31a3));
		parent->Log(strtxt);
		strtxt.Format("SettingVer=%d(0x%x)\r\n", ReadReg16(m_ov495id, 0x418a)<<8|ReadReg16(m_ov495id, 0x418b), ReadReg16(m_ov495id, 0x418a)<<8|ReadReg16(m_ov495id, 0x418b));
		parent->Log(strtxt);
	}
	else parent->m_Status.SetWindowTextA("ov491/493/495 is not existed!");	
}

void COV495ISP::OnBnClickedButtonSpiinit()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	ov495_spi_init();
	parent->m_Status.SetWindowTextA("ov491/493/495 Burn Tool init ok!");
	m_Eraseflash.EnableWindow(TRUE);
	//m_Readflash.EnableWindow(TRUE);
	m_Burnflash.EnableWindow(TRUE);
}


void COV495ISP::OnBnClickedButtonEraseflash()
{
	// TODO: Add your control notification handler code here
	m_Eraseflash.EnableWindow(FALSE);
	AfxBeginThread(WorkerThread_Erase, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
	m_Eraseflash.EnableWindow(FALSE);
}


void COV495ISP::OnBnClickedButtonReadflash()
{
	// TODO: Add your control notification handler code here

}


void COV495ISP::OnBnClickedButtonBurnflash()
{
	// TODO: Add your control notification handler code here
	m_Burnflash.EnableWindow(FALSE);
	AfxBeginThread(WorkerThread_Burn, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
	m_Burnflash.EnableWindow(TRUE);
}

UINT COV495ISP::WorkerThread_Erase( LPVOID pParam)
{
	COV495ISP* pObject = (COV495ISP*)pParam;

    if (pObject == NULL)
		return 1;   // if pObject is not valid	
	pObject->erase_sf();

    return 0;   // thread completed successfully
}

UINT COV495ISP::WorkerThread_Burn( LPVOID pParam)
{
	COV495ISP* pObject = (COV495ISP*)pParam;

    if (pObject == NULL)
		return 1;   // if pObject is not valid	

	pObject->burn_sf();
	if(pObject->m_check_verify.GetCheck())
	{
		pObject->verify_sf();
	}
    return 0;   // thread completed successfully
}


void COV495ISP::OnBnClickedButtonReadsnr()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	CString strtxt;
	int SCCB_ID=0, RegData=0, RegAddr=0;
	m_SId.GetWindowTextA(s_SId);
	sscanf_s(s_SId,_T("%x"),&SCCB_ID);
	m_SAddr.GetWindowTextA(s_SAddr);
	sscanf_s(s_SAddr,_T("%x"),&RegAddr);
	RegData = ov495_ReadSensor(SCCB_ID, RegAddr);
	s_SVal.Format(_T("%x"),RegData);
	m_SVal.SetWindowTextA(s_SVal);
	strtxt.Format(_T("%02x %04x %02x;<hc r>\r\n"), SCCB_ID, RegAddr, RegData);
	parent->Log(strtxt);
}


void COV495ISP::OnBnClickedButtonWritesnr()
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	CString strtxt;
	int SCCB_ID=0, RegData=0, RegAddr=0;
	m_SId.GetWindowTextA(s_SId);
	sscanf_s(s_SId,_T("%x"),&SCCB_ID);
	m_SAddr.GetWindowTextA(s_SAddr);
	sscanf_s(s_SAddr,_T("%x"),&RegAddr);
	m_SVal.GetWindowTextA(s_SVal);
	sscanf_s(s_SVal,_T("%x"),&RegData);
	ov495_WriteSensor(SCCB_ID, RegAddr, RegData);
	strtxt.Format(_T("%02x %04x %02x;<hc w>\r\n"),SCCB_ID, RegAddr, RegData);	
	parent->Log(strtxt);
}


void COV495ISP::OnBnClickedButtonIsp0()
{
	// TODO: Add your control notification handler code here
	WriteReg16(m_ov495id, 0x3516, 0x00); //unlock
	WriteReg16(m_ov495id, 0x354a, 0x1d);
	WriteReg16(m_ov495id, 0x354d, 0x10);

	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	parent->Log("Switch to ISP0\r\n");
}


void COV495ISP::OnBnClickedButtonIsp1()
{
	// TODO: Add your control notification handler code here
	WriteReg16(m_ov495id, 0x3516, 0x00); //unlock
	WriteReg16(m_ov495id, 0x354a, 0x1e);
	WriteReg16(m_ov495id, 0x354d, 0x00);
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	parent->Log("Switch to ISP1\r\n");
}
