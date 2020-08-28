#include "StdAfx.h"
#include "SoftKey.h"
#include "UKey.h"
#include "KeyDef.h"

#define PID 0X8762
#define VID 0X3689
#define PID_NEW 0X2020
#define VID_NEW 0X3689
#define PID_NEW_2 0X2020
#define VID_NEW_2 0X2020

HDEVINFO  ( _stdcall *l_SetupDiGetClassDevsA)(CONST GUID *,   PCSTR , HWND , IN DWORD  );
BOOL   ( _stdcall *l_SetupDiEnumDeviceInterfaces)(HDEVINFO ,PSP_DEVINFO_DATA ,CONST GUID   *,DWORD,PSP_DEVICE_INTERFACE_DATA);
BOOL  ( _stdcall *l_SetupDiGetDeviceInterfaceDetailA)(HDEVINFO ,PSP_DEVICE_INTERFACE_DATA,PSP_DEVICE_INTERFACE_DETAIL_DATA_A ,    DWORD,PDWORD ,PSP_DEVINFO_DATA  );
BOOL  ( _stdcall *l_SetupDiDestroyDeviceInfoList)(HDEVINFO DeviceInfoSet);

void (_stdcall *l_HidD_GetHidGuid) (LPGUID   HidGuid);
BOOLEAN (_stdcall *l_HidD_SetFeature )(HANDLE   HidDeviceObject,PVOID    ReportBuffer,ULONG    ReportBufferLength);
BOOLEAN (_stdcall *l_HidD_GetFeature )(HANDLE   HidDeviceObject,OUT   PVOID    ReportBuffer,ULONG    ReportBufferLength);
BOOLEAN (_stdcall *l_HidD_GetPreparsedData )(HANDLE  HidDeviceObject,OUT   PHIDP_PREPARSED_DATA  * PreparsedData);
BOOLEAN (_stdcall *l_HidD_GetAttributes) (IN  HANDLE HidDeviceObject,OUT PHIDD_ATTRIBUTES    Attributes);
BOOLEAN (__stdcall *l_HidD_FreePreparsedData )(PHIDP_PREPARSED_DATA PreparsedData);
NTSTATUS (__stdcall *l_HidP_GetCaps) (IN  PHIDP_PREPARSED_DATA  PreparsedData,OUT PHIDP_CAPS Capabilities);

SoftKey::SoftKey()
{
    IsLoad=LoadLib();
}

SoftKey::~SoftKey()
{
    if(hSetApiLib) FreeLibrary(hSetApiLib);
    if(hHidLib) FreeLibrary(hHidLib);
}

int SoftKey::Hanldetransfe(HANDLE hUsbDevice,BYTE *array_in,int InLen,BYTE *array_out,int OutLen,char *InPath)
{
    BOOL IsU=FALSE;
    if(lstrlenA(InPath)>1)
    {
        if(InPath[5]==':')IsU=TRUE;
    }
    if(IsU)
    {
        //注意，这里inlen要加1
        if(!Ukey_transfer(hUsbDevice,array_in,InLen+1,array_out,OutLen)){
            CloseHandle(hUsbDevice);return -93;
        }
    }
    else
    {
        if(InLen>0)if(!SetFeature(hUsbDevice,array_in,InLen)){CloseHandle(hUsbDevice);return -93;}
        if(OutLen>0)if(!GetFeature(hUsbDevice,array_out,OutLen)){CloseHandle(hUsbDevice);return -94;}
    }
    CloseHandle(hUsbDevice);
    return 0;
}

