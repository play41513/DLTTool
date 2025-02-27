//---------------------------------------------------------------------------


#pragma hdrstop

#include <windows.h>    // 安全移除USB裝置用 *要比 vcl.h 早編譯
#include "COM_control.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

extern DWORD WINAPI WORKTESTExecute(LPVOID Param);
AnsiString DeviceCOM;
DWORD 	   dwMAX_WATT;
DWORD 	   dwLoader_Type;


struct ComNum_Info {
HANDLE handle;
HANDLE Threadhandle;
AnsiString ComName;
};
struct ComNum_Info comNum_Info[10]; // COM資訊

cCOM::cCOM(void)
{
	dwLoader_Type	= 0;
	plReadValue 	= NULL;
	bCOM_PORT_OPEN 	= false;
}
void  cCOM::SetReadValueElement(TPanel* pl)
{
	plReadValue = pl;
}
DWORD cCOM::GetLoaderModel()
{
	return dwLoader_Type;
}
DWORD cCOM::cmd_get_system_consumptoin(char *s)
{
	que_Msg.push(GetTickCount());
	TStringList *sList = new TStringList();
	sList->Delimiter = '\r\n';
	sList->DelimitedText = AnsiString(s);
	que_Msg.push(AnsiString(s));
	double dCurrent,dVoltage;
	if(sList->Count >=2)
	{
		dCurrent = GetNumOfString(sList->Strings[0]);
		dVoltage = GetNumOfString(sList->Strings[1]);
	}
	else
	{
		delete  sList;
		return RS232_WRITE;
	}
	delete  sList;
	 //過載不加電壓補償 先以此方法判斷
	if(maxLoadCurr > 1)
		dVoltage = dVoltage+dbLossVol;
	//====================
	astrVoltage = AnsiString(dVoltage);
	if( dCurrent<(minLoadCurr)|| dCurrent > (maxLoadCurr))
	{
		astrVoltage = AnsiString(dCurrent)+"A";
		//電流異常 重新計數
		bVolNormalValue = false;
		//超時
		if(GetTickCount() > dwGetDataTimeOut)
		{
			que_Msg.push("(!)Load-Voltage Failed");
			return LOAD_TEST_FAIL;
		}
	}
	if(plReadValue != NULL)
		plReadValue->Caption = dVoltage;
	if( dVoltage <= maxLoadVol && dVoltage >= minLoadVol)
	{
		if(GetTickCount() <= dwGetDataTimeOut)
		{
			if(!bVolNormalValue)
			{   //電壓為正常範圍時 開始計數
				bVolNormalValue = true;
				dwGetNormalVolCount = 0;
			}
			dwGetNormalVolCount++;
			if(dwGetNormalVolCount >= 3)
				return LOAD_TEST_PASS;
		}
		else
		{
			que_Msg.push("(!)Load-Voltage Failed");
			return LOAD_TEST_FAIL;
		}
	}
	else
	{   //電壓為異常範圍時 重新計數
		bVolNormalValue = false;
		//超時->FAIL
		if(GetTickCount() > dwGetDataTimeOut)
		{
			que_Msg.push("(!)Load-Voltage Failed");
			return LOAD_TEST_FAIL;
		}
	}
	return RS232_WRITE;
}
DWORD cCOM::cmd_get_value_OCP_polling(char *s)
{
	que_Msg.push(GetTickCount());
	TStringList *sList = new TStringList();
	sList->Delimiter = '\r\n';
	sList->DelimitedText = AnsiString(s);
	que_Msg.push(AnsiString(s));
	double dCurrent,dVoltage;
	if(sList->Count >=2)
	{
		dCurrent = GetNumOfString(sList->Strings[0]);
		dVoltage = GetNumOfString(sList->Strings[1]);
	}
	else
	{
		delete  sList;
		return RS232_WRITE;
	}
	delete  sList;

	astrVoltage = AnsiString(dVoltage);
	if( dCurrent<(minLoadCurr)|| dCurrent > (maxLoadCurr))
	{
		dwPollingCount++;
		astrVoltage = AnsiString(dCurrent)+"A";
		if(dwPollingCount >= dwPollingOCPdenominator)
		{
			que_Msg.push("(!)Load-Voltage Failed");
			return LOAD_TEST_FAIL;
		}
		return RS232_WRITE;
	}
	if(plReadValue != NULL)
		plReadValue->Caption = dVoltage;
	if( dVoltage <= maxLoadVol && dVoltage >= minLoadVol)
	{
		if(GetTickCount() <= dwGetDataTimeOut)
		{
			dwGetNormalVolCount++;
			dwPollingCount = 0;
			if(dwGetNormalVolCount >= 3)
				return LOAD_TEST_PASS;
		}
		else
		{
			que_Msg.push("(!)Load-Voltage Failed");
			return LOAD_TEST_FAIL;
		}
	}
	else
	{
		dwPollingCount++;
		if(dwPollingCount >= dwPollingOCPdenominator)
		{
			que_Msg.push("(!)Load-Voltage Failed");
			return LOAD_TEST_FAIL;
		}
		else if(GetTickCount() > dwGetDataTimeOut)
		{
			que_Msg.push("(!)Load-Voltage Failed");
			return LOAD_TEST_FAIL;
		}
	}

	return RS232_WRITE;
}
DWORD cCOM::open_dev_com()
{
	if(!CheckLoaderType())
		return LOADER_NO_SUPPORT;
	if(bCOM_PORT_OPEN)
	{
		return LOADER_OK;
	}
	DWORD WaitTime=GetTickCount()+5000;
	if(!strstr(DeviceCOM.c_str(),"COM")) SearchDevCOM(); //偵測是否有裝置名稱
	while(1)
	{
		if(strstr(DeviceCOM.c_str(),"NO-SUPPORT"))
		{
			bCOM_PORT_OPEN = false;
			return LOADER_NO_SUPPORT;
		}
		else if(strstr(DeviceCOM.c_str(),"COM"))
		{
			DEV_hCom = CreateFile(WideString(DeviceCOM).c_bstr(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
				OPEN_EXISTING, 0, NULL);
			if (DEV_hCom == INVALID_HANDLE_VALUE) {
				Delay(100);
				Dev_Stop();
				DEV_hCom = CreateFile(WideString(DeviceCOM).c_bstr(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
					OPEN_EXISTING, 0, NULL);
				if (DEV_hCom == INVALID_HANDLE_VALUE){
					CloseHandle(DEV_hCom);
					bCOM_PORT_OPEN = false;
					return 0;
				}
			}
			DCB dcb; // 設定comport相關係數
			GetCommState(DEV_hCom, &dcb);
			BuildCommDCB(L"115200,n,8,1", &dcb);
			SetCommState(DEV_hCom, &dcb);
			// 設定TimeOut
			COMMTIMEOUTS TimeOut;
			TimeOut.ReadIntervalTimeout = MAXDWORD;
			TimeOut.ReadTotalTimeoutMultiplier = 0;
			TimeOut.ReadTotalTimeoutConstant = 100;
			TimeOut.WriteTotalTimeoutMultiplier  = 0;
			TimeOut.WriteTotalTimeoutConstant  = 0;
			SetCommTimeouts(DEV_hCom, &TimeOut);
			PurgeComm(DEV_hCom,PURGE_TXCLEAR);
			bCOM_PORT_OPEN = true;
			return LOADER_OK;
		}
		else if(GetTickCount()>WaitTime)
		{
			bCOM_PORT_OPEN = false;
			return LOADER_NOT_FIND;
		}
		else Delay(100);
	}
}
int  cCOM::Dev_CMD_Test()
{
	unsigned long Written ;
	int dwStep = RS232_WRITE ;
	bool bError = false;
	dwGetNormalVolCount = 0;
	dwGetDataTimeOut = 0,dwShortTimeOut = 0;
	dwPollingCount   = 0;
	//

	while(dwStep)
	{
		switch(dwStep)
		{
			case RS232_WRITE:
			//寫
				dwStep = RS232_READ;
				bNoResponse = false;
				que_Msg.push(que_cmd.front());
				WriteFile(DEV_hCom,que_cmd.front().c_str(),que_cmd.front().Length(),&Written,NULL);
				cmd_TimeOut=GetTickCount()+1000;
				break;
			case RS232_READ:
				//讀
				if(strstr(que_cmd.front().c_str(),"meas:curr?;meas:volt?"))
				{
					DWORD ByteRead;
					char RXBuff[30];
					char buff[30];
					while(1)
					{
						ReadFile(DEV_hCom, RXBuff, 30, &ByteRead,NULL);//接收
						if (ByteRead>0) // 接收到數據
						{
							strncpy(buff,RXBuff,ByteRead);
							if(bOCPPolling)
								dwStep = cmd_get_value_OCP_polling(buff);
							else
								dwStep = cmd_get_system_consumptoin(buff);
							if(dwStep == TEST_SHORT_PASS || dwStep == LOAD_TEST_PASS)
								que_cmd.pop();
							while(1)
							{
								ReadFile(DEV_hCom, RXBuff, 30, &ByteRead,NULL);
								if (ByteRead == 0)
									break;
								Delay(30);
							}
							break;
						}
						else if(GetTickCount()>cmd_TimeOut)
						{
							bNoResponse = true;
							dwStep = Dev_NO_RESPOND;
							break;
						}
					}
					memset(buff, 0, sizeof(buff));
					memset(RXBuff, 0, sizeof(RXBuff));
				}
				else
				{
					if(strstr(que_cmd.front().c_str(),"LOAD ON"))
					{
						bLoad = true;
						bVolNormalValue = false;
						dwGetDataTimeOut = GetTickCount()+dwLoadTimeOut;
					}
					else if(strstr(que_cmd.front().c_str(),"LOAD OFF"))
					{
						bLoad = false;
						dwGetDataTimeOut = GetTickCount()+dwLoadTimeOut;
						bVolNormalValue = false;
					}
					Delay(50); // 回應間隔
					que_cmd.pop();

					if(!que_cmd.empty())
						dwStep = RS232_WRITE;
					else dwStep = TEST_END;
				}
				break;
			case LOAD_TEST_PASS:
				if(!que_cmd.empty())
					dwStep = RS232_WRITE;
				else	dwStep = TEST_END;
				break;
			case LOAD_TEST_FAIL:
				bError = true;
				que_cmd.c.clear();
				que_Msg.push("(!)LOAD FAILED");
				if(bLoad)
				{
					que_cmd.push("SHORt OFF;LOAD OFF;\n");
					dwStep = RS232_WRITE;
				}
				else dwStep = TEST_END;
				break;
			case Dev_NO_RESPOND:
				CloseHandle(DEV_hCom);
				bCOM_PORT_OPEN = false;
				bError = true;
				dwStep = TEST_END;
				break;
			case TEST_END:

				break;
		}
		Delay(50);
	}
    que_cmd.push("END");
	return !bError;
}
//---------------------------------------------------------------------------
void cCOM::search_Reg_ComNum() // 取得 regedt COM
{
	AnsiString temppath;
	TRegistry *reg = new TRegistry();
	TStringList* itemSet = new TStringList();
	reg->RootKey = HKEY_LOCAL_MACHINE;
	temppath = "HARDWARE\\DEVICEMAP\\SERIALCOMM";
	reg->OpenKey(temppath.c_str(), false);
	reg->GetValueNames(itemSet);
	for(int i=0;i<itemSet->Count;i++)
	{
		AnsiString comName =(AnsiString)reg->ReadString(itemSet->Strings[i]);
		if(strlen(comName.c_str())>4) comName="\\\\.\\"+comName;
		comNum_Info[i].ComName=(AnsiString)comName.c_str();
	}
	reg->CloseKey();
	delete itemSet;
	delete reg;
}
void cCOM::SearchDevCOM()// Create多執行緒 找裝置
{
	search_Reg_ComNum();
	DeviceCOM = "";
	int Count=0;
	while(strstr(comNum_Info[Count].ComName.c_str(),"COM"))
	{
		comNum_Info[Count].Threadhandle = ::CreateThread(0, 0, WORKTESTExecute, (void*)Count, 0, NULL);
		CloseHandle(comNum_Info[Count].Threadhandle);
		Count++;
	}
}
DWORD WINAPI WORKTESTExecute(LPVOID Param) // 執行續
{

//開COM
	comNum_Info[(int)Param].handle = CreateFile(WideString(comNum_Info[(int)Param].ComName).c_bstr(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, 0, 0);
	if (comNum_Info[(int)Param].handle != INVALID_HANDLE_VALUE) {
		DCB dcb; // 設定comport相關係數
		GetCommState(comNum_Info[(int)Param].handle, &dcb);
		BuildCommDCB(L"115200,n,8,1", &dcb);
		SetCommState(comNum_Info[(int)Param].handle, &dcb);
		// 設定TimeOut
		COMMTIMEOUTS TimeOut;
		GetCommTimeouts(comNum_Info[(int)Param].handle, &TimeOut);
		TimeOut.ReadIntervalTimeout = MAXDWORD;
		TimeOut.ReadTotalTimeoutMultiplier = 0;
		TimeOut.ReadTotalTimeoutConstant = 300;  //(總讀取 time out 時間 = 0.1 秒)
		TimeOut.WriteTotalTimeoutMultiplier = 0;
		TimeOut.WriteTotalTimeoutConstant = 100; //(總寫入 time out 時間 = 0.1 秒)
		SetCommTimeouts(comNum_Info[(int)Param].handle, &TimeOut);
	//寫
		unsigned long Written ;
		char* data="*remote;name?\n";
		WriteFile(comNum_Info[(int)Param].handle,data,strlen(data),&Written,NULL);
	//讀
		DWORD ByteRead;
		char RXBuff[1024];
		char buff[1024];
		ReadFile(comNum_Info[(int)Param].handle, RXBuff, 1024, &ByteRead,NULL);//接收
		if (ByteRead>0) // 接收到數據
		{
			strncpy(buff,RXBuff,ByteRead);
			if(strstr(buff,"3315F")||strstr(buff,"3310F")||strstr(buff,"3311F"))
			{  //75W                 //150W                //300W
				DeviceCOM = "";
				if(dwMAX_WATT == LOADER_3310F)
				{
					if(strstr(buff,"3310F") || strstr(buff,"3311F"))
					{
						dwLoader_Type = strstr(buff,"3310F") ? LOADER_3310F:LOADER_3311F;
						DeviceCOM = comNum_Info[(int)Param].ComName;
					}
				}
				else if(dwMAX_WATT == LOADER_3311F)
				{
					if(strstr(buff,"3311F"))
					{
						dwLoader_Type = LOADER_3311F;
						DeviceCOM = comNum_Info[(int)Param].ComName;
					}
				}
				else if(dwMAX_WATT == LOADER_3315F)
				{
					dwLoader_Type = LOADER_3315F;
					DeviceCOM = comNum_Info[(int)Param].ComName;
				}
				if(DeviceCOM == "")
					DeviceCOM = "NO-SUPPORT";
			}
		}
		CloseHandle(comNum_Info[(int)Param].handle);
		memset(RXBuff, 0, sizeof(RXBuff));
	}
	return 0;
}
void cCOM::Dev_Stop() {
	bCOM_PORT_OPEN = false;
	Delay(50);
	que_cmd.c.clear();
	CloseHandle(DEV_hCom);
	DEV_hCom = NULL;
}
void cCOM::CloseDLLoad()
{
	if(DEV_hCom != NULL)
	{
		unsigned long Written ;
		WriteFile(DEV_hCom,"SHORt OFF;LOAD OFF;\n",strlen("SHORt OFF;LOAD OFF;\n"),&Written,NULL);
		Delay(50);
	}
}
void cCOM::QueueClear()//Queue Clear
{
	/*while(!FrmMain->que_cmd.empty())
	{
		FrmMain->que_cmd.pop();
	} */
}
double cCOM::GetNumOfString(AnsiString String)
{
	TCHAR HexChar[10];
	bool bPoint = false;
	AnsiString sResult = "";
	int DecimalPlace = 0;

	try
	{
		String = String.UpperCase();// 把字串轉成全大寫
		_tcscpy_s(HexChar, 10, WideString(String).c_bstr());
		for(int j=0;j<String.Length();j++)
		{
			if(HexChar[j] < 0x30 || HexChar[j] > 0x39){
				if(HexChar[j] == 0x2E)
				{
					if(!bPoint&&j==0)
					{
						sResult+="0";
					}
					sResult+=HexChar[j];
					bPoint = true;
				}
			}
			else
			{
				if(bPoint) DecimalPlace++;
				sResult+=HexChar[j];
			}
			if(DecimalPlace==3) break;
		}

		return sResult.ToDouble();
	}
	catch(...)
	{
	   return 0;
    }
}
void cCOM::SetTestVoltage(AnsiString Voltage)
{
	gVoltage = Voltage;
}
void cCOM::SetMAX_WATT(DWORD MAX_WATT)
{
	dwMAX_WATT = MAX_WATT;
}
void cCOM::Delay(ULONG iMilliSeconds) // 原版delay time 用在thread裡面
{
	ULONG iStart;
	iStart = GetTickCount();
	while (GetTickCount() - iStart <= iMilliSeconds)
		Application->ProcessMessages();
}
bool cCOM::CheckLoaderType()
{
	if(dwMAX_WATT != 0 && dwLoader_Type != 0)
	{
		if(dwMAX_WATT > dwLoader_Type)
			return false;
	}
	return true;
}
