#ifndef LIB_BULK_API_H
#define LIB_BULK_API_H

//#define FPGA_INOUT  // FPGA translate Raw10/12 to RAW16 format to output

#define CMD_OVIF_DRIVER_INIT			0x100
#define CMD_OVIF_DRIVER_QUIT			0x101
#define CMD_OVIF_PREVIEW_ON				0x102
#define CMD_OVIF_PREVIEW_OFF			0x103
#define CMD_OVIF_SET_OVLP_NUM			0x200
#define CMD_OVIF_SET_FRM_NUM			0x201
#define CMD_OVIF_SET_PACKETSIZE			0x202
#define CMD_OVIF_SET_SIDEBYSIDE			0x203
#define CMD_OVIF_GET_DEV_COUNT			0x204
#define CMD_OVIF_GET_FRAMERATE			0x301



/*
* This is message control function, which allows you to init\quit driver, preview on\off etc;
*/
extern "C" int OVIF_Command(int cmd, LPARAM pi, LPARAM po);

/*
* This is to set Callback function, which allows you access\process the original video data;
*/
typedef void (OVImgProcCallBack)(unsigned char* buf, int length);
extern "C" int SetCBFunc(OVImgProcCallBack pFunc);

/*
* This function is to set Video format in your application. Current firmware only support the following 
* features: 
* Two resolutions: 2560*800, 1280*400;
* Four formats: YUV, RAW8, RAW12, RAW16;
* Two frame rate: 60fps, 30fps; 
*/
extern "C" int QuickSet(int width, int height, int format, int framerate);

/*
* This is to capture usb data;
*/
extern "C"  int AcquireUSBdata(unsigned char *pBuffer, int* pBufferSize);
///////////////////////////////////////////////////////////////////////////////////////////////////////////
////						 I2C access mothod                                              ////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
* I2C access mothod;
*/
//extern "C" int ReadRegister(int SCCBID ,int addr, int& dat, bool b16bitAddress=0);
//extern "C" int WriteRegister(int SCCBID , int addr, int dat, bool b16bitAddress=0);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" int ReadRegister_Gaia(int SCCBID ,int addr, int& dat, bool b16bitAddress=0); 
extern "C" int WriteRegister_Gaia(int SCCBID , int addr, int dat, bool b16bitAddress=0); 
extern "C"  int ReadRegister_GaiaBurst(unsigned char SCCBID, unsigned int addr, unsigned char* value, unsigned char b16bitAddress, unsigned char num); 
extern "C"  int WriteRegister_GaiaBurst(unsigned char SCCBID , unsigned int addr, unsigned char* value, unsigned char b16bitAddress, unsigned char num);

class PatherSDK
{
	int OVIF_Command(int cmd, LPARAM pi, LPARAM po);
	int SetCBFunc(OVImgProcCallBack pFunc);
	int QuickSet(int width, int height, int format, int framerate);
	int AcquireUSBdata(unsigned char *pBuffer, int* pBufferSize);
	//int ReadRegister(int SCCBID ,int addr, int& dat, bool b16bitAddress=0);
	//int WriteRegister(int SCCBID , int addr, int dat, bool b16bitAddress=0);
	int ReadRegister_Gaia(int SCCBID ,int addr, int& dat, bool b16bitAddress=0); 
	int WriteRegister_Gaia(int SCCBID , int addr, int dat, bool b16bitAddress=0); 
	int ReadRegister_GaiaBurst(unsigned char SCCBID, unsigned int addr, unsigned char* value, unsigned char b16bitAddress, unsigned char num); 
	int WriteRegister_GaiaBurst(unsigned char SCCBID , unsigned int addr, unsigned char* value, unsigned char b16bitAddress, unsigned char num);

};
#endif //LIB_BULK_API_H