HANDLE SoftKey::MyOpenPath(char *InPath)
{
    BOOL biao;int count=0;
    if(strlen(InPath)<1)
    {
        char OutPath[260];
        biao=isfindmydevice(0,&count,OutPath);
        if(!biao){return INVALID_HANDLE_VALUE;}
        return  CreateFileA(OutPath,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    }
    else
    {
        return  CreateFileA(InPath,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    }
}


BOOL  SoftKey::LoadLib()
{
    char SetupApiPath[MAX_PATH*2]="";char HidPath[MAX_PATH*2]="";

    GetSystemDirectoryA(SetupApiPath,MAX_PATH);lstrcatA(SetupApiPath,"\\SetupApi.dll");

    hSetApiLib=LoadLibraryExA(SetupApiPath,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
    if(hSetApiLib!=NULL)
    {
         l_SetupDiGetClassDevsA=(HDEVINFO ( __stdcall *)(CONST GUID *,   PCSTR , HWND , IN DWORD  ))
                                    GetProcAddress(hSetApiLib,"SetupDiGetClassDevsA");
         l_SetupDiEnumDeviceInterfaces=(BOOL   ( __stdcall *)(HDEVINFO ,PSP_DEVINFO_DATA ,CONST GUID   *,DWORD,PSP_DEVICE_INTERFACE_DATA))
                            GetProcAddress(hSetApiLib,"SetupDiEnumDeviceInterfaces");
         l_SetupDiGetDeviceInterfaceDetailA=(BOOL  ( __stdcall *)(HDEVINFO ,PSP_DEVICE_INTERFACE_DATA,PSP_DEVICE_INTERFACE_DETAIL_DATA_A ,    DWORD,PDWORD ,PSP_DEVINFO_DATA  ))
                            GetProcAddress(hSetApiLib,"SetupDiGetDeviceInterfaceDetailA");
         l_SetupDiDestroyDeviceInfoList=(BOOL  ( __stdcall *)(HDEVINFO DeviceInfoSet))
                        GetProcAddress(hSetApiLib,"SetupDiDestroyDeviceInfoList");

    }
    else
    {
        MessageBox(NULL, L"错误", L"加载SetupApi动态库时错误。", MB_OK|MB_ICONERROR);
        return FALSE;
    }

    GetSystemDirectoryA(HidPath,MAX_PATH);lstrcatA(HidPath,"\\hid.dll");
    hHidLib=LoadLibraryExA(HidPath,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
    if(hHidLib!=NULL)
    {
        l_HidD_GetHidGuid=(void (__stdcall *) ( LPGUID  ))
                GetProcAddress(hHidLib,"HidD_GetHidGuid");
        l_HidD_SetFeature=(BOOLEAN (__stdcall *)(HANDLE ,PVOID ,ULONG))
                    GetProcAddress(hHidLib,"HidD_SetFeature");
        l_HidD_GetFeature=(BOOLEAN (__stdcall *)( HANDLE ,PVOID  ,ULONG ))
                    GetProcAddress(hHidLib,"HidD_GetFeature");
        l_HidD_GetPreparsedData=(BOOLEAN (__stdcall *)(HANDLE  ,PHIDP_PREPARSED_DATA  * ))
                    GetProcAddress(hHidLib,"HidD_GetPreparsedData");
        l_HidD_GetAttributes=(BOOLEAN (__stdcall *)(HANDLE  ,PHIDD_ATTRIBUTES  ))
                    GetProcAddress(hHidLib,"HidD_GetAttributes");
        l_HidD_FreePreparsedData=(BOOLEAN (__stdcall *)(PHIDP_PREPARSED_DATA ))
                GetProcAddress(hHidLib,"HidD_FreePreparsedData");
        l_HidP_GetCaps=(NTSTATUS (__stdcall *)(PHIDP_PREPARSED_DATA  ,PHIDP_CAPS  ))
                        GetProcAddress(hHidLib,"HidP_GetCaps");
    }
    else
    {
        MessageBox(NULL, L"错误?", L"加载hid动态库时错误。", MB_OK|MB_ICONERROR);
        return FALSE;
    }
    return TRUE;
}

BOOL SoftKey::Ukey_transfer(HANDLE hDevice,LPVOID lpInBuffer, int inLen,	LPVOID lpOutBuffer,	int OutLen)
{
    BYTE *dataBuffer=NULL;
    SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
    ULONG length = 0,returned = 0;
    BOOL status = 0;
    HGLOBAL hglb_1;
    if(inLen>1)
    {

        ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
        sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
        //	sptdwb.sptd.PathId = 0;
        sptdwb.sptd.TargetId = 1;
        //	sptdwb.sptd.Lun = 0;
        sptdwb.sptd.CdbLength = 16;
        sptdwb.sptd.SenseInfoLength = 24;
        sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_OUT;
        sptdwb.sptd.DataTransferLength = inLen;
        sptdwb.sptd.TimeOutValue = 100;
        sptdwb.sptd.DataBuffer = lpInBuffer;
        sptdwb.sptd.SenseInfoOffset =
            offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
        //CopyMemory(&sptdwb.sptd.Cdb[0],kcw_p,16);

        // CovertBuf(sptdwb.sptd.Cdb);

        sptdwb.sptd.Cdb[0] = 0xdd;
        sptdwb.sptd.Cdb[1] = 0xfe;                         // Data mode
        sptdwb.sptd.Cdb[3] = (BYTE)inLen;
        sptdwb.sptd.Cdb[4] = (BYTE)(inLen>>8);


        length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
        status = DeviceIoControl(hDevice,
            IOCTL_SCSI_PASS_THROUGH_DIRECT,
            &sptdwb,
            length,
            &sptdwb,
            length,
            &returned,
            FALSE);
        if((sptdwb.sptd.ScsiStatus != 0) || (status == 0) )return FALSE;
    }

    if(OutLen>0)
    {

        //	dataBuffer=new BYTE[kcw_p->dKCWInLength+5];
        hglb_1=GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE,OutLen);
        dataBuffer=(BYTE *)LocalLock(hglb_1);
        ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
        sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
        //	sptdwb.sptd.PathId = 0;
        sptdwb.sptd.TargetId = 1;
        //	sptdwb.sptd.Lun = 0;
        sptdwb.sptd.CdbLength = 16;
        sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
        sptdwb.sptd.SenseInfoLength = 24;
        sptdwb.sptd.DataTransferLength =OutLen ;
        sptdwb.sptd.TimeOutValue = 100;
        sptdwb.sptd.DataBuffer = dataBuffer;
        sptdwb.sptd.SenseInfoOffset =
            offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
        //CopyMemory(&sptdwb.sptd.Cdb[0],kcw_p,16);
        sptdwb.sptd.Cdb[0] = 0xd1;
        sptdwb.sptd.Cdb[1] = 0xfe;
        sptdwb.sptd.Cdb[5] = (BYTE)OutLen;
        sptdwb.sptd.Cdb[6] = (BYTE)(OutLen>>8); // 命令
        length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
        status = DeviceIoControl(hDevice,
            IOCTL_SCSI_PASS_THROUGH_DIRECT,
            &sptdwb,
            length,
            &sptdwb,
            length,
            &returned,
            FALSE);
        if((sptdwb.sptd.ScsiStatus != 0) || (status == 0) ){GlobalUnlock(hglb_1);LocalFree(hglb_1);return FALSE;}
        CopyMemory(lpOutBuffer,dataBuffer,OutLen);
        GlobalUnlock(hglb_1);LocalFree(hglb_1);
        return TRUE;
    }

    return TRUE;
}


int SoftKey::NT_GetVersion(int *Version,char *InPath)
{
   int ret;
   BYTE array_in[25];BYTE array_out[25];
   HANDLE hUsbDevice=MyOpenPath(InPath);
   if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;
   array_in[1]=GETVERSION;
   ret= Hanldetransfe(hUsbDevice,array_in,1,array_out,1,InPath);
   if(ret!=0)return ret;
   *Version=array_out[0];
   return 0x0;
}

int SoftKey::ReadDword(  DWORD *in_data,char *Path)
{
   BYTE b[4];int result;
   DWORD t[4];
   result= NT_Read(&b[0],&b[1],&b[2],&b[3],Path);
   t[0]=b[0];t[1]=b[1];t[2]=b[2];t[3]=b[3];
   *in_data=t[0]|(t[1]<<8)|(t[2]<<16)|(t[3]<<24);
   return result;
}

int SoftKey::WriteDword(  DWORD *in_data,char *Path)
{
   BYTE b[4];
   b[0]=(BYTE)*in_data;b[1]=(BYTE)(*in_data>>8);
   b[2]=(BYTE)(*in_data>>16);b[3]=(BYTE)(*in_data>>24);
   return NT_Write(&b[0],&b[1],&b[2],&b[3],Path);
}

int SoftKey::WriteDword_2(  DWORD *in_data,char *Path)
{
   BYTE b[4];
   b[0]=(BYTE)*in_data;b[1]=(BYTE)(*in_data>>8);
   b[2]=(BYTE)(*in_data>>16);b[3]=(BYTE)(*in_data>>24);
   return NT_Write_2(&b[0],&b[1],&b[2],&b[3],Path);
}

int SoftKey::NT_Read(  BYTE * ele1,BYTE * ele2,BYTE * ele3,BYTE * ele4,char *Path)
{
   int ret;
   BYTE array_out[25];
   HANDLE hUsbDevice=MyOpenPath(Path);
   if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;
   ret= Hanldetransfe(hUsbDevice,NULL,0,array_out,4,Path);
   if(ret!=0)return ret;
   *ele1=array_out[0];
   *ele2=array_out[1];
   *ele3=array_out[2];
   *ele4=array_out[3];
   return 0;
}

int SoftKey::NT_Write(  BYTE * ele1,BYTE * ele2,BYTE * ele3,BYTE * ele4,char *InPath)
{
   int ret;
   BYTE array_in[25];
   HANDLE hUsbDevice=MyOpenPath(InPath);
   if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;
   array_in[1]=3;array_in[2]=*ele1;array_in[3]=*ele2;array_in[4]=*ele3;array_in[5]=*ele4;//	array_in[1]=*ele1;array_in[2]=*ele2;
   ret= Hanldetransfe(hUsbDevice,array_in,5,NULL,0,InPath);
   return ret;
}


int SoftKey::NT_Write_2(  BYTE * ele1,BYTE * ele2,BYTE * ele3,BYTE * ele4,char *InPath)
{
   int ret;
   BYTE array_in[25];
   HANDLE hUsbDevice=MyOpenPath(InPath);
   if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;
   array_in[1]=4;array_in[2]=*ele1;array_in[3]=*ele2;array_in[4]=*ele3;array_in[5]=*ele4;//	array_in[1]=*ele1;array_in[2]=*ele2;
   ret= Hanldetransfe(hUsbDevice,array_in,5,NULL,0,InPath);
   return ret;
}

int SoftKey::NT_FindPort(  int start,char *OutPath)
{
   int count=0;
   ZeroMemory(OutPath,sizeof(OutPath));
   if(isfindmydevice(start,&count,OutPath))return 0;
   return NotFondKey;
}

int SoftKey::NT_FindPort_2(  int start,DWORD in_data,DWORD verf_data,char *OutPath)
{
   int count=0;
   int pos;DWORD out_data;int ret;
   for(pos=start;pos<127;pos++)
   {
       if(!isfindmydevice(pos,&count,OutPath)){return NotFondKey;}
       ret=WriteDword( &in_data,OutPath);
       if(ret!=0){continue;}
       ret=ReadDword( &out_data,OutPath);
       if(ret!=0){continue;}
       if(out_data==verf_data){return 0;}
   }
   ZeroMemory(OutPath,sizeof(OutPath));
   return NotFondKey;
}

int SoftKey::Read(BYTE *OutData,short address,BYTE *password,char *Path )
{
    BYTE opcode=0x80;
        int ret;
    BYTE array_in[25],array_out[25];int n;
    HANDLE hUsbDevice=MyOpenPath(Path);
    if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;

    if(address>495 || address<0)return ErrAddrOver;
    if(address>255){opcode=0xa0;address=address-256;}

    //array_in[1]要输入的数据长度，array_in[2]要接受的数据长度。array_in[3]指令，
    //array_in[n+3]数据
    array_in[1]=READBYTE;//read 0x10;write 0x11;
    array_in[2]=opcode;//0x01;//read 0x80 ;write 0x40;
    array_in[3]=(BYTE)address;
    for(n=5;n<14;n++)
    {
        array_in[n]=*password;
        password++;
    }
    ret= Hanldetransfe(hUsbDevice,array_in,13,array_out,2,Path);
    if(ret!=0)return ret;
    if(array_out[0]!=0x53 )
    {
        return ErrReadPWD;//表示失败；
    }
    *OutData=array_out[1];
    return 0;
}

 int  SoftKey::Write(BYTE InData,short address,BYTE *password,char *Path )
{
    BYTE opcode=0x40;int ret;

    BYTE array_in[25],array_out[25];int n;
    HANDLE hUsbDevice=MyOpenPath(Path);
    if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;

    if(address>511 || address<0)return ErrAddrOver;
    if(InData<0)return -79;
    if(address>255){opcode=0x60;address=address-256;}

    array_in[1]=WRITEBYTE;//read 0x10;write 0x11;
    array_in[2]=opcode;//0x01;//read 0x80 ;write 0x40;
    array_in[3]=(BYTE)address;
    array_in[4]=InData;
    for(n=5;n<14;n++)
    {
        array_in[n]=*password;
        password++;
    }
    ret= Hanldetransfe(hUsbDevice,array_in,13,array_out,2,Path);
    if(ret!=0)return ret;
    if(array_out[1]!=1 )
    {
        return ErrWritePWD;//表示失败；
    }
    return 0;
}

 int SoftKey::Y_Read(BYTE *OutData,short address ,short len,BYTE *password,char *Path )
{
    short addr_l,addr_h;int n;int ret;

    BYTE array_in[512],array_out[512];
    HANDLE hUsbDevice=MyOpenPath(Path);
    if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;

    if(address>495 || address<0)return ErrAddrOver;
    if( len>255)return -87;
    if(len+address>511)return ErrAddrOver;
    {addr_h=(address>>8)<<1;addr_l=address&0xff;}

    //array_in[1]要输入的数据长度，array_in[2]要接受的数据长度。array_in[3]指令，
    //array_in[n+3]数据
    array_in[1]=YTREADBUF;//read 0x10;write 0x11;
    array_in[2]=(BYTE)addr_h;//0x01;//read 0x80 ;write 0x40;
    array_in[3]=(BYTE)addr_l;
    array_in[4]=(BYTE)len;
    for(n=0;n<8;n++)
    {
        array_in[5+n]=*password;
        password++;
    }
    ret= Hanldetransfe(hUsbDevice,array_in,13,array_out,len+1,Path);
    if(ret!=0)return ret;

    if(array_out[0]!=0x0 )
    {
        return ErrReadPWD;//表示失败；
    }
    for(n=0;n<len;n++)
    {
        *OutData=array_out[n+1];
        OutData++;
    }
    return 0;
}

 int SoftKey::Y_Write(BYTE *InData,short address,short len,BYTE *password,char *Path )
{
    short addr_l,addr_h;int n;int ret;
    BYTE array_in[512],array_out[512];
    HANDLE hUsbDevice=MyOpenPath(Path);
    if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;

    if(len>255)return -87;
    if(address+len-1>511 || address<0)return ErrAddrOver;
    {addr_h=(address>>8)<<1;addr_l=address&0xff;}

    array_in[1]=YTWRITEBUF;//read 0x10;write 0x11;
    array_in[2]=(BYTE)addr_h;//0x01;//read 0x80 ;write 0x40;
    array_in[3]=(BYTE)addr_l;
    array_in[4]=(BYTE )len;
    for(n=0;n<8;n++)
    {
        array_in[5+n]=*password;
        password++;
    }
    for(n=0;n<len;n++)
    {
        array_in[13+n]=*InData;
        InData++;
    }
    ret= Hanldetransfe(hUsbDevice,array_in,13+len,array_out,3,Path);
    if(ret!=0)return ret;

    if(array_out[0]!=0x0)
    {
        return ErrWritePWD;//表示失败；
    }
    return 0;
}

BOOL SoftKey::isfindmydevice( int pos ,int *count,char *OutPath)
{

   HDEVINFO                            hardwareDeviceInfo=NULL;
   SP_INTERFACE_DEVICE_DATA            deviceInfoData;
   ULONG                               i;
   GUID                                hidGuid;
   PSP_INTERFACE_DEVICE_DETAIL_DATA_A    functionClassDeviceData = NULL;
   ULONG                               predictedLength = 0;
   ULONG                               requiredLength = 0;
   HANDLE                              d_handle;
   HIDD_ATTRIBUTES                    Attributes;

   if(!LoadLib())return FALSE;
   *count=0;

   (*l_HidD_GetHidGuid) (&hidGuid);

   hardwareDeviceInfo = (  *l_SetupDiGetClassDevsA ) ( &hidGuid,
                                              NULL, // Define no enumerator (global)
                                              NULL, // Define no
                                              DIGCF_PRESENT | DIGCF_DEVICEINTERFACE); // Function class devices.

  if(hardwareDeviceInfo==INVALID_HANDLE_VALUE)
  {
      return FALSE;
  }
   deviceInfoData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);

       for (i=0; (*l_SetupDiEnumDeviceInterfaces) (hardwareDeviceInfo,0,&hidGuid,i,&deviceInfoData); i++)
       {
               if (ERROR_NO_MORE_ITEMS == GetLastError()) break;
               (*l_SetupDiGetDeviceInterfaceDetailA) (hardwareDeviceInfo,&deviceInfoData,NULL,
                       0,&requiredLength,NULL);
               predictedLength = requiredLength;
               functionClassDeviceData =(SP_INTERFACE_DEVICE_DETAIL_DATA_A *) malloc (predictedLength);
               if (functionClassDeviceData)
               {
#ifdef _WIN64
                   functionClassDeviceData->cbSize = sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA_A);
#else
                   functionClassDeviceData->cbSize = 5;
#endif
               }
               else
               {
                   (*l_SetupDiDestroyDeviceInfoList) (hardwareDeviceInfo);
                   return FALSE;
               }

               if (! (*l_SetupDiGetDeviceInterfaceDetailA) (hardwareDeviceInfo,&deviceInfoData,functionClassDeviceData,
                          predictedLength,&requiredLength,NULL))
               {
                   free( functionClassDeviceData);
                   (*l_SetupDiDestroyDeviceInfoList) (hardwareDeviceInfo);
                   return FALSE;
               }
                d_handle = CreateFileA (functionClassDeviceData->DevicePath,  GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL, OPEN_EXISTING, 0,  NULL);
                if (INVALID_HANDLE_VALUE != d_handle)
                {
                    if ((*l_HidD_GetAttributes) (d_handle, &Attributes))
                    {
                        if(((Attributes.ProductID==PID) && (Attributes.VendorID==VID)) ||
                            ((Attributes.ProductID==PID_NEW) && (Attributes.VendorID==VID_NEW)) ||
                               ((Attributes.ProductID==PID_NEW_2) && (Attributes.VendorID==VID_NEW_2)))
                           {
                                   if(pos==*count)
                                   {
                                       lstrcpyA(OutPath,functionClassDeviceData->DevicePath);
                                       free(functionClassDeviceData);
                                       (*l_SetupDiDestroyDeviceInfoList) (hardwareDeviceInfo);
                                        CloseHandle(d_handle);
                                       return TRUE;
                                   }
                                   (*count)++;
                           }
                   }
                    CloseHandle(d_handle);
                }
               free(functionClassDeviceData);
           }
   (*l_SetupDiDestroyDeviceInfoList) (hardwareDeviceInfo);
   return FALSE;

}


BOOL SoftKey::SetFeature (HANDLE hDevice,BYTE *array_in,INT in_len)
/*++
RoutineDescription:
Given a struct _HID_DEVICE, take the information in the HID_DATA array
pack it into multiple reports and send it to the hid device via HidD_SetFeature()
--*/
{

   PHIDP_PREPARSED_DATA Ppd;
   HIDP_CAPS            Caps;
    BOOLEAN   Status;
   BOOLEAN   FeatureStatus;
   DWORD     ErrorCode;
   INT		  i;
   BYTE FeatureReportBuffer[512];

   if (!(*l_HidD_GetPreparsedData) (hDevice, &Ppd)) return FALSE;

   if (!(*l_HidP_GetCaps) (Ppd, &Caps))
   {
           (*l_HidD_FreePreparsedData)(Ppd);
           return FALSE;
   }

   Status = TRUE;

   FeatureReportBuffer[0]=0x02;

   array_in++;
   for(i=1;i<=in_len;i++)
   {
       FeatureReportBuffer[i]=*array_in;
       array_in++;
   }
   FeatureStatus =((*l_HidD_SetFeature)(hDevice,
                                 FeatureReportBuffer,
                                 Caps.FeatureReportByteLength));

   ErrorCode = GetLastError();

   Status = Status && FeatureStatus;
   (*l_HidD_FreePreparsedData)(Ppd);
   return (Status);
}

BOOL SoftKey::GetFeature (HANDLE hDevice,BYTE *array_out,INT out_len)
/*++
RoutineDescription:
  Given a struct _HID_DEVICE, fill in the feature data structures with
  all features on the device.  May issue multiple HidD_GetFeature() calls to
  deal with multiple report IDs.
--*/
{
  // ULONG     Index;
   BOOLEAN   FeatureStatus;
   BOOLEAN   Status;
   INT		  i;
   BYTE      FeatureReportBuffer[512];
   PHIDP_PREPARSED_DATA Ppd;
   HIDP_CAPS            Caps;
   if (!(*l_HidD_GetPreparsedData) (hDevice, &Ppd)) return FALSE;

   if (!(*l_HidP_GetCaps) (Ppd, &Caps))
   {
           (*l_HidD_FreePreparsedData)(Ppd);
           return FALSE;
   }
   /*
   // As with writing data, the IsDataSet value in all the structures should be
   //    set to FALSE to indicate that the value has yet to have been set
   */
   Status = TRUE;

   memset(FeatureReportBuffer, 0x00, 512);

   FeatureReportBuffer[0] = 0x01;

   FeatureStatus = (*l_HidD_GetFeature) (hDevice,
                                     FeatureReportBuffer,
                                     Caps.FeatureReportByteLength);
   if (FeatureStatus)
   {

       if(FeatureStatus)
       {
           for(i=0;i<out_len;i++)
           {
               *array_out=FeatureReportBuffer[i];
               array_out++;
           }
       }
   }

   Status = Status && FeatureStatus;
   (*l_HidD_FreePreparsedData)(Ppd);

  return (Status);
}


int SoftKey::NT_GetID(  DWORD *ID_1,DWORD *ID_2,char *InPath)
{
       DWORD t[8];int ret;
   BYTE array_in[25];BYTE array_out[25];
   HANDLE hUsbDevice=MyOpenPath(InPath);
   if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;
   array_in[1]=GETID;
   ret= Hanldetransfe(hUsbDevice,array_in,1,array_out,8,InPath);
   if(ret!=0)return ret;
   t[0]=array_out[0];t[1]=array_out[1];t[2]=array_out[2];t[3]=array_out[3];
   t[4]=array_out[4];t[5]=array_out[5];t[6]=array_out[6];t[7]=array_out[7];
   *ID_1=t[3]|(t[2]<<8)|(t[1]<<16)|(t[0]<<24);
   *ID_2=t[7]|(t[6]<<8)|(t[5]<<16)|(t[4]<<24);
   return 0;
}

int SoftKey::NT_Cal(  BYTE * InBuf,BYTE *OutBuf,char *InPath)
{
   int n;int ret;
   BYTE array_in[25];BYTE array_out[25];
   HANDLE hUsbDevice=MyOpenPath(InPath);
   if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;
   array_in[1]=CAL_TEA;
   for (n=2;n<10;n++)
   {
       array_in[n]=InBuf[n-2];
   }
   ret= Hanldetransfe(hUsbDevice,array_in,9,array_out,10,InPath);
   if(ret!=0)return ret;
   memcpy(OutBuf,array_out,8);
   if(array_out[8]!=0x55)
   {
       return ErrResult;
   }
   return 0;
}

int SoftKey::NT_SetCal_2(BYTE *InData,BYTE IsHi,char *Path )
{
      int n;int ret;
      BYTE array_in[30],array_out[25];
      BYTE opcode=SET_TEAKEY;
      HANDLE hUsbDevice=MyOpenPath(Path);
      if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;

      array_in[1]=opcode;
      array_in[2]=IsHi;
      for(n=0;n<8;n++)
      {
          array_in[3+n]=*InData;
          InData++;
      }
      ret= Hanldetransfe(hUsbDevice,array_in,11,array_out,3,Path);
      if(ret!=0)return ret;
      if(array_out[0]!=0x0)
      {
          return ErrResult;//表示失败；
      }
      return 0;
}

int SoftKey::NT_ReSet(char *Path )
{

       int ret;
      BYTE array_in[30],array_out[25];
      HANDLE hUsbDevice=MyOpenPath(Path);
      if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;
      array_in[1]=MYRESET;
      ret= Hanldetransfe(hUsbDevice,array_in,2,array_out,1,Path);
      if(ret!=0)return ret;
      if(ret!=0)return ret;
      if(array_out[0]!=0x0)
      {
          return ErrResult;//表示失败；
      }
      return 0;
}

int SoftKey::NT_SetID(  BYTE * InBuf,char *InPath)
{
  int n,ret;
  BYTE array_in[25];BYTE array_out[25];
  HANDLE hUsbDevice=MyOpenPath(InPath);
  if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;
  array_in[1]=SETID;
  for (n=2;n<10;n++)
  {
      array_in[n]=InBuf[n-2];
  }
  ret=Hanldetransfe(hUsbDevice,array_in,9,array_out,1,InPath);
  if(ret!=0)return ret;
  if(array_out[0]!=0x0)
  {
       return ErrResult;//表示失败；
  }
  return 0;
}

int  SoftKey::NT_GetProduceDate(  BYTE *OutDate,char *InPath)
{
       int ret;
    BYTE array_in[25];
    HANDLE hUsbDevice=MyOpenPath(InPath);
    if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;
    array_in[1]=GETSN;
    ret= Hanldetransfe(hUsbDevice,array_in,1,OutDate,8,InPath);
    return ret;
}

int  SoftKey::F_GetVerEx(int *Version,char *InPath)
{
       int ret;
   BYTE array_in[25];BYTE array_out[25];
   HANDLE hUsbDevice=MyOpenPath(InPath);
   if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;

   array_in[1]=GETVEREX;
   ret= Hanldetransfe(hUsbDevice,array_in,1,array_out,1,InPath);
   if(ret!=0)return ret;
   *Version=array_out[0];
   return 0x0;
}


int SoftKey::NT_Cal_New(  BYTE * InBuf,BYTE *OutBuf,char *InPath)
{
   int n,ret;
   BYTE array_in[25];BYTE array_out[25];
   HANDLE hUsbDevice=MyOpenPath(InPath);
   if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;

   array_in[1]=CAL_TEA_2;
   for (n=2;n<10;n++)
   {
       array_in[n]=InBuf[n-2];
   }
   ret= Hanldetransfe(hUsbDevice,array_in,9,array_out,10,InPath);
   if(ret!=0)return ret;
   memcpy(OutBuf,array_out,8);
   if(array_out[8]!=0x55)
   {
       return ErrResult;
   }

   return 0;
}

int SoftKey::NT_SetCal_New(BYTE *InData,BYTE IsHi,char *Path )
{
    int n,ret;
    BYTE array_in[30],array_out[25];
    BYTE opcode=SET_TEAKEY_2;
    HANDLE hUsbDevice=MyOpenPath(Path);
    if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;

    array_in[1]=opcode;
    array_in[2]=IsHi;
    for(n=0;n<8;n++)
    {
      array_in[3+n]=*InData;
      InData++;
    }
    ret= Hanldetransfe(hUsbDevice,array_in,11,array_out,3,Path);
    if(ret!=0)return ret;
    if(array_out[0]!=0x0)
    {
        return ErrResult;//表示失败；
    }
    return 0;
}

int SoftKey::NT_Set_SM2_KeyPair(BYTE *PriKey,BYTE *PubKeyX,BYTE *PubKeyY,char *sm2UserName,char *Path )
{

      int ret;
      BYTE array_in[256],array_out[25];

      HANDLE hUsbDevice=MyOpenPath(Path);
      if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;
      memset(array_in,0,256);

      array_in[1]=SET_ECC_KEY;
      memcpy(&array_in[2+ECC_MAXLEN*0],PriKey,ECC_MAXLEN);
      memcpy(&array_in[2+ECC_MAXLEN*1],PubKeyX,ECC_MAXLEN);
      memcpy(&array_in[2+ECC_MAXLEN*2],PubKeyY,ECC_MAXLEN);
      if(sm2UserName!=0)
      {
          memcpy(&array_in[2+ECC_MAXLEN*3],sm2UserName,USERID_LEN);
      }
      ret= Hanldetransfe(hUsbDevice,array_in,ECC_MAXLEN*3+2+USERID_LEN,array_out,1,Path);
      if(ret!=0)return ret;
      if(array_out[0]!=0x20)return USBStatusFail;

      return 0;
}

int SoftKey::NT_Get_SM2_PubKey(BYTE *KGx,BYTE *KGy,char *sm2UserName,char *Path )
{

      int ret;
      BYTE array_in[256],array_out[256];
      HANDLE hUsbDevice=MyOpenPath(Path);
      array_out[0]=0xfb;
      if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;

      array_in[1]=GET_ECC_KEY;
      ret= Hanldetransfe(hUsbDevice,array_in,2,array_out,ECC_MAXLEN*2+2+USERID_LEN,Path);
      if(ret!=0)return ret;

      if(array_out[0]!=0x20)return USBStatusFail;
      memcpy(KGx,&array_out[1+ECC_MAXLEN*0],ECC_MAXLEN);
      memcpy(KGy,&array_out[1+ECC_MAXLEN*1],ECC_MAXLEN);
      memcpy(sm2UserName,&array_out[1+ECC_MAXLEN*2],USERID_LEN);

      return 0;
}


int SoftKey::NT_GenKeyPair(BYTE* PriKey,BYTE *PubKey,char *Path )
{

      int ret;
      BYTE array_in[256],array_out[256];
      HANDLE hUsbDevice=MyOpenPath(Path);
      array_out[0]=0xfb;
      if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;

      array_in[1]=GEN_KEYPAIR;
      ret= Hanldetransfe(hUsbDevice,array_in,2,array_out,ECC_MAXLEN*3+2,Path);
      if(ret!=0)return ret;

      if(array_out[0]!=0x20)
      {
          return FAILEDGENKEYPAIR;//表示读取失败；
      }
      memcpy(PriKey,&array_out[1+ECC_MAXLEN*0],ECC_MAXLEN);
      memcpy(PubKey,&array_out[1+ECC_MAXLEN*1],ECC_MAXLEN*2+1);
      return 0;
}

int SoftKey::NT_Set_Pin(char *old_pin,char *new_pin,char *Path )
{

      int ret;
      BYTE array_in[256],array_out[25];
      HANDLE hUsbDevice=MyOpenPath(Path);
      if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;

      array_in[1]=SET_PIN;
      memcpy(&array_in[2+PIN_LEN*0],old_pin,PIN_LEN);
      memcpy(&array_in[2+PIN_LEN*1],new_pin,PIN_LEN);

      ret= Hanldetransfe(hUsbDevice,array_in,PIN_LEN*2+2,array_out,2,Path);
      if(ret!=0)return ret;

      if(array_out[0]!=0x20)return USBStatusFail;
      if(array_out[1]!=0x20)return FAILPINPWD;
      return 0;
}


int SoftKey::NT_SM2_Enc(BYTE *inbuf,BYTE *outbuf,BYTE inlen,char *Path )
{

      int n;int ret;
      BYTE array_in[256],array_out[256];

      HANDLE hUsbDevice=MyOpenPath(Path);
      if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;

      array_out[0]=0xfb;
      array_in[1]=MYENC;
      array_in[2]=inlen;
      for(n=0;n<inlen;n++)
      {
        array_in[3+n]=inbuf[n];
      }
      ret= Hanldetransfe(hUsbDevice,array_in,inlen+1+2,array_out,inlen+SM2_ADDBYTE+3,Path);
      if(ret!=0)return ret;

      if(array_out[0]!=0x20)return USBStatusFail;
      if(array_out[1]==0)return FAILENC;

      memcpy(outbuf,&array_out[2],inlen+SM2_ADDBYTE);

      return 0;
}

int SoftKey::NT_SM2_Dec(BYTE *inbuf,BYTE *outbuf,BYTE inlen,char* pin,char *Path )
{

      int n;int ret;
      BYTE array_in[256],array_out[256];

      HANDLE hUsbDevice=MyOpenPath(Path);
      if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;

      array_out[0]=0xfb;
      array_in[1]=MYDEC;
      memcpy(&array_in[2],pin,PIN_LEN);
      array_in[2+PIN_LEN]=inlen;
      for(n=0;n<inlen;n++)
      {
        array_in[2+PIN_LEN+1+n]=inbuf[n];
      }
      ret= Hanldetransfe(hUsbDevice,array_in,inlen+1+2+PIN_LEN,array_out,inlen-SM2_ADDBYTE+4,Path);
      if(ret!=0)return ret;

      if(array_out[2]!=0x20)return FAILPINPWD;
      if(array_out[1]==0) return FAILENC;
      if(array_out[0]!=0x20)return USBStatusFail;
      memcpy(outbuf,&array_out[3],inlen-SM2_ADDBYTE);

      return 0;
}

int SoftKey::NT_Sign(BYTE *inbuf,BYTE *outbuf,char* pin,char *Path )
{

      int n;int ret;
      BYTE array_in[256],array_out[256];
      HANDLE hUsbDevice=MyOpenPath(Path);
      if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;

      array_out[0]=0xfb;
      array_in[1]=YTSIGN;
      memcpy(&array_in[2],pin,PIN_LEN);
      for(n=0;n<32;n++)
      {
        array_in[2+PIN_LEN+n]=inbuf[n];
      }
      ret= Hanldetransfe(hUsbDevice,array_in,32+2+PIN_LEN,array_out,64+2,Path);
      if(ret!=0)return ret;

      if(array_out[1]!=0x20)return FAILPINPWD;
      if(array_out[0]!=0x20)return USBStatusFail;
      memcpy(outbuf,&array_out[2],64);

      return 0;
}

int SoftKey::NT_Sign_2(BYTE *inbuf,BYTE *outbuf,char* pin,char *Path )
{

      int n;int ret;
      BYTE array_in[256],array_out[256];
      HANDLE hUsbDevice=MyOpenPath(Path);
      if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;

      array_out[0]=0xfb;
      array_in[1]=YTSIGN_2;
      memcpy(&array_in[2],pin,PIN_LEN);
      for(n=0;n<32;n++)
      {
        array_in[2+PIN_LEN+n]=inbuf[n];
      }
      ret= Hanldetransfe(hUsbDevice,array_in,32+2+PIN_LEN,array_out,64+2,Path);
      if(ret!=0)return ret;
      if(array_out[1]!=0x20)return FAILPINPWD;
      if(array_out[0]!=0x20)return USBStatusFail;
      memcpy(outbuf,&array_out[2],64);

      return 0;
}

int SoftKey::NT_Verfiy(BYTE *inbuf,BYTE *InSignBuf,BOOL *outbiao,char *Path )
{

      int n;int ret;
      BYTE array_in[256],array_out[256];
      HANDLE hUsbDevice=MyOpenPath(Path);
      if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;

      array_out[0]=0xfb;
      array_in[1]=YTVERIFY;
      for(n=0;n<32;n++)
      {
        array_in[2+n]=inbuf[n];
      }
      for(n=0;n<64;n++)
      {
        array_in[2+32+n]=InSignBuf[n];
      }
      ret= Hanldetransfe(hUsbDevice,array_in,32+2+64,array_out,3,Path);
      if(ret!=0)return ret;
      *outbiao=array_out[1];
      if(array_out[0]!=0x20)return USBStatusFail;

      return 0;
}

int  SoftKey::NT_GetChipID(  BYTE *OutChipID,char *InPath)
{
   int ret;
   BYTE array_in[25];BYTE array_out[25];
   HANDLE hUsbDevice=MyOpenPath(InPath);
   if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;
   array_in[1]=GET_CHIPID;
   ret= Hanldetransfe(hUsbDevice,array_in,1,array_out,17,InPath);
   if(ret!=0)return ret;
   if(array_out[0]!=0x20)return USBStatusFail;
   memcpy(OutChipID,&array_out[1],16);

   return 0;
}

int SoftKey::Sub_SetOnly(BOOL IsOnly,BYTE Flag,char *InPath)
{
   int ret;
   BYTE array_in[25];BYTE array_out[25];
   HANDLE hUsbDevice=MyOpenPath(InPath);
   if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;
   array_in[1]=Flag;
   if(IsOnly)array_in[2]=0;else array_in[2]=0xff;
   ret=Hanldetransfe(hUsbDevice,array_in,3,array_out,1,InPath);
   if(ret!=0)return ret;
   if(array_out[0]!=0x0)
   {
       return ErrResult;//表示失败；
   }
   return 0;
}

int SoftKey::NT_SetHidOnly(  BOOL IsHidOnly,char *InPath)
{
   return Sub_SetOnly(IsHidOnly,SETHIDONLY,InPath);
}

int  SoftKey::NT_SetUReadOnly(char *InPath)
{
   return Sub_SetOnly(TRUE,SETREADONLY,InPath);
}

int SoftKey::GetTrashBufLen(char * Path,int *OutLen)
{
   PHIDP_PREPARSED_DATA Ppd;
   HIDP_CAPS            Caps;
   HANDLE hUsbDevice=MyOpenPath(Path);
   if(hUsbDevice == INVALID_HANDLE_VALUE)return NotFondKey;
   if (!(*l_HidD_GetPreparsedData) (hUsbDevice, &Ppd)) return ErrSendData;
   if (!(*l_HidP_GetCaps) (Ppd, &Caps))
   {
       (*l_HidD_FreePreparsedData)(Ppd);
       return ErrSendData;
   }
   CloseHandle(hUsbDevice);
   *OutLen=Caps.FeatureReportByteLength-5;
    (*l_HidD_FreePreparsedData)(Ppd);
   return 0;
}

int SoftKey::NT_GetIDVersion(int *Version,char *InPath)
{
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(NT_GetVersion)(Version,InPath);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

int SoftKey::GetID(  DWORD *ID_1,DWORD *ID_2,char *InPath)
{
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(NT_GetID)(ID_1,ID_2,InPath);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

int SoftKey::sRead(  DWORD *in_data,char *Path)
{
    int ret;
    //////////////////////////////////////////////////////////////////////////////////

    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(ReadDword)(in_data,Path);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

int SoftKey::sWrite(  DWORD out_data,char *Path)
{
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(WriteDword)(&out_data,Path);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

int SoftKey::YWrite(BYTE InData,short address,char * HKey,char *LKey,char *Path )
{
    if(address>495){return  -81;}
    BYTE ary1[8];
    myconvert(HKey,LKey,ary1);
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(Write)(InData,address,ary1,Path);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

 int SoftKey::YRead(BYTE *OutData,short address,char * HKey,char *LKey,char *Path )
{
    if(address>495){return  -81;}
    BYTE ary1[8];
    myconvert(HKey,LKey,ary1);
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(Read)(OutData,address,ary1,Path);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

int SoftKey::YWriteEx(BYTE *InData,short Address,short len,char *HKey,char *LKey,char *Path )
{
    int ret;BYTE password[8];int n,trashLen;int temp_leave, leave; HANDLE hsignal;
     if(Address+len-1>495 || Address<0)return -81;

     ret=GetTrashBufLen(Path,&trashLen);
     trashLen=trashLen-8;
     if(ret!=0)return ret;

     myconvert(HKey,LKey,password);

     hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");

     WaitForSingleObject(hsignal,INFINITE);
     temp_leave=Address-Address/trashLen*trashLen; leave=trashLen-temp_leave;
     if(leave>len){leave=len;}
     if(leave>0)
     {
         for(n=0;n<leave/trashLen;n++)
         {
             ret=(Y_Write)(InData+n*trashLen,Address+n*trashLen,trashLen,password,Path);
             if(ret!=0){ReleaseSemaphore(hsignal,1,NULL);CloseHandle(hsignal);return  ret;}
         }
         if(leave-trashLen*n>0)
         {
             ret=(Y_Write)(InData+n*trashLen,Address+n*trashLen,(BYTE)(leave-n*trashLen),password,Path);
             if(ret!=0){ReleaseSemaphore(hsignal,1,NULL);CloseHandle(hsignal);return  ret;}

         }
     }
     len=len-leave;Address=Address+leave;InData=InData+leave;
     if (len>0)
     {
         for(n=0;n<len/trashLen;n++)
         {
             ret=(Y_Write)(InData+n*trashLen,Address+n*trashLen,trashLen,password,Path);
             if(ret!=0){ReleaseSemaphore(hsignal,1,NULL);CloseHandle(hsignal);return ret;}
         }
         if(len-trashLen*n>0)
         {
             ret=(Y_Write)(InData+n*trashLen,Address+n*trashLen,(BYTE)(len-n*trashLen),password,Path);
             if(ret!=0){ReleaseSemaphore(hsignal,1,NULL);CloseHandle(hsignal);return ret;}

         }
     }
     ReleaseSemaphore(hsignal,1,NULL);
     CloseHandle(hsignal);
     return ret;
}

int SoftKey::YReadEx(BYTE *OutData,short Address,short len,char *HKey,char *LKey,char *Path )
{
    int ret;BYTE password[8];int n,trashLen; HANDLE hsignal;
    if(Address+len-1>495 || Address<0)return -81;

    ret=GetTrashBufLen(Path,&trashLen);
    if(ret!=0)return ret;

   myconvert(HKey,LKey,password);

    hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    for(n=0;n<len/trashLen;n++)
    {
        ret=(Y_Read)(OutData+n*trashLen,Address+n*trashLen,trashLen,password,Path);
        if(ret!=0){ReleaseSemaphore(hsignal,1,NULL);CloseHandle(hsignal);return ret;}
    }
    if(len-trashLen*n>0)
    {
        ret=(Y_Read)(OutData+n*trashLen,Address+n*trashLen,(len-trashLen*n),password,Path);
        if(ret!=0){ReleaseSemaphore(hsignal,1,NULL);CloseHandle(hsignal);return ret;}

    }
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

int SoftKey::FindPort_2(  int start,DWORD in_data,DWORD verf_data,char *OutPath)
{
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(NT_FindPort_2)(start,in_data,verf_data,OutPath);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

int SoftKey::FindPort(  int start,char *OutPath)
{
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(NT_FindPort)(start,OutPath);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

int SoftKey::sWrite_2(  DWORD out_data,char *Path)
{
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(WriteDword_2)(&out_data,Path);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

VOID SoftKey::myconvert(char *hkey,char *lkey,BYTE *out_data)
{
    DWORD z,z1;int n;
    z=HexToInt(hkey);
    z1=HexToInt(lkey);
    for(n=0;n<=3;n++)
    {
        *out_data=(BYTE)((z<<(n*8))>>24);
        out_data++;
    }
    for(n=0;n<=3;n++)
    {
        *out_data=(BYTE)((z1<<(n*8))>>24);
        out_data++;
    }
}

DWORD SoftKey::HexToInt(char* s)
{
    char hexch[] = "0123456789ABCDEF";
    size_t i;
   DWORD r,n,k,j;
   char ch;

    k=1; r=0;
    for (i=strlen(s);  i>0; i--) {
    ch = s[i-1]; if (ch > 0x3f) ch &= 0xDF;
    n = 0;
        for (j = 0; j<16; j++)
        if (ch == hexch[j])
            n = j;
        r += (n*k);
        k *= 16;
    }
    return r;
}

int SoftKey::SetReadPassword(char * W_HKey,char *W_LKey,char * new_HKey,char *new_LKey,char *InPath)
{

    BYTE ary1[8];BYTE ary2[8];
    myconvert(W_HKey,W_LKey,ary1);
    myconvert(new_HKey,new_LKey,ary2);
    int ret;int address=0x1f0;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(Y_Write)(ary2,address,8,ary1,InPath);
    if(ret!=0){ReleaseSemaphore(hsignal,1,NULL);return ret;}
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;

}


int SoftKey::SetWritePassword(char * W_HKey,char *W_LKey,char * new_HKey,char *new_LKey,char *InPath)
{
    BYTE ary1[8];BYTE ary2[8];
    myconvert(W_HKey,W_LKey,ary1);
    myconvert(new_HKey,new_LKey,ary2);
    int ret;int address=0x1f8;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(Y_Write)(ary2,address,8,ary1,InPath);
    if(ret!=0){ReleaseSemaphore(hsignal,1,NULL);return ret;}
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;

}

int SoftKey::YWriteString(char *InString,short Address,char * HKey,char *LKey,char *Path )
{
    BYTE ary1[8];int n,trashLen;int ret=0,outlen,total_len;HANDLE hsignal;//int versionex;

    ret=GetTrashBufLen(Path,&trashLen);
    trashLen=trashLen-8;
    if(ret!=0)return ret;

    myconvert(HKey,LKey,ary1);

    outlen=lstrlenA(InString);
    total_len=Address+outlen;
    if(total_len>495){return -47;}

    hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    {
        WaitForSingleObject(hsignal,INFINITE);
        for(n=0;n<outlen/trashLen;n++)
        {
            ret=(Y_Write)((BYTE *)(InString+n*trashLen),Address+n*trashLen,trashLen,ary1,Path);
            if(ret!=0){ReleaseSemaphore(hsignal,1,NULL);CloseHandle(hsignal);return  ret;}
        }
        if(outlen-trashLen*n>0)
        {
            ret=(Y_Write)((BYTE *)(InString+n*trashLen),Address+n*trashLen,(BYTE)(outlen-n*trashLen),ary1,Path);
            if(ret!=0){ReleaseSemaphore(hsignal,1,NULL);CloseHandle(hsignal);return  ret;}

        }
        ReleaseSemaphore(hsignal,1,NULL);
    }
    CloseHandle(hsignal);
    return ret;
}

int SoftKey::YReadString(char *string ,short Address,int len,char * HKey,char *LKey,char *Path )
{
    BYTE ary1[8];int n,trashLen;int ret=0;HANDLE hsignal;int total_len;//DWORD z,z1;int versionex;

        ret=GetTrashBufLen(Path,&trashLen);
        if(ret!=0)return ret;

        myconvert(HKey,LKey,ary1);

        total_len=Address+len;
        if(total_len>495){return -47;}

        hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
        {
            WaitForSingleObject(hsignal,INFINITE);
            for(n=0;n<len/trashLen;n++)
            {
                ret=(Y_Read)((BYTE*)(string+n*trashLen),Address+n*trashLen,trashLen,ary1,Path);
                if(ret!=0)
                {
                    ReleaseSemaphore(hsignal,1,NULL);CloseHandle(hsignal);
                    return ret;
                }
            }
            if(len-trashLen*n>0)
            {
                ret=(Y_Read)((BYTE*)(string+n*trashLen),Address+n*trashLen,(len-trashLen*n),ary1,Path);
                if(ret!=0)
                {
                    ReleaseSemaphore(hsignal,1,NULL);CloseHandle(hsignal);
                    return ret;
                }

            }
            ReleaseSemaphore(hsignal,1,NULL);
        }
        CloseHandle(hsignal);
        return ret;
}

 void SoftKey::ByteArrayToHexString(BYTE *in_data,char * OutString,int len)
{
    memset(OutString,0,len*2);
    char temp[5];int n;
    for (n=1;n<=len;n++)
    {
        wsprintfA(temp,"%02X",*in_data);
        in_data++;
        lstrcatA(OutString,temp);
    }
}

 VOID SoftKey::HexStringToByteArray(char * InString,BYTE *in_data)
{
    int len=lstrlenA(InString);
    char temp[5];int n;
    for (n=1;n<=len;n=n+2)
    {
        ZeroMemory(temp,sizeof(temp));
        lstrcpynA(temp,&InString[n-1],2+1);
        *in_data=(BYTE)HexToInt(temp);
        in_data++;
    }
}

 void SoftKey::HexStringToByteArrayEx(char * InString,BYTE *in_data)
{
    int len=lstrlenA(InString);
    char temp[5];int n;
    if(len>32)len=32;
    for (n=1;n<=len;n=n+2)
    {
        ZeroMemory(temp,sizeof(temp));
        lstrcpynA(temp,&InString[n-1],2+1);
        *in_data=(BYTE)HexToInt(temp);
        in_data++;
    }

}

int SoftKey::SetCal_2(char *Key,char *InPath)
{
    BYTE KeyBuf[16];
    memset(KeyBuf,0,16);
    HexStringToByteArrayEx(Key,KeyBuf);
    //注意，这里是地址互换的
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(NT_SetCal_2)(&KeyBuf[8],0,InPath);
    if(ret!=0)goto error1;
    ret=(NT_SetCal_2)(&KeyBuf[0],1,InPath);
error1:
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;

}

int SoftKey::Cal(  BYTE *InBuf,BYTE *OutBuf,char *Path)
{
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=NT_Cal(InBuf,OutBuf,Path);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

int SoftKey::GetLen(char *InString)
{
    return lstrlenA(InString)+1;
}


int SoftKey::EncString(  char *InString,char *OutString,char *Path)
{

    int ret;int n;BYTE *outbuf;
    int len=lstrlenA(InString)+1;if(len<8)len=8;
    outbuf=new BYTE[len];
    memset(outbuf,0,len);
    memcpy(outbuf,InString,lstrlenA(InString)+1);
    for(n=0;n<=(len-8);n=n+8)
    {
        ret=Cal((BYTE *)&outbuf[n],&outbuf[n],Path);
        if(ret!=0){delete [] outbuf;return ret;}
    }
    ByteArrayToHexString(outbuf,OutString,len);
    delete [] outbuf;
    return ret;
}

void SoftKey:: EncBySoft(   BYTE  *   aData,  BYTE   *   aKey   )
{
    const   unsigned   long   cnDelta   =   0x9E3779B9;
    register   unsigned   long   y   =   (   (   unsigned   long   *   )aData   )[0],   z   =   (   (   unsigned   long   *   )aData   )[1];
    register   unsigned   long   sum   =   0;
    unsigned   long   a   =   (   (   unsigned   long   *   )aKey   )[0],   b   =   (   (   unsigned   long   *   )aKey   )[1];
    unsigned   long   c   =   (   (   unsigned   long   *   )aKey   )[2],   d   =   (   (   unsigned   long   *   )aKey   )[3];
    int   n   =   32;

    while   (   n--   >   0   )
    {
        sum   +=   cnDelta;
        y   +=   ((   z   <<   4   )   +   a )  ^   (z   +   sum )  ^  ( (   z   >>   5   )   +   b);

        z   +=   ((   y   <<   4   )   +   c )  ^   (y   +   sum )  ^  ( (   y   >>   5   )   +   d);
    }
    (   (   unsigned   long   *   )aData   )[0]   =   y;
    (   (   unsigned   long   *   )aData   )[1]   =   z;
}

void SoftKey:: DecBySoft(    BYTE  *   aData,   BYTE   *   aKey   )
{
    const   unsigned   long   cnDelta   =   0x9E3779B9;
    register   unsigned   long   y   =   (   (   unsigned   long   *   )aData   )[0],   z   =   (   (   unsigned   long   *   )aData   )[1];
    register   unsigned   long   sum   =   0xC6EF3720;
    unsigned   long   a   =   (   (   unsigned   long   *   )aKey   )[0],   b   =   (   (   unsigned   long   *   )aKey   )[1];
    unsigned   long   c   =   (   (   unsigned   long   *   )aKey   )[2],   d   =   (   (   unsigned   long   *   )aKey   )[3];
    int   n   =   32;
    while   (   n--   >   0   )
    {
        z   -=  ( (   y   <<   4   )   +   c )  ^  ( y   +   sum  ) ^ (  (   y   >>   5   )   +   d);
        y   -=  ( (   z   <<   4   )   +   a )  ^  ( z   +   sum  ) ^ (  (   z   >>   5   )   +   b);
        sum   -=   cnDelta;
    }
    (   (   unsigned   long   *   )aData   )[0]   =   y;
    (   (   unsigned   long   *   )aData   )[1]   =   z;
}

int SoftKey::sWriteEx(  DWORD in_data,DWORD *out_data,char *Path)
{
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(WriteDword)(&in_data,Path);
    if(ret!=0)goto error1;
    ret=(ReadDword)(out_data,Path);
error1:
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

int SoftKey::sWrite_2Ex(  DWORD in_data,DWORD *out_data,char *Path)
{
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(WriteDword_2)(&in_data,Path);
    if(ret!=0)goto error1;
    ret=(ReadDword)(out_data,Path);
error1:
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

int SoftKey::ReSet(char *InPath)
{

    int ret;int Version;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(NT_GetVersion)(&Version,InPath);
    if(ret!=0)goto finish;
    ret=(NT_ReSet)(InPath);
    if(ret!=0)goto finish;
    ret=(NT_ReSet)(InPath);
    if(ret!=0)goto finish;
    ret=(NT_ReSet)(InPath);
    if(ret!=0)goto finish;
    ret=(NT_ReSet)(InPath);
finish:
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;

}

 int SoftKey::NT_GetVersionEx(int *version,char *InPath )
{
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(F_GetVerEx)(version,InPath);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

int SoftKey::SetCal_New(char *Key,char *InPath)
{
    BYTE KeyBuf[16];
    memset(KeyBuf,0,16);
    HexStringToByteArrayEx(Key,KeyBuf);
    //注意，这里是地址互换的
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(NT_SetCal_New)(&KeyBuf[8],0,InPath);
    if(ret!=0)goto error1;
    ret=(NT_SetCal_New)(&KeyBuf[0],1,InPath);
error1:
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;

}

int SoftKey::Cal_New(  BYTE *InBuf,BYTE *OutBuf,char *Path)
{
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=NT_Cal_New(InBuf,OutBuf,Path);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

int SoftKey::EncString_New(  char *InString,char *OutString,char *Path)
{

    int ret;int n;BYTE *outbuf;
    int len=lstrlenA(InString)+1;if(len<8)len=8;
    outbuf=new BYTE[len];
    memset(outbuf,0,len);
    memcpy(outbuf,InString,lstrlenA(InString)+1);
    for(n=0;n<=(len-8);n=n+8)
    {
        ret=Cal_New((BYTE *)&outbuf[n],&outbuf[n],Path);
        if(ret!=0){delete [] outbuf;return ret;}
    }
    ByteArrayToHexString(outbuf,OutString,len);
    delete [] outbuf;
    return ret;
}

void SoftKey::SwitchByte2Char(char *outstring,BYTE *inbyte,int inlen)
{
   int n;char temp[3];
   memset(outstring,0,ECC_MAXLEN*2);
   for(n=0;n<inlen;n++)
   {
        wsprintfA(temp,"%02X",inbyte[n]);
        lstrcatA(outstring,temp);
   }

}

void SoftKey::SwitchChar2Byte(char *instring,BYTE *outbyte)
{
   int n;char temp[3];
   int inlen=lstrlenA(instring)/2;
   for(n=0;n<inlen;n++)
   {
       temp[2]=(char)0;
       lstrcpynA(temp,&instring[n*2],2+1);
        *outbyte=HexToInt(temp);
        outbyte++;
   }

}

int SoftKey::YT_GenKeyPair(char* PriKey,char *PubKeyX,char *PubKeyY,char *InPath)
{

    int ret;BYTE b_PriKey[ECC_MAXLEN],b_PubKey[ECC_MAXLEN*2+1];//其中第一个字节是标志位，忽略
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(NT_GenKeyPair)(b_PriKey,b_PubKey,InPath);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    memset(PriKey,0,2*ECC_MAXLEN+1);
    memset(PubKeyX,0,2*ECC_MAXLEN+1);
    memset(PubKeyY,0,2*ECC_MAXLEN+1);
    SwitchByte2Char(PriKey,b_PriKey,ECC_MAXLEN);
    SwitchByte2Char(PubKeyX,&b_PubKey[1],ECC_MAXLEN);
    SwitchByte2Char(PubKeyY,&b_PubKey[1+ECC_MAXLEN],ECC_MAXLEN);
    return ret;

}

int SoftKey::Set_SM2_KeyPair(char *PriKey,char *PubKeyX,char *PubKeyY,char *sm2UserName,char *InPath )
{

    int ret;BYTE b_PriKey[ECC_MAXLEN],b_PubKeyX[ECC_MAXLEN],b_PubKeyY[ECC_MAXLEN];
    SwitchChar2Byte(PriKey,b_PriKey);
    SwitchChar2Byte(PubKeyX,b_PubKeyX);
    SwitchChar2Byte(PubKeyY,b_PubKeyY);
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(NT_Set_SM2_KeyPair)(b_PriKey,b_PubKeyX,b_PubKeyY,sm2UserName,InPath);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;

}

int SoftKey::Get_SM2_PubKey(char *PubKeyX,char *PubKeyY,char *sm2UserName,char *InPath)
{

    int ret;BYTE b_PubKeyX[ECC_MAXLEN],b_PubKeyY[ECC_MAXLEN];
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(NT_Get_SM2_PubKey)(b_PubKeyX,b_PubKeyY,sm2UserName,InPath);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    memset(PubKeyX,0,2*ECC_MAXLEN+1);
    memset(PubKeyY,0,2*ECC_MAXLEN+1);
    SwitchByte2Char(PubKeyX,b_PubKeyX,ECC_MAXLEN);
    SwitchByte2Char(PubKeyY,b_PubKeyY,ECC_MAXLEN);
    return ret;

}

int SoftKey::SM2_EncBuf(BYTE *InBuf,BYTE *OutBuf,int inlen,char *InPath)
{

    int ret,temp_inlen;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    while(inlen>0)
    {
        if(inlen>MAX_ENCLEN)
            temp_inlen=MAX_ENCLEN;
        else
            temp_inlen=inlen;
        ret=(NT_SM2_Enc)(InBuf,OutBuf,temp_inlen,InPath);
        if(ret!=0)goto err;
        inlen=inlen-MAX_ENCLEN;
        InBuf=InBuf+MAX_ENCLEN;
        OutBuf=OutBuf+MAX_DECLEN;
    }
err:
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;

}

int SoftKey::SM2_DecBuf(BYTE *InBuf,BYTE *OutBuf,int inlen,char* pin,char *InPath)
{

    int ret,temp_inlen;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    while(inlen>0)
    {
        if(inlen>MAX_DECLEN)
            temp_inlen=MAX_DECLEN;
        else
            temp_inlen=inlen;
        ret=(NT_SM2_Dec)(InBuf,OutBuf,temp_inlen,pin,InPath);
        if(ret!=0)goto err;
        inlen=inlen-MAX_DECLEN;
        InBuf=InBuf+MAX_DECLEN;
        OutBuf=OutBuf+MAX_ENCLEN;
    }
err:
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;

}

int SoftKey::SM2_EncString(char *InString,char *OutString,char *InPath)
{

    int inlen=lstrlenA(InString)+1;
    int outlen=(inlen/(MAX_ENCLEN)+1)*SM2_ADDBYTE+inlen;
    BYTE *OutBuf=new BYTE[outlen];
    BYTE *p=OutBuf;
    int ret,temp_inlen;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    while(inlen>0)
    {
        if(inlen>MAX_ENCLEN)
            temp_inlen=MAX_ENCLEN;
        else
            temp_inlen=inlen;
        ret=(NT_SM2_Enc)((BYTE *)InString,OutBuf,temp_inlen,InPath);
        if(ret!=0)goto err;
        inlen=inlen-MAX_ENCLEN;
        InString=InString+MAX_ENCLEN;
        OutBuf=OutBuf+MAX_DECLEN;
    }
err:
    memset(OutString,0,2*outlen+1);
    ByteArrayToHexString(p,OutString,outlen);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    delete [] p;
    return ret;

}

int SoftKey::SM2_DecString(char *InString,char *OutString,char* pin,char *InPath)
{

    int inlen=lstrlenA(InString)/2;
    BYTE *InByte=new BYTE[inlen];
    BYTE *p=InByte;
    int ret,temp_inlen;
    SwitchChar2Byte(InString,InByte);
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    while(inlen>0)
    {
        if(inlen>MAX_DECLEN)
            temp_inlen=MAX_DECLEN;
        else
            temp_inlen=inlen;
        ret=(NT_SM2_Dec)((BYTE *)InByte,(BYTE *)OutString,temp_inlen,pin,InPath);
        if(ret!=0)goto err;
        inlen=inlen-MAX_DECLEN;
        InByte=InByte+MAX_DECLEN;
        OutString=OutString+MAX_ENCLEN;
    }
err:
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    delete [] p;
    return ret;

}

int SoftKey::YtSetPin(char *old_pin,char *new_pin,char *InPath )
{
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(NT_Set_Pin)(old_pin,new_pin,InPath);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

void SoftKey::SwitchBigInteger2Byte(char *instring,BYTE *outbyte,int *outlen)
{
    int n;char temp[3];
    int inlen=lstrlenA(instring)/2;
    *outlen=0x20;

    for(n=0;n<inlen;n++)
    {
        temp[2]=(char)0;
        lstrcpynA(temp,&instring[n*2],2+1);
        *outbyte=(BYTE)HexToInt(temp);
        outbyte++;
    }

}


int SoftKey::GetChipID( char *OutChipID,char *InPath)
{
    int ret;BYTE b_OutChipID[16];
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(NT_GetChipID)(b_OutChipID,InPath);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    memset(OutChipID,0,33);
    ByteArrayToHexString(b_OutChipID,OutChipID,16);
    return ret;
}

int SoftKey::SetHidOnly(  BOOL IsHidOnly,char *InPath)
{
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=NT_SetHidOnly(IsHidOnly,InPath);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}
int SoftKey::SetUReadOnly( char *InPath)
{
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=NT_SetUReadOnly(InPath);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

int SoftKey::ReadVerfData(char *InPath)
{
    BYTE dataBuffer[50];HANDLE fileHandle;
    SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
    ULONG length = 0,returned = 0;
    BOOL status = 0;
    char DevicePath[20];
    lstrcpyA(DevicePath,"\\\\.\\");lstrcatA(DevicePath,InPath);
    fileHandle = CreateFileA(DevicePath,GENERIC_WRITE | GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING, 0, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE)return OPENUSBFAIL;
    ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
    ZeroMemory(dataBuffer, DISK_INF_LEN);
    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = 6;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
    sptdwb.sptd.SenseInfoLength = 24;
    sptdwb.sptd.DataTransferLength = DISK_INF_LEN;
    sptdwb.sptd.TimeOutValue = 100;
    sptdwb.sptd.DataBuffer = dataBuffer;
    sptdwb.sptd.SenseInfoOffset =
        offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
    sptdwb.sptd.Cdb[0] = 0x12;
    sptdwb.sptd.Cdb[4] = DISK_INF_LEN;                         // 命令
    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);

    status = DeviceIoControl(fileHandle,
        IOCTL_SCSI_PASS_THROUGH_DIRECT,
        &sptdwb,
        length,
        &sptdwb,
        length,
        &returned,
        FALSE);

    CloseHandle(fileHandle);
    if((sptdwb.sptd.ScsiStatus == 0) && (status != 0) )
    {
        if(memcmp(dataBuffer,DISK_INF,DISK_INF_LEN)==0)
        {
            //MessageBox(NULL,DevicePath,"OK",MB_OK);
            return 0;
        }
    }
    return OPENUSBFAIL;
}

int SoftKey::sFindPort(short pos,char *OutPath)
{
    DWORD   dwBuffer=512*2;   char cdriver[10]="";int  nSize;int len=1;//char temp[30]="";
    char   szBuffer[512*2];
    int count=0;
    DWORD lpSectorsPerCluster;
    DWORD lpBytesPerSector;
    DWORD lpNumberOfFreeClusters;
    DWORD lpTotalNumberOfClusters;
    UINT   nPreErrorMode;

    //ADD 2008-4-5
    ZeroMemory(OutPath,sizeof(OutPath));
    {
        //return isfindmydevice(pos,OutPath,count);
        ZeroMemory(   szBuffer,    dwBuffer   );
        nSize   =   GetLogicalDriveStringsA(   dwBuffer,   szBuffer   );
        while(nSize>=4)
        {
            nSize=nSize-4;
            lstrcpyA(cdriver,&szBuffer[nSize]);
            len=lstrlenA(cdriver);
            cdriver[len-1]=0;
            if((cdriver[0]=='a') || (cdriver[0]=='A') || (cdriver[0]=='b') || (cdriver[0]=='B'))continue;
            if((GetDriveTypeA(cdriver)==DRIVE_CDROM) || (GetDriveTypeA(cdriver)==DRIVE_REMOTE) ||
                (GetDriveTypeA(cdriver)==DRIVE_UNKNOWN))continue;
            //#ifdef _CHECKDISK
            /*add-2010-6-13是否需要这个，注意如果容量改变，这个要改变	*/
            nPreErrorMode   =   ::SetErrorMode(SEM_FAILCRITICALERRORS)   ;
            GetDiskFreeSpaceA(cdriver,&lpSectorsPerCluster,&lpBytesPerSector,&lpNumberOfFreeClusters,&lpTotalNumberOfClusters);
            SetErrorMode(nPreErrorMode) ;
            if(lpTotalNumberOfClusters!=344|| lpSectorsPerCluster!=1 )continue;

            //#endif
            if(ReadVerfData(cdriver)==0)
            {

                if(pos==count)
                {
                    lstrcpyA(OutPath,"\\\\.\\");
                    lstrcatA(OutPath,cdriver);
                    return 0;

                }
                (count)++;
            }
        }
    }
    return NOUSBKEY;
}

int SoftKey::FindU(int pos,char *OutPath)
{
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    int ret=sFindPort(pos,OutPath);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

int  SoftKey::NT_FindU_2(  int start,DWORD in_data,DWORD verf_data,char*OutPath )
{
    int pos;DWORD out_data;int ret;
    for(pos=start;pos<127;pos++)
    {
        if(sFindPort(pos,OutPath)!=0){return -92;}
        ret=WriteDword( &in_data,OutPath);
        if(ret!=0){return ret;}
        ret=ReadDword( &out_data,OutPath);
        if(ret!=0){return ret;}
        if(out_data==verf_data){return 0;}
    }
    ZeroMemory(OutPath,sizeof(OutPath));
    return -92;
}

int SoftKey::FindU_2(  int start,DWORD in_data,DWORD verf_data,char*OutPath)
{
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(NT_FindU_2)(start,in_data,verf_data,OutPath);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

int SoftKey::NT_IsUReadOnly(BOOL *IsReadOnly,char *InPath)
{
    BYTE dataBuffer[50];HANDLE fileHandle;
    SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
    ULONG length = 0,returned = 0;
    BOOL status = 0;
    char DevicePath[20];
    lstrcpyA(DevicePath,"\\\\.\\");lstrcatA(DevicePath,InPath);
    fileHandle = CreateFileA(DevicePath,GENERIC_WRITE | GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING, 0, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE)return OPENUSBFAIL;
    ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
    ZeroMemory(dataBuffer, DISK_SENSE_LEN);
    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = 6;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
    sptdwb.sptd.SenseInfoLength = 24;
    sptdwb.sptd.DataTransferLength = DISK_SENSE_LEN;
    sptdwb.sptd.TimeOutValue = 100;
    sptdwb.sptd.DataBuffer = dataBuffer;
    sptdwb.sptd.SenseInfoOffset =
        offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
    sptdwb.sptd.Cdb[0] = 0x1a;
    sptdwb.sptd.Cdb[2] = SCSI_MSPGCD_RETALL;
    sptdwb.sptd.Cdb[4] = DISK_SENSE_LEN;                         // 命令
    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);

    status = DeviceIoControl(fileHandle,
        IOCTL_SCSI_PASS_THROUGH_DIRECT,
        &sptdwb,
        length,
        &sptdwb,
        length,
        &returned,
        FALSE);

    CloseHandle(fileHandle);
    if((sptdwb.sptd.ScsiStatus == 0) && (status != 0) )
    {
        if(dataBuffer[2]==0x80)
        {
            *IsReadOnly=TRUE;
        }
        else
            *IsReadOnly=FALSE;
        return 0;
    }
    return OPENUSBFAIL;
}

int SoftKey::IsUReadOnly(BOOL *IsReadOnly,char *InPath)
{
    int ret;
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=NT_IsUReadOnly(IsReadOnly,InPath);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;
}

void SoftKey::EnCode(   BYTE  * InData,BYTE  * OutData,  char *Key  )
{
    BYTE KeyBuf[16];
    memset(KeyBuf,0,16);
    HexStringToByteArrayEx(Key,KeyBuf);
    memcpy(OutData,InData,8);
    EncBySoft(OutData,KeyBuf);
}

void SoftKey::DeCode(   BYTE  * InData, BYTE  * OutData, char *Key  )
{
    BYTE KeyBuf[16];
    memset(KeyBuf,0,16);
    HexStringToByteArrayEx(Key,KeyBuf);
    memcpy(OutData,InData,8);
    DecBySoft(OutData,KeyBuf);
}

void SoftKey::StrDec(  char *InString,char *OutString,char *Key)
{
  int n;
  int len=lstrlenA(InString)/2;
  BYTE KeyBuf[16];
  memset(KeyBuf,0,16);
  HexStringToByteArrayEx(Key,KeyBuf);
  HexStringToByteArray(InString,(BYTE *)OutString);
  for(n=0;n<=(len-8);n=n+8)
    {
      DecBySoft((BYTE *)&OutString[n],KeyBuf);
    }
  return ;
}

void SoftKey::StrEnc(  char *InString,char *OutString,char *Key)
{
  int n;BYTE *outbuf;
  int len=lstrlenA(InString)+1;if(len<8)len=8;
  BYTE KeyBuf[16];
  memset(KeyBuf,0,16);
  HexStringToByteArrayEx(Key,KeyBuf);
  outbuf=new BYTE[len];
  memset(outbuf,0,len);
  memcpy(outbuf,InString,lstrlenA(InString)+1);
  for(n=0;n<=(len-8);n=n+8)
    {
      EncBySoft(&outbuf[n],KeyBuf);
    }
  ByteArrayToHexString(outbuf,OutString,len);
  delete [] outbuf;
}

int SoftKey::GetProduceDate(  char *OutDate,char *InPath)
{
    int ret;
    BYTE B_OutBDate[8];
    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=(NT_GetProduceDate)(B_OutBDate,InPath);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    ByteArrayToHexString(B_OutBDate,OutDate, 8);
    return ret;
}

int SoftKey::SetID(char * Seed,char *InPath)
{

    int ret;
    BYTE KeyBuf[8];
    memset(KeyBuf,0,8);
    HexStringToByteArray(Seed,KeyBuf);

    HANDLE hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");
    WaitForSingleObject(hsignal,INFINITE);
    ret=NT_SetID(KeyBuf,InPath);
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;

}

int SoftKey::Y_SetCal(BYTE *InData,short address,short len,BYTE *password,char *Path )
{
       short addr_l,addr_h;int n;int ret;
   BYTE array_in[25],array_out[25];
   HANDLE hUsbDevice=MyOpenPath(Path);
    if(hUsbDevice == INVALID_HANDLE_VALUE)return NOUSBKEY;

   if(len>8)return -87;
   if(address+len-1>2047 || address<0)return ErrAddrOver;
   {addr_h=(address>>8)<<1;addr_l=address&0xff;}

   array_in[1]=SETCAL;//read 0x10;write 0x11;
   array_in[2]=(BYTE)addr_h;//0x01;//read 0x80 ;write 0x40;
   array_in[3]=(BYTE)addr_l;
   array_in[4]=(BYTE )len;
   for(n=0;n<8;n++)
   {
       array_in[5+n]=*password;
       password++;
   }
   for(n=0;n<len;n++)
   {
       array_in[13+n]=*InData;
       InData++;
   }
   ret= Hanldetransfe(hUsbDevice,array_in,13+len,array_out,1,Path);
   if(ret!=0)return ret;

   if(array_out[0]!=0x0)
   {
       return ErrWritePWD;//表示写失败；
   }
   return 0;
}


int  SoftKey::SetCal(char * W_HKey,char *W_LKey,char * new_HKey,char *new_LKey,char *InPath)
{

    BYTE ary1[8];BYTE ary2[8];	int ret;short address=0x7f0-8;

    myconvert(W_HKey,W_LKey,ary1);
    myconvert(new_HKey,new_LKey,ary2);

    HANDLE	hsignal=CreateSemaphoreA(NULL,1,1,"ex_sim");

    WaitForSingleObject(hsignal,INFINITE);
    ret=Y_SetCal(ary2,address,8,ary1,InPath);
    if(ret!=0){ReleaseSemaphore(hsignal,1,NULL);return ret;}
    ReleaseSemaphore(hsignal,1,NULL);
    CloseHandle(hsignal);
    return ret;

}

void  SoftKey::SnToProduceDate(char* InSn,char *OutProduceDate)
{

   char temp[5]="";
   memset(temp,0,5);
   lstrcpynA(temp,&InSn[0],2+1);
   int year=2000 + HexToInt(temp);
   lstrcpynA(temp,&InSn[2],2+1);
   int month=  HexToInt(temp);
   lstrcpynA(temp,&InSn[4],2+1);
   int day=  HexToInt(temp);
   lstrcpynA(temp,&InSn[6],2+1);
   int hour=  HexToInt(temp);
   lstrcpynA(temp,&InSn[8],2+1);
   int minutes=  HexToInt(temp);
   lstrcpynA(temp,&InSn[10],2+1);
   int sec=  HexToInt(temp);
   lstrcpynA(temp,&InSn[12],4+1);
   int sn=  HexToInt(temp);
    wsprintfA(OutProduceDate,"%d年%d月%d日%d时%d分%d秒--序号：%d", year,month,day,hour,minutes,sec,sn);
}

//使用普通算法一查找指定的加密锁
int SoftKey::CheckKeyByFindort_2()
{
	char DevicePath[260];
    return FindPort_2(0, 1, 1491343602, DevicePath);
}

//使用带长度的方法从指定的地址读取字符串
int SoftKey::ReadStringEx(int addr,char *outstring,char *DevicePath)
{
	int ret;
    short nlen;
    BYTE buf[1];
    //先从地址0读到以前写入的字符串的长度
    ret = YReadEx(buf, addr, 1, "3AD93F82", "F987317C", DevicePath);
    nlen = buf[0];
    if( ret != 0 )
    {
        return ret;
    }
	memset(outstring,0,nlen+1);//outstring[nlen]=0;//将最后一个字符设置为0，即结束字符串
    //再读取相应长度的字符串
    ret = YReadString(outstring, addr+1, nlen, "3AD93F82", "F987317C", DevicePath);
    return ret;

}

 //使用从储存器读取相应数据的方式检查是否存在指定的加密锁
int SoftKey::CheckKeyByReadEprom()
{
	int n,ret;
	char DevicePath[260];
	char outstring[260];
	//@NoUseCode_data return 1;//如果没有使用这个功能，直接返回1
	for(n=0;n<255;n++)
	{
		ret=FindPort(n,DevicePath);
		if(ret!=0)return ret;
		ret=ReadStringEx(0,outstring,DevicePath);
		if((ret==0) && (memcmp(outstring,"css",lstrlen(L"css"))==0))return 0;
	}
	return -92;
}

//使用增强算法一检查加密锁，这个方法可以有效地防止仿真
int SoftKey::CheckKeyByEncstring()
{
	//推荐加密方案：生成随机数，让锁做加密运算，同时在程序中端使用代码做同样的加密运算，然后进行比较判断。
	
	int n,ret;
	char DevicePath[260];
	char InString[20];
	
	//@NoUseKeyEx return 1;//如果没有使用这个功能，直接返回1

	srand((unsigned)time(NULL)); 
	sprintf(InString,"%X%X",rand(),rand());

	for(n=0;n<255;n++)
	{
		ret=FindPort(n,DevicePath);
		if(ret!=0)return ret;
		if(Sub_CheckKeyByEncstring(InString,DevicePath)==0)return 0;
	}
	return -92;
}

int SoftKey::Sub_CheckKeyByEncstring(char *InString,char *DevicePath)
{
	//'使用增强算法对字符串进行加密
    int ret,nlen;
    char *outstring;
    nlen = strlen(InString) + 1;
    if( nlen < 8 ) nlen = 8; 
    outstring = new char[nlen * 2+1];//注意，这里要加1一个长度，用于储存结束学符串
    ret = EncString(InString, outstring, DevicePath);
    if (ret != 0) {  return ret;}
    char *outstring_2;
    outstring_2 = new char[nlen * 2+1];//注意，这里要加1一个长度，用于储存结束学符串
    StrEnc(InString,outstring_2, "2C2FFBCCB73038869EFC6AE9356D3869");
	if(memcmp(outstring,outstring_2,strlen(outstring_2))==0)//比较结果是否相符
		ret=0;
	else
		ret=-92;
	delete [] outstring_2;delete [] outstring;
	return ret;

}

//使用增强算法二检查是否存在对应的加密锁
int SoftKey::CheckKeyByEncstring_New()
{
	int n,ret;
	char DevicePath[260];
	char outstring[260];
	int myrnd;
	char EncInString[][50]={"13938","7644","29548","1873","26080","29559","26338","28696","532","15174","23100","21935","12885","10091","16151","19274","1294","13479","10026","953","10698","31837","20048","13166","19249","9871","17539","4733","2319","31710",
"14302","32405","4685","6029","24217","15626","24308","14717","272","23758","2608","14385","9148","14451","19530","4811","20790","6187","5293","8973","1083","29638","17377","28503","12706","21689","1426","7342","29285","22548",
"19080","179","19873","20617","25262","21414","8846","2156","14095","620","12934","9133","9173","8325","19202","8671","29255","31251","3999","51","29394","17881","25619","31611","14080","30521","30267","29227","27513","12341",
"26640","26195","21001","21358","6573","8102","23449","13863","4939","28021","25138","29249","3908","2228","769","22591","9233","5619","6221","324","3662","18275","21377","271","6836","7898","30503","14339","12317","20422",
"12522","17700","31107","16699","5011","6162","12289","12267","12555","8976","1232","28711","21598","11520","11081","20450","23477","17250","744","6318","12890","19547","24902","12732","27319","14508","31772","25055","30806","21941",
"29511","12681","2531","16251","31422","27850","804","29342","9373","17751","5894","32353","19228","5249","402","22181","6807","15673","25073","17902","15239","11793","13423","32603","7302","829","27765","20656","23629","18572",
"11918","29775","21433","23411","10851","827","16321","27906","5309","1811","2945","9196","16776","11290","14897","24194","24012","4916","28972","5304","18870","17377","8686","23069","4749","15939","21517","32741","3260","11550",
"3911","22242","3534","14290","25970","4486","18027","29793","7308","17890","15454","31086","30892","4009","25746","4156","9742","21036","23824","1706","17162","6750","18178","8508","2589","8401","16562","11540","7134","27573",
"32650","12787","26887","4320","2581","17468","15476","6080","4603","30448","25886","31593","28167","28082","33","19597","13348","3781","25266","29565","29531","21768","9265","4653","3543","16895","18451","1580","13023","28619",
"2037","2735","30944","17200","15760","31365","5988","15758","27838","12835","16659","28391","28025","4505","7609","2015","13788","7834","25662","2998","17439","18039","5295","8782","10398","16419","21042","2602","12242","3632",
"23828","10860","11349","8399","11455","17621","24801","6099","10683","18376","2640","9389","1972","16454","2461","3185","16242","27634","11744","18894","10465","27406","7765","20248","13578","4732","25684","26689","18455","23997",
"5465","15579","13138","244","7421","29207","20261","6882","9978","27848","13595","1770","10538","13152","6620","20237","3464","28656","18228","15855","31787","4651","13835","4304","17745","4450","18541","18834","27473","20104",
"9091","8376","19623","5679","10647","5988","28140","17032","5140","10583","25562","32101","11650","7924","23883","8763","11148","32572","19770","16407","10493","8659","13503","15279","16318","24824","3350","4436","17112","27536",
"20887","14071","6264","14107","10687","7723","13285","4540","13092","22048","12170","21556","27523","19883","1778","13286","15030","13629","28868","13524","7349","232","22734","15263","27547","2652","31223","16266","18851","21229",
"6220","7286","11046","25997","23284","1255","158","22734","17978","5116","165","28897","21896","32633","4596","24985","13673","27072","21863","12392","7557","21458","20471","13919","19111","12115","13503","16944","6331","4227",
"20215","23087","15559","69","3068","7756","7928","28185","9540","18969","21655","16932","27244","22217","14862","3879","11053","15663","11496","25730","16737","22458","26061","16442","15106","25726","29320","8431","9259","10029",
"20484","14308","28948","11032","9291","15098","2073","9523","6277","22196","27570","30221","21244","16436","1148","19418","4381","26427","2862","25708"};
	char EncOutString[][50]={"AD35A57C26091908","3F469AAF466A649A","9CC506D82F79315A","8C7657A484912BCD","1A1F05979D34717B","E617F779C99489D4","D977C2C7EBFCF1E5","B65DD692BC369109","9FC77B75583C6990","F993879F318981AD","1317A710887FD78D","B6D6B3D6C9917EB2","737C565D96E3B62C","3C414B7C436E96AB","9B1F8CB278DCCA05","E975085B104F279F","689935C342E8D849","90F77137C4603E73","D204E0EB98B34DDA","4CFA78536AD5C0A0","DBFF3763F7747050","1D711E622C238C33","FD8FFD1F5F74C577","3D4FA244351A3EDE","5C55279970E11435","3D7D02C5536C0799","AEA595365338CB16","05236089F23DF02B","A6047AF1826108BC","4C6B1DCF64F98CF5",
"32F1C8A907BF669E","7170304C8A3FE050","83D83139C10BB982","7EE37C994D8934E5","2517BD0AE8011E60","C26598F6F07F1AE0","43024EEE8B93AE63","E8507AE63C3F216D","3A12CC82B2A3F5D7","018F7AF657A4564F","E809EF3330BFA4AC","58E49C610DB3A6B9","FB30A63DF4420E92","9AD99777C78B1797","C08193CEB8A89F67","A64726CBE982D1CC","E5FA5F7E47EE3BA9","C3F721E40CC0C1F0","F23EA26D1989DE95","552CDD69B52581C5","CE9BDFBEFA012D67","73B31E88AAC5CFB0","C21C065371B5E3ED","E61D8C7BF1DD48EB","677CB678D47542CE","69EEC7D00CD12B41","9D65368CBE223D14","34F16EF86F548B98","7F72F604A0797CEB","9FB0198F4298E221",
"5CF885AA5FF6C7E7","E9C907FBB2E1532C","8F911E181FBC7641","73CB8FDB485957F4","B21EDD35F0E89C6C","50ED81751C383BE1","0719EBD7560E10C4","1FC4B08829A0BB1A","A9ADBBF159FD7876","5B93BD55C807278D","940851B51B64D538","EBA207AD0F3F3C0E","88492F997FD881A9","7621B17E451D221D","C02C9678A10D8B67","6BAA48189F8657E3","751A8DF9A177505B","9742BDE6495FDE3B","449F5C62FA76897D","28F3C6562EF53F77","EA71A578A3FC781F","76653DDE5BBD1AD0","62B06BDE40C57B8A","ED9C01422F7DE819","BC68BD096A335C99","2A24E527CFC7374F","911BD2DD64C50B0E","7D40A8F8958D0D60","891619AEBA5A9180","27632D011F1E8828",
"54781F60E4D16D31","330172C247E41312","101F44F081D86720","664E013600F96AB4","4717DE8CBB0CCB46","D8B7F042837ADD53","1FAE4DD5D098A28C","F3FA7FB494783732","5C9EB0B19B72DCD7","40AA56D26F0D88AF","0549C568A26D37A2","EEF405BE9DD24D0D","80675F4C8E72C416","F1BAAD5E1646CBF0","040877A263B04D42","6408392B82496106","C75AAA6A594F52E4","0D7DD5D5D4D07993","714515EFD2513174","EE69FCF2647FEBF0","85B533AECDF97C60","E416A640D8A27AA9","ACE0028C6D4745CF","309AB10A50273E5F","8F9D58FD1C7F6240","05342C61141ED45E","7BD6062D8F07E46B","EE304F355829A8BA","4C1CD53340D3179C","28D9C75F48731F2D",
"9CF89250ACAE5C39","04100D7BA4B06482","0E78AE037102F9C4","8818632808AC6265","0AE661F8306B1167","F72FB126456EEFF0","EA81F45F2F601E04","41355020E809C38E","2B06EB275DEF6CC7","633F2DA6D706DFCF","3666958DEABDA2CF","0E49FDBE6FBEACCE","9D7BE013ABB1FA85","E33E874A1DEBD75D","4C5ED29582FA7670","6C2E7F8261318B03","0430076B549C00CE","DE4F9CC4250752CB","4679B3160C26FB75","810E04994929EB6A","677474978B65F7D3","E99DAE0C5B822D48","4EFE585E91803A2D","5369F67406D57FCD","B6348100D68F55B1","349E924F5085B52B","F2FAAFCD2C969607","0F18CE3D70BC5489","B82491314D12D327","0B051439FC504337",
"E6BF5E85FE4D9E07","0D58BC0048EC9894","036E5BEFC02D4519","5C8E28B395C82A71","74259EFF2B665ACB","23EE77229F420526","C2AF54E7CB102F5A","7C8D06466416C19A","7640A26CAF892AB2","470DC0021F352F33","D68664564CC47D54","98E92EE32A8468AC","32AD43B2DD4110CF","F87E46748051EFC6","3C842D65C526A04B","7ADB711F0C6472EC","2EEF6A904F633034","D8C4397D98C8A236","CDFAB76C54F46983","BFA3C55088884571","EB6E2B74130501DA","C3F2F57123D0D326","03D3A4330198A6E4","9F01BFA47A9B1786","03A9037A26F96B86","3246800932C1B062","5C41730B4EBAA9B3","F0646C9F57D45B3F","75857626D491B2E3","F72D6B34AA437C41",
"1878EB8D0B8E1B4B","AAB813AF9FABD771","258F3EF44F3D249E","B96D4FB11D1B51DA","47AA1A021AF360B4","2E0FF285697FD2E5","1E03C053842CC0CA","FD9EC5B78F1EE306","CDDFE016AA4B0015","BA2BF93B1967BE10","D94AD8D06CA34546","8CD7C9F9F9E5E73B","D20659E7E1AB72EA","F2087EAE2F15031C","7A0F7889F73164D2","691865ACC17E9A9D","79E5259902F9136E","CCCA12F127344CDB","4CA41FDCB7FADE1B","29E5EBA3AB47A66A","CD6962DD48228C54","C21C065371B5E3ED","D253C3A243E5104A","44592B0578D4D882","2CB595A45C91D606","9BE1958886614867","EB334A0D0573FCCD","90F55937498200BC","1D51A896DADF35DD","FEC0ED1DB446AC2F",
"877BDB5B76511530","6CB35771CBB1D91E","2419864A94FC9925","EDC772A926E52EDC","F28C234D7434090F","EEB33FAABC872E20","053B26BFEF195D6F","A1C7EE7A6DA190A2","A512D5A2E35BD443","140F9023E9E32320","BB162BEAE62B1272","6455154373C7323E","3BC868C7991984AA","0D6687295B8FCC38","EA3A16C64FD9FD85","04CEA1E60DCA61DA","E504299028FCECD1","2FA72FAA10394BCC","D3139ECF4DB313DE","F7CAA29799307725","A217825C7BDEEA59","091CF8F2F5944235","200A5645A9CA87F8","57BC1532D985B8A6","10C135EEE3065B35","2CB7CF61F183926C","D49ECF14861B0342","9701213AACCF5803","76E673815E094B00","1723382AC3C5A771",
"3C1E7322EFF697A0","38E0251CEF7FD5B2","489D5C1839988184","8DD86D9ECBF84A94","BB1FF8D9F9A8BC6B","844074E6B69980ED","BD5A47CEB3A45D29","8DD84E598FAF3449","E7C839B8BDF8CDA2","27B92B350087CDD5","AA5C1BC00BCC4118","4D4ED71562F09422","B4FF51EDCFFECE60","038F4BE3F0279113","0CCA6F899EC2C89C","89968948AF8D2C33","B3E7B46DCE1B3027","042CF6AE64217715","2E7293383AE2FA88","61C70BC52BD12BAC","4CB33C4A3B897E98","A98A4125FF0B218B","BFDF097E17C0AF06","AA062278F79CD98B","60D1E6C1B0AE8482","0291B93A82511F47","F69191346E3A0328","FE1CC8CE3026228D","BE32203DBEA6ECA7","DCFC3E1ABB9FC3C2",
"6F51DCC6EA428F30","41A2B33329908DB5","8E997FA9A754E3A3","7C956A8E276E082A","C22EA7679ABBEC5F","48D5CD6C4D63929B","2E36F238F5096C8C","6593E49F234636FC","847B8779444318B3","F5E56C51886D3B1A","F5393A5498EFA3D6","2A355828DE9B411F","E17B7CD69C1E3E23","8990C744FA08A809","31274978D1261C26","A8EF52300A96B3A0","FE24FF9659D0AC15","086249FCF065D526","C06C049721A9E026","7125CDD8CC8C411A","0EF799DB650FBBB3","53C79DF687384FDA","9216169DEB235EAB","0819560ED6137B76","FF2EF67370272F0F","F5D083CDE61D8708","F032E389AAF81122","A141EDA12F6312FB","84E3E1E56C9190C4","C466B35BA42C59A2",
"5FB5833758C0918C","7C8E154AFB783E5A","F9149A905E16565D","7193BD03610FFA19","8D2CE8BDDA79F7C3","9CF8628019BEC76D","F0394D3984DDCFB4","1E1634BDCA706AD7","2CA5C6BFB0283484","446B6CDF474A857D","9DCB7FF52FA5F2C4","31CA85C71F466914","A78176A38B9CA49C","563C95877D38D9D3","4B354386DF70F438","FB8CC6E44682136A","9E86D6539FCCE20E","C58401DEA8ACC487","6FA87CCD6DAB85CB","4D6B1DC00844EEFE","527572AFE294D247","E6804543482AD702","FCF1FC5872DFE07B","79947CDDE6A87D8A","08A9649516399D6C","F43151DDBD7FA016","4542E7645441B346","A4549F3CCF0D1289","C1117F02B76A4261","2AC3EA7C71E9DF29",
"CFE1237263756CEA","E6224885A2912966","368DD4A3AB5BF72F","8925AFFB0BE26E96","56BD3F2E1EE7171F","8F5CDB85B43E5454","5DFE21C3D106A1E4","8937CD51F3BC5A80","314DB44663558E49","54347DE5EFDE3582","4D8368C7F05F50C0","76398B90CDA9818A","08022F79177CF8F8","89900EC185BE5A27","2B1E027144F95FEF","AA1A89A160E5DD47","7F649587B3C98EA6","A1D6B9561760799B","258DB9B8B88EE06F","4A086C46AEA13E18","62452AAAB4912F70","C4A76CC614307198","998FDA7BC920E157","22D58982530BF133","6576191A3C94CAB0","37F8CB5C01A5A4C2","AB2D363DCA427395","8E42D2F28810FC8A","9956526F95C80003","D524215932EFC463",
"5A94FBDB9A0DDF18","E63A6E6E8472CAC4","454AB763B8D13C7B","AF56AE58C33EA8F0","4EDBD239F4136105","2E36F238F5096C8C","7954E305C9C3A097","AC2580C835E08E86","12ADE4CD59ED9345","C5255C91D434F02E","F36B7B41BB436519","CD102D7CC4D57866","7B1D68EEECDDBA89","C171880A7791991B","BA98A18F9E7A1D8C","B4D353DF8EFE7832","064C0C840BB3D212","D0AF11FA99FA6F3C","18C1082171B81AE9","A431A47B3AF4405D","C6724A797AB636E9","18FD08EB0A6D5877","BA89854E27F75599","01B7532ECA847EF9","D2BA890757C0D3F6","7EB7FAFD2E3C56DA","F074EF680A1FC7EB","B2FED0834FA1C707","BD6B33F9EC9AC5D9","B31BA8B667958E60",
"5640D38AF684B1BC","46B718CBE68655CB","D06A561C812D8308","D5B19E58BF2B0A5C","225C424F2A17FAC9","8D58FC123D60C873","38AE06D43A5BCAB7","4FC3333DA9AD215C","6C581148ACC71823","B6EB33D8F62FBF06","82D7BB5D323266B6","3D74654AF89C25BD","7770C79B3FDD1E33","FDA1BD9F287CD9F0","365BA8541E179123","8E1EE79D54B67234","B942429CBB8D0001","FBC59D411566B875","3AF88C99E11BEDA0","CF52FC23ECE89BAA","178362DCE11A396A","932DA6AAC71B9AC4","C293B2ADCD61C80D","9292DFEEB8CF0298","18397EDFE1FF9428","C2F9C0AD8874AC1E","2D95FB8AFEE84472","35C69508107B58F6","775261DE7A53C6AC","E2D510285B51F5C4",
"731ABD2EC382AFCA","CDC2D9330A6FACBE","D85EBB8D6D09F87F","0D6AA1F28E95C317","C3604FE6734B3E94","A1E2674479145893","523653232F5594F6","C293B2ADCD61C80D","AACEDE1E3712269E","036C7D6603627191","DFD45A870FD10133","0F30B750B934B61E","6B7B0EBDF402E271","B8C5E553BAB5B714","342A207BFB5D9FD2","C2E6D0733C2BFCDB","11306F51FC8F9583","786C8C9ADF23C7E8","E9E43EB1DE340DB6","0BF81B79D26F033F","20CBA0D0D6BCD05D","D19525F0559AD4D2","706A7A7EA3565B2B","C6EC2C3A784E6342","5F591B02D6503358","F79CC5D1D17A2CC2","BA89854E27F75599","1A515353BC02332E","0677F7F601B2F87F","D5341CFE97D4DD3E",
"8E2C0CB9A734ADFF","A71E6E524D9A55FA","9654B38F6708E558","E188D077BC2FA9A1","09190CDC0DF931D7","1E55C29C2FA12ACC","198D855E19881361","2F16B14F52CE02BC","87D7F14EEAC85628","EAD94915D2A80A95","53F7E13F09E28D9B","4E21E6CCA4D02ABC","072DAEDD13D048C1","4AE279B2AE436ACC","897BD7E0A3F8E337","961DF99B2AAFF67A","4D39204969BBD615","A1A9E5BEC3E9C039","46601974505B6512","129B081AAA57C74A","97E6B2C998B34EA8","B4DC16C093213D1F","1BEB256E52DE3780","BD617EC6FD465CE8","B653E1A3F63E2C91","1D7A615B90D66CFA","88A5AEC847FB100D","293A2DAC0111BD6D","18B5C47A702BA0C3","95AD2BD0A1971C96",
"E1AFC3C721BA86C8","4BB67027A8B00DFF","9354128170EC19E9","4200D3457CC0E050","D1B7FFEF8D5A7E81","9DE52B5AF3D4E397","DC1AEE135A5F4001","0D1B7A2630C9F5FC","EFB8C9E2935DC354","F2F60A087E5F62B0","D0B147FCE0405B61","271CDFC445040959","AF2F24203B149C5F","AFCA330B988DC381","D0239D5F1D8B2F63","E142AB9D9564EE29","8D07634E8AA95D5C","D6499056D93E7FEB","A055DA099A41EB69","896C3CFE9AC3445B"};
	//@NoUseNewKeyEx return 1;//如果没有使用这个功能，直接返回1
	//@NoSupNewKeyEx return 2;//如果该锁不支持这个功能，直接返回2

	srand((unsigned)time(NULL)); 
	myrnd=rand()%500;
	for(n=0;n<255;n++)
	{
		ret=FindPort(n,DevicePath);
		if(ret!=0)return ret;
		ret=EncString_New(EncInString[myrnd], outstring,DevicePath);
		if(memcmp(outstring,EncOutString[myrnd],strlen(EncOutString[myrnd]))==0)return 0;
	}
	return -92;
}