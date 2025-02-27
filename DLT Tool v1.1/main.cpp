﻿//---------------------------------------------------------------------------
#include <windows.h>    // 安全移除USB裝置用 *要比 vcl.h 早編譯
#include <SetupAPI.h> // 安全移除USB裝置用 *要比 vcl.h 早編譯
#include <cfgmgr32.h> // 安全移除USB裝置用 *要比 vcl.h 早編譯
#include <vcl.h>
#pragma hdrstop

#include "main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFrmMain *FrmMain;
//UI調整參數
int MouseY,memo_heigh=0,move_timeout = 0;//,VOLTAGE_TOLERANCE_RANGE=20;
bool xCapture;
float voltage;
float MAX_VOLTAGE_TOLERANCE_RANGE[5],MIN_VOLTAGE_TOLERANCE_RANGE[5];

int TimeStart = 0;
bool bTesting = false;
//---------------------------------------------------------------------------
__fastcall TFrmMain::TFrmMain(TComponent* Owner)
	: TForm(Owner)
{
	EnumHID();

	bTEST_END_SWITCH_5V = false;
	//
	FrmMain->OldWindowProc = WindowProc;
	FrmMain->WindowProc = MyWindowProc;
	GUID guid;
	// 註冊USB HUB裝置
	guid =StringToGUID(GUID_USB_HUB);
	g_DeviceNogification.RegisterWindowsDeviceInterfaceNotification(
		Handle,guid);

	//USB Device
	guid =StringToGUID(GUID_USB_DEVICE);
	g_DeviceNogification.RegisterWindowsDeviceInterfaceNotification(
		Handle,guid);

	//HID Device
	guid =StringToGUID(GUID_HID);
	g_DeviceNogification.RegisterWindowsDeviceInterfaceNotification(
		Handle,guid);
	//
	//ReadRegSet();
	if(FindIniFile()) ReadInISet();
	CL_DEV_CONTROL = new cCOM();

	FrmMain->Caption = APP_TITLE;
}
void __fastcall TFrmMain::EnumHID(){
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD i,j;
	AnsiString SS,USBPath;
	PSP_DEVICE_INTERFACE_DETAIL_DATA   pDetail   =NULL;
	GUID GUID_USB_HID =StringToGUID(GUID_HID);
	DEBUG("[ HID裝置列舉 ]");
	//--------------------------------------------------------------------------
	//   獲取設備資訊
	hDevInfo = SetupDiGetClassDevs((LPGUID)&GUID_USB_HID,
	0,   //   Enumerator
	0,
	DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );
	if   (hDevInfo   ==   INVALID_HANDLE_VALUE){
		DEBUG("ERROR - SetupDiGetClassDevs()"); //   查詢資訊失敗
	}
	else{
	//--------------------------------------------------------------------------
		SP_DEVICE_INTERFACE_DATA   ifdata;
		DeviceInfoData.cbSize   =   sizeof(SP_DEVINFO_DATA);
		for (i=0;SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData);i++)	//   枚舉每個USB設備
		{
			ULONG   len;
			CONFIGRET cr;
			PNP_VETO_TYPE   pnpvietotype;
			CHAR   vetoname[MAX_PATH];
			ULONG   ulStatus;
			ULONG   ulProblemNumber;
			ifdata.cbSize   =   sizeof(ifdata);
			if (SetupDiEnumDeviceInterfaces(								//   枚舉符合該GUID的設備介面
			hDevInfo,           //   設備資訊集控制碼
			NULL,                         //   不需額外的設備描述
			(LPGUID)&GUID_USB_HID,//GUID_CLASS_USB_DEVICE,                     //   GUID
			(ULONG)i,       //   設備資訊集裏的設備序號
			&ifdata))                 //   設備介面資訊
			{
				ULONG predictedLength   =   0;
				ULONG requiredLength   =   0;
				//   取得該設備介面的細節(設備路徑)
				SetupDiGetInterfaceDeviceDetail(hDevInfo,         //   設備資訊集控制碼
					&ifdata,          //   設備介面資訊
					NULL,             //   設備介面細節(設備路徑)
					0,         	      //   輸出緩衝區大小
					&requiredLength,  //   不需計算輸出緩衝區大小(直接用設定值)
					NULL);            //   不需額外的設備描述
				//   取得該設備介面的細節(設備路徑)
				predictedLength=requiredLength;
				pDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)::GlobalAlloc(LMEM_ZEROINIT,   predictedLength);
				pDetail->cbSize   =   sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

				if(SetupDiGetInterfaceDeviceDetail(hDevInfo,         //   設備資訊集控制碼
					&ifdata,             //   設備介面資訊
					pDetail,             //   設備介面細節(設備路徑)
					predictedLength,     //   輸出緩衝區大小
					&requiredLength,     //   不需計算輸出緩衝區大小(直接用設定值)
					NULL))               //   不需額外的設備描述
				{
					try
					{
						char ch[512];
						for(j=0;j<predictedLength;j++)
						{
						ch[j]=*(pDetail->DevicePath+8+j);
						}
						SS=ch;
						DEBUG(SS);
					}
					catch(...)
					{
						DEBUG(SS+"列舉失敗");
					}

				}
				else
				{
					DEBUG("SetupDiGetInterfaceDeviceDetail F");
				}
			}
			else
			{
				DEBUG("SetupDiEnumDeviceInterfaces F");
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TFrmMain::MyWindowProc(TMessage& Message){
	PDEV_BROADCAST_DEVICEINTERFACE pDevInf;
	AnsiString DBCC_Name;
	AnsiString DutVPID = "vid_"+AnsiString(edtVID->Text)+"&pid_"+AnsiString(edtPID->Text);

	pDevInf =(PDEV_BROADCAST_DEVICEINTERFACE)Message.LParam;

	//
	if (Message.Msg==WM_DEVICECHANGE){
		if(Message.WParam ==DBT_DEVICEARRIVAL){
			if(AnsiString(pDevInf->dbcc_name).Length()>0)
			{
				DBCC_Name =AnsiString(pDevInf->dbcc_name).LowerCase();
				if(strstr(DBCC_Name.c_str(),AnsiString(PD_BOARD_PVID).c_str()))
				{
					EnumHID();
					DEBUG("PD Test Board Is Plugins");
				}
				if(ckbAuto->Checked)
				{
					if(strstr(DBCC_Name.c_str(),DutVPID.LowerCase().c_str())&&GetTickCount()>move_timeout)
					{
						DEBUG(move_timeout);
						if(!bTesting)
						{
							Delay(numNoLoadDelay);
							btnStart->Click();
						}
					}
				}
			}
		}
		else if(Message.WParam ==DBT_DEVICEREMOVECOMPLETE){
			DBCC_Name =AnsiString(pDevInf->dbcc_name).LowerCase();
			if(strstr(DBCC_Name.c_str(),AnsiString(PD_BOARD_PVID).c_str()))
				if(m_hid.m_online) HID_TurnOff();
			if(ckbAuto->Checked&&!bTesting)
			{
				if(strstr(DBCC_Name.c_str(),DutVPID.LowerCase().c_str()))
				{

					pl_read_NoLoad_voltage->Font->Color = clWhite;
					pl_read_NoLoad_voltage->Caption = "";
					pl_full_load_voltage->Font->Color = clWhite;
					pl_full_load_voltage->Caption = "";
					pl_over_load_voltage->Font->Color = clWhite;
					pl_over_load_voltage->Caption = "";
					plResult->Caption = "Wait...";
					plResult->Color = clWhite;
					move_timeout = GetTickCount()+1000;
				}
			}
		}

	}
	// 繼續原始訊息的分派
	TFrmMain::WndProc(Message);
}
//---------------------------------------------------------------------------
bool TFrmMain::HID_TurnOn(){
	//配置HID物件
	wchar_t HID_PID[5];
	wchar_t HID_VID[5];
	wcsncpy(HID_PID,PD_BOARD_PVID+8,4);
	wcsncpy(HID_VID,PD_BOARD_PVID+17,4);
	HID_PID[4]='\0';
	HID_VID[4]='\0';

	if(m_hid.Configure(HID_PID,HID_VID,L"",L"")){
		if(!m_hid.Open()){
			MessageBox(this->Handle, _T("開啟 PD 板失敗!"), Application->Title.t_str(), MB_ICONWARNING);
			return false;
        }
	}else{
		MessageBox(this->Handle, _T("配置 PD 板失敗!"), Application->Title.t_str(), MB_ICONWARNING);
		return false;
	}
	HID_PID[4] = 0;
	HID_PID[4] = 0;

	return true;
}
void TFrmMain::HID_TurnOff(){
	m_hid.Close();
}
//---------------------------------------------------------------------------
void TFrmMain::Tx(AnsiString Value){
	int szTx, szCmd;
	unsigned char * cmd;
	TStringList * list;

	szTx = m_hid.GetTxBytes();
	m_or.AllocBuf(szTx);
	cmd = (unsigned char *)m_or.GetBufAddr();

	list = new TStringList();
	list->DelimitedText = Value;

	if(list->Count > szTx){
		szCmd = szTx;
	}else{
        szCmd = list->Count;
	}

	for(int i=0; i<szCmd; i++){
		list->Strings[i] = L"0x" + list->Strings[i];
		cmd[i] = list->Strings[i].ToInt();
	}

	HidD_SetOutputReport(m_hid.m_hWrite, cmd, szTx);
	UI_DisplayCmd(cmd, szTx);

	delete list;
}
void __fastcall TFrmMain::UI_DisplayCmd(unsigned char *pBuf, int size){
	String str, str2;
	str = _T("寫入 : ");

	for(int i=0; i<size; i++){
		if(i == 0){
			str2.printf(_T("%02X"), pBuf[i]);
		}else{
			str2.printf(_T(" %02X"), pBuf[i]);
		}

		str += str2;
	}

	DEBUG(str);
}
//---------------------------------------------------------------------------
bool TFrmMain::Rx(AnsiString Value)
{
	DWORD nBytes;
	bool bl=false;

	if(!m_ir.AllocBuf(m_hid.GetRxBytes())){
		MessageBox(NULL, _T("FAIL!"), _T("Allocate read buffer"), MB_ICONSTOP);
		return false;
	}

	if(m_ir.Read(m_hid.m_hRead)){
		if(HID_ReadReport(Value)) bl = true;
	}else{
		if(GetLastError() == ERROR_IO_PENDING){
			WaitForSingleObject(m_hid.m_hRead, 1000);
			if(GetOverlappedResult(m_hid.m_hRead, &m_ir, &nBytes, false)){
				if(HID_ReadReport(Value)) bl = true;
			}
			CancelIo(m_hid.m_hRead);
		}
	}
	return bl;
}
bool __fastcall TFrmMain::HID_ReadReport(AnsiString Value){
	String buf, buf2;
	const unsigned char *report = (const unsigned char *)m_ir.GetBufAddr();

	buf = _T("讀取 : ");

	for(int i=0; i!=m_ir.GetBufSize(); i++){
		if(i==0){
			buf2.printf(_T("%02X"), report[i]);
		}else{
			buf2.printf(_T(" %02X"), report[i]);
		}

		buf += buf2;
	}
	DEBUG(buf);

	AnsiString buf3 = AnsiString(buf.c_str());
	if(Value!="")
	{
		if(!strcmp(buf3.SubString(8,Value.Length()).c_str(),Value.c_str()))
			return true;
		else return false;
	}
	else
	{
		Rx_ValueAnalyze(buf3.SubString(8,23));
		return true;
	}
}
void TFrmMain::Rx_ValueAnalyze(AnsiString Value){
	AnsiString HexValue = Value.SubString(19,2)+Value.SubString(22,2);
	m_ADValue.printf("%.3f",float(HexToInt(HexValue))*UNIT);
	DEBUG("Voltage Value : "+m_ADValue+" V");
}
//---------------------------------------------------------------------------
void TFrmMain::Voltage_Test()
{
	//初始化
	int step = READ_NO_LOAD_VOLTAGE_VALUE;  //DUCK180 不透過PD版直接取得負載機電壓
	bool bl = true;
	if(!bTEST_END_SWITCH_5V)
	{
		pl_read_NoLoad_voltage->Color = clWhite;
		pl_read_NoLoad_voltage->Caption = "Testing...";
		pl_full_load_voltage->Color = clWhite;
		pl_full_load_voltage->Caption = "";
		pl_over_load_voltage->Color = clWhite;
		pl_over_load_voltage->Caption = "";
	}

	FrmMain->Refresh();

	while(bl)
	{
		switch(step)
		{
			case READ_NO_LOAD_VOLTAGE_VALUE:
				step = DL_CMD("load off;meas:curr?;meas:volt?\n");
				if(step == TEST_PASS)
				{
					if(bTEST_END_SWITCH_5V)
					{
						bTEST_END_SWITCH_5V = false;
						step = TEST_VOLTAGE_END;
					}
					else
					{
						pl_read_NoLoad_voltage->Font->Color = clGreen;
						step = READ_FULL_LOAD_VOLTAGE_VALUE;
					}
				}
				else if(step == TEST_FAIL)
				{
					if(bTEST_END_SWITCH_5V)
					{
						bTEST_END_SWITCH_5V = false;
						step = TEST_VOLTAGE_END;
					}
					else
					{
						pl_read_NoLoad_voltage->Font->Color = clRed;
						g_bError = true;
						step = READ_FULL_LOAD_VOLTAGE_VALUE;
					}
				}
				else if(step == DL_NOT_FIND)
				{
					pl_read_NoLoad_voltage->Font->Color = clRed;
					pl_read_NoLoad_voltage->Caption = "FAIL";
					g_bError = true;
				}
				break;
			case READ_FULL_LOAD_VOLTAGE_VALUE:
				pl_full_load_voltage->Caption = "Testing...";
				FrmMain->Refresh();
				step = DL_CMD("LOAD ON\n");
				if(step == TEST_PASS)
				{
					pl_full_load_voltage->Font->Color = clGreen;
					if(ckbOverLoad->Checked)
					  step = READ_OVER_LOAD_VOLTAGE_VALUE;
					else step = TEST_VOLTAGE_END;
				}
				else if(step == TEST_FAIL)
				{
					pl_full_load_voltage->Font->Color = clRed;
					if(ckbOverLoad->Checked)
					  step = READ_OVER_LOAD_VOLTAGE_VALUE;
					else step = TEST_VOLTAGE_END;
					g_bError = true;
				}
				else if(step == DL_NOT_FIND)
				{
					pl_read_NoLoad_voltage->Font->Color = clRed;
					pl_read_NoLoad_voltage->Caption = "FAIL";
					g_bError = true;
				}
				break;
			case READ_OVER_LOAD_VOLTAGE_VALUE:
				pl_over_load_voltage->Caption = "Testing...";
				FrmMain->Refresh();
				bOverload = true;
				step = DL_CMD("LOAD ON\n");
				bOverload = false;
				if(step == TEST_PASS)
				{
					pl_over_load_voltage->Font->Color = clGreen;
					step = REBACK_NO_LOAD;
				}
				else if(step == TEST_FAIL)
				{
					pl_over_load_voltage->Font->Color = clRed;
					step = TEST_VOLTAGE_END;
					g_bError = true;
				}
				else if(step == DL_NOT_FIND)
				{
					pl_read_NoLoad_voltage->Font->Color = clRed;
					pl_read_NoLoad_voltage->Caption = "FAIL";
					g_bError = true;
				}
				bOverload = false;
				DL_CMD("LOAD OFF\n");
				break;
			case REBACK_NO_LOAD:
				step = DL_CMD("load off;meas:curr?;meas:volt?\n");
				if(step == TEST_FAIL)
				{
					pl_read_NoLoad_voltage->Font->Color = clRed;
					g_bError = true;
				}
				else if(step == DL_NOT_FIND)
				{
					pl_read_NoLoad_voltage->Font->Color = clRed;
					pl_read_NoLoad_voltage->Caption = "FAIL";
					g_bError = true;
				}
				step = TEST_VOLTAGE_END;
				break;
			case TEST_VOLTAGE_END:
				DL_CMD("LOAD OFF\n");
				bl =false;
				break;
			case HID_NOT_FIND:
				pl_read_NoLoad_voltage->Font->Color = clRed;
				pl_read_NoLoad_voltage->Caption = "FAIL";
				g_bError = true;
				bl =false;
				break;
			case DL_NOT_FIND:
				g_bError = true;
				bl =false;
				break;
		}
	}
}

//---------------------------------------------------------------------------
int  TFrmMain::HexToInt(AnsiString HexStr) {//16進位轉10進位
	int iIndex, iHex, totalChar;
	TCHAR HexChar[8];

	HexStr.UpperCase(); // 把字串轉成全大寫
	_tcscpy_s(HexChar, 8, WideString(HexStr).c_bstr());
//	strcpy(HexChar, HexStr.c_str()); // 把字串轉成字元陣列
	iHex = 0;
	totalChar = HexStr.Length(); // 取得字串的長度
	for (iIndex = 0; iIndex < totalChar; iIndex++) {
		iHex <<= 4;
		if ((HexChar[iIndex] >= 0x30) && (HexChar[iIndex] <= 0x39)) {
			iHex += (HexChar[iIndex] - 0x30); // 把 0 - 9 字元轉成數字
		}
		else if ((HexChar[iIndex] >= 0x41) && (HexChar[iIndex] <= 0x46)) {
			iHex += (HexChar[iIndex] - 0x37); // ­把 A - F 字元轉成數字
		}
		else {
			iHex = 0;
			break;
		}
	}
	return iHex + HARDWARE_VOLTAGE_COMPENSATION;
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::btnStartClick(TObject *Sender)
{
	if(btnStart->Enabled)
	{
		plResult->Caption = "Testing...";
		plResult->Color = clWhite;
		btnStart->Enabled = false;
		g_bError = false;
		TimeStart = GetTickCount();
		bool bHAVEDEVICE = false;
		pl_read_NoLoad_voltage->Font->Color = clBlue;
		pl_read_NoLoad_voltage->Caption = "";
		pl_full_load_voltage->Font->Color = clBlue;
		pl_full_load_voltage->Caption = "";
		pl_over_load_voltage->Font->Color = clBlue;
		pl_over_load_voltage->Caption = "";

		bHAVEDEVICE = true;
		VoltageMainTest();

		if(pl_read_NoLoad_voltage->Caption == "FAIL")
		{
			pl_full_load_voltage->Font->Color = clRed;
			pl_full_load_voltage->Caption = "FAIL";
			if(ckbOverLoad->Checked)
			{
				pl_over_load_voltage->Font->Color = clRed;
				pl_over_load_voltage->Caption = "FAIL";
			}
		}
		DEBUG((GetTickCount()-TimeStart)/1000);
		if(!bHAVEDEVICE) g_bError = true;
		plResult->Color = g_bError ? clRed:clLime;
		plResult->Caption = g_bError ? "FAIL":"PASS";

		btnStart->Enabled = true;
	}
}
//---------------------------------------------------------------------------
void TFrmMain::ReadRegSet()
{
	AnsiString value;
	TRegistry *reg = new TRegistry();

	try
	{
		reg->RootKey = HKEY_CURRENT_USER;
		reg->OpenKey("SOFTWARE", false);
		bool bl = reg->OpenKey("PD Voltage Test", true);
		bl = reg->OpenKey("PD Voltage Test 2.0.2", true);

		if(bl)
		{

			if(reg->ValueExists("DutPluginAutoTest")){
				value = reg->ReadString("DutPluginAutoTest");
				ckbAuto->Checked = value == 1 ? true:false;
			}
			if(reg->ValueExists("DutPidVid")){
				value = reg->ReadString("DutPidVid");
				edtVID->Text = value.SubString(5,4);
				edtPID->Text = value.SubString(14,4);
			}
			if(reg->ValueExists("SetCurrent")){
				edtSetCurrent->Text = reg->ReadString("SetCurrent");
			}
			for(int i=1 ;i<=5 ;i++)
			{
				if(reg->ValueExists("VoltageToleranceRange_"+IntToStr(i))){
					value = reg->ReadString("VoltageToleranceRange_"+IntToStr(i));
					((TEdit *)FindComponent("edt_min"+IntToStr(i)))->Text	  = value.SubString(1,value.Pos("、")-1);
					((TEdit *)FindComponent("edt_max"+IntToStr(i)))->Text	  = value.SubString(value.Pos("、")+2,value.Pos("&")-value.Pos("、")-2);
					value = value.Delete(1,value.Pos("&"));
					((TEdit *)FindComponent("edt_load_min"+IntToStr(i)))->Text	  = value.SubString(1,value.Pos("、")-1);
					((TEdit *)FindComponent("edt_load_max"+IntToStr(i)))->Text	  = value.SubString(value.Pos("、")+2,value.Length()-value.Pos("、")+2);
				}
			}
			if(reg->ValueExists("OverLoadTestItem")){
				value = reg->ReadString("OverLoadTestItem");
				TPanel* zCkbOverLoad[5];
				for(int i=0;i<5;i++)
				   zCkbOverLoad[i] = (TPanel*)FindComponent("plFullLoad_"+IntToStr(i+1));
				for(int i=1;i <=value.Length();i++)
					zCkbOverLoad[value.SubString(i,1).ToInt()-1]->Caption = "V";
			}
			SaveVoltageRangeSetting();
		}
	}__finally {
		delete reg;
	}
	FrmMain->Refresh();
}
void TFrmMain::SetRegVal(AnsiString numTestItem,bool AutoTest,AnsiString DutVPID){
	TRegistry *reg = new TRegistry;

	try {
		reg->RootKey = HKEY_CURRENT_USER;
		reg->OpenKey("SOFTWARE", false);
		bool bl = reg->OpenKey("PD Voltage Test", true);
		bl = reg->OpenKey("PD Voltage Test 2.0.2", true);
		AnsiString VoltageRange[5];
		for(int i =1 ;i <=5 ;i++)
			VoltageRange[i-1] =
				AnsiString(((TEdit *)FindComponent("edt_min"+IntToStr(i)))->Text)
				+ "、"
				+ AnsiString(((TEdit *)FindComponent("edt_max"+IntToStr(i)))->Text)
				+ "&"
				+ AnsiString(((TEdit *)FindComponent("edt_load_min"+IntToStr(i)))->Text)
				+ "、"
				+ AnsiString(((TEdit *)FindComponent("edt_load_max"+IntToStr(i)))->Text);

		if(bl){
			reg->WriteString("AutoTestItem", numTestItem);
			if(edtVID->Font->Color==clBlue&&edtPID->Font->Color==clBlue)
				reg->WriteString("DutPidVid", DutVPID);
			reg->WriteString("DutPluginAutoTest", AutoTest ? "1":"0");
			reg->WriteString("SetCurrent",edtSetCurrent->Text);
			reg->WriteString("VoltageToleranceRange_1",  VoltageRange[0]);
			reg->WriteString("VoltageToleranceRange_2",  VoltageRange[1]);
			reg->WriteString("VoltageToleranceRange_3", VoltageRange[2]);
			reg->WriteString("VoltageToleranceRange_4", VoltageRange[3]);
			reg->WriteString("VoltageToleranceRange_5", VoltageRange[4]);

			AnsiString numOverLoad="";
			for(int i=0;i<5;i++)
			   if(((TPanel*)FindComponent("plFullLoad_"+IntToStr(i+1)))->Caption!="")
					numOverLoad+=AnsiString(i+1);
			reg->WriteString("OverLoadTestItem", numOverLoad);
		}
	} __finally {
		delete reg;
	}
}

void __fastcall TFrmMain::FormClose(TObject *Sender, TCloseAction &Action)
{
	if(btnStart->Enabled)
		SetInIVal(ckbAuto->Checked,"vid_"+edtVID->Text+"&pid_"+edtPID->Text);
}
//---------------------------------------------------------------------------
void __fastcall TFrmMain::ckbAutoClick(TObject *Sender)
{
	btnStart->Caption = ckbAuto->Checked ? "Auto Test":"Start";
}
//---------------------------------------------------------------------------
bool TFrmMain::CheckVPIDSET(TEdit * edt){
	TCHAR HexChar[8];
	if(edt->Text.Length() == 0){}
	else if(edt->Text.Length() <= 4 ){
		edt->Text = edt->Text.UpperCase();// 把字串轉成全大寫
		AnsiString pvid = edt->Text;
		_tcscpy_s(HexChar, 8, WideString(pvid).c_bstr());
		for(int j=0;j<pvid.Length();j++)
		{
			if(HexChar[j] < 0x30 || HexChar[j] > 0x39){
				if(HexChar[j] < 0x41 || HexChar[j] > 0x5A){
					edt->Font->Color = clRed;
					return false;
				}
			}
		}
	}
	else
	{
		int bb = edt->Text.Length();
		edt->Font->Color = clRed;
		return false;
	}
	if(edt->Text.Length() == 4) edt->Font->Color = clBlue;
	else edt->Font->Color = clWindowText;

	edt-> SelStart=edt-> Text.Length();
	return true;
}


void __fastcall TFrmMain::edtVIDChange(TObject *Sender)
{
	CheckVPIDSET((TEdit*)Sender);
}
//---------------------------------------------------------------------------
void TFrmMain::VoltageMainTest()
{
	if(!bTesting)
	{
		bTesting = true;
		if(btnStart->Enabled)
		{
			g_bError = false;
			plResult->Caption = "Testing...";
			plResult->Color = clWhite;
			pl_read_NoLoad_voltage->Font->Color = clBlue;
			pl_read_NoLoad_voltage->Caption = "";
			pl_full_load_voltage->Font->Color = clBlue;
			pl_full_load_voltage->Caption = "";
			pl_over_load_voltage->Font->Color = clBlue;
			pl_over_load_voltage->Caption = "";
		}
		Voltage_Test();
		if(btnStart->Enabled)
		{
			plResult->Color = g_bError ? clRed:clLime;
			plResult->Caption = g_bError ? "FAIL":"PASS";
		}
		bTesting = false;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::moDebugChange(TObject *Sender)
{
	if(moDebug->Lines->Count >500) moDebug->Clear();
}
//---------------------------------------------------------------------------



void __fastcall TFrmMain::edtPassWordEnter(TObject *Sender)
{
	if(edtPassWord->Font->Color == clRed)
	{
		edtPassWord->Font->Color = clBlue;
		edtPassWord->Text 	= "";
	}
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::btnSetClick(TObject *Sender)
{
	if(btnSet->Caption == "ENTER")
	{
		if(edtPassWord->Text.UpperCase() == PASSWORD)
		{
			btnSet->Caption		 = "END";
			SetValueEnabled(true);
		}
		else edtPassWord->Font->Color = clRed;
	}
	else
	{
		if(SaveVoltageRangeSetting())
		{
			btnSet->Caption	  	= "ENTER";
			edtPassWord->Text 	= "";
			SetValueEnabled(false);
//---------------------------------
			if(btnStart->Enabled)
				SetInIVal(ckbAuto->Checked,"vid_"+edtVID->Text+"&pid_"+edtPID->Text);
		}
	}
}
//---------------------------------------------------------------------------
void  TFrmMain::SetValueEnabled(bool Enabled)
{
	edtPassWord->Enabled = !Enabled;
	edtVID->Enabled = Enabled;
	edtPID->Enabled = Enabled;
	ckbAuto->Enabled = Enabled;
	ckbOverLoad->Enabled = Enabled;

	pl_SetRange->Enabled	  = Enabled;
	edt_min->Enabled = Enabled;
	edt_max->Enabled = Enabled;
	edt_load_min->Enabled = Enabled;
	edt_load_max->Enabled = Enabled;
	edt_overload_min->Enabled = Enabled;
	edt_overload_max->Enabled = Enabled;

	edtSetCurrent->Enabled = Enabled;
	edtSetOverLoadCurrent->Enabled = Enabled;
	edtLossVol->Enabled = Enabled;
	btnStart->Enabled = !Enabled;
	pl_Set_Switch->Enabled = !Enabled;
}


void __fastcall TFrmMain::pl_ckb1_1Click(TObject *Sender)
{
	TPanel* pl=(TPanel*)Sender;
	pl->Caption = pl->Caption == "+" ? "-":"+";
	pl->Tag = pl->Caption=="+" ? 1:0;
}
//---------------------------------------------------------------------------


void __fastcall TFrmMain::edt_minChange(TObject *Sender)
{
	TEdit* edt=(TEdit*)Sender;
	if(edt->Text.Length()>5)
		edt->Text = edt->Text.SubString(1,5);
	TCHAR HexChar[8];
	bool bPASS = true;
	bool bPoint = false;
	if(edt->Text.Length() == 0){
		bPASS =false;
	}
	else
	{
		edt->Text = edt->Text.UpperCase();// 把字串轉成全大寫
		_tcscpy_s(HexChar, 8, edt->Text.c_str());
		for(int j=0;j<edt->Text.Length();j++)
		{
			if(HexChar[j] < 0x30 || HexChar[j] > 0x39){
				if(HexChar[j] == 0x2E)
				{
					if(j == edt->Text.Length()-1 ||bPoint)
					{
						edt->Font->Color = clRed;
						edt-> SelStart=edt-> Text.Length();
						bPASS = false;
					}
					if(!bPoint) bPoint = true;
				}
				else
				{
					edt->Font->Color = clRed;
					edt-> SelStart=edt-> Text.Length();
					bPASS = false;
				}
			}
		}
	}
	if(bPASS)
	{
		edt->Font->Color = edt->Text.ToDouble() <= 99 ? clBlue : clRed;
	}

	edt-> SelStart=edt-> Text.Length();
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::edt_minExit(TObject *Sender)
{
	TEdit* edt=(TEdit*)Sender;
	if(edt->Text.Length() == 0 || edt->Font->Color == clRed){
		edt->Text = AnsiString(edt->Tag)+".00";
		edt->Font->Color = clBlue;
	}
	else
	{
		AnsiString edtnum = AnsiString(edt->Text.ToDouble());
		edtnum.printf("%.2f",atof(edtnum.c_str()));
		edt->Text =  edtnum;
	}

}
//---------------------------------------------------------------------------
bool TFrmMain::SaveVoltageRangeSetting()
{

	if(edt_min->Text.ToDouble()>edt_max->Text.ToDouble())
	{
		AnsiString ERROR_MSG = "最小電壓不應大於最大電壓";
		MessageBoxA(NULL,ERROR_MSG.c_str(),"ERROR", MB_ICONEXCLAMATION);
		return false;
	}
	return true;
}

void __fastcall TFrmMain::pl_Set_SwitchClick(TObject *Sender)
{
	if(pl_Set->Height)
	{
	   pl_Set->Height = 0;
	   pl_Set_Switch ->Caption ="v";
	   FrmMain->Height -=229;
	}
	else
	{
	   pl_Set->Height = 229;
	   FrmMain->Height +=229;
	   pl_Set_Switch ->Caption ="^";
	}
}
//---------------------------------------------------------------------------
void TFrmMain::Delay(ULONG iMilliSeconds) // 原版delay time 用在thread裡面
{
	ULONG iStart;
	iStart = GetTickCount();
	while (GetTickCount() - iStart <= iMilliSeconds)
		Application->ProcessMessages();
}
//---------------------------------------------------------------------------
// PSU Test
int TFrmMain::DL_CMD(AnsiString CMD)
{
	int Result;
	if(CL_DEV_CONTROL->open_dev_com())
	{
		que_cmd.c.clear();
		char buff[200];
		if(bOverload)
			que_cmd.push("*remote;chan 1;mode cc;pres on;curr:high "+edtSetOverLoadCurrent->Text+"\n");
		else que_cmd.push("*remote;chan 1;mode cc;pres on;curr:high "+edtSetCurrent->Text+"\n");
		que_cmd.push(CMD);
		//測試
		Result = CL_DEV_CONTROL->Dev_CMD_Test() ? TEST_PASS:TEST_FAIL;
		if(CL_DEV_CONTROL->bNoResponse)
		{
			CL_DEV_CONTROL->bCOM_PORT_OPEN = false;
			MessageBox(this->Handle, _T("動態負載機通訊失敗!請檢查線路!"), Application->Title.t_str(), MB_ICONWARNING);
			Result = DL_NOT_FIND;
		}
	}
	else
	{
		MessageBox(this->Handle, _T("動態負載機通訊失敗!請檢查線路!"), Application->Title.t_str(), MB_ICONWARNING);
		DEBUG("ERROR : Not Find Device COM-PORT");
		CL_DEV_CONTROL->bCOM_PORT_OPEN = false;
		Result = DL_NOT_FIND;
	}
	return Result;
}



void __fastcall TFrmMain::plResultDblClick(TObject *Sender)
{
	if(moDebug->Height)
	{
		plResult->Height = 458;
	}
	else
	{
		plResult->Height = 88;
    }
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::edtPassWordKeyPress(TObject *Sender, wchar_t &Key)
{
	if(Key == 13)
		btnSet->Click();
	else
		edtPassWord->Font->Color = clBlue;
}
//---------------------------------------------------------------------------
bool TFrmMain::FindIniFile()
{
	TSearchRec Sr;
	TStringList*FileList=new TStringList();
	if(FindFirst("*.ini",faAnyFile,Sr)==0)
	{
		do
		{
			FileList->Add(Sr.Name);
		}
		while(FindNext(Sr)==0);
		FindClose(Sr);
	}
	if(FileList->Count>1||FileList->Count==0)
	{
		delete FileList;
		plIniName->Font->Color = clRed;
		return false;
	}
	else
	{
		FILE_DUT_SET_INI = FileList->Strings[0];
		if(FILE_DUT_SET_INI.Length()>12)
			plIniName->Caption = FILE_DUT_SET_INI.SubString(8,FILE_DUT_SET_INI.Length()-11);
		delete FileList;
		return true;
	}
}
bool TFrmMain::ReadInISet()
{
	//
	AnsiString value = Findfilemsg(FILE_DUT_SET_INI,"[LossVoltage]",1);
	edtLossVol->Text = value;
	//
	value = Findfilemsg(FILE_DUT_SET_INI,"[DutPluginAutoTest]",1);
	ckbAuto->Checked = value == 1 ? true:false;
	//
	value = Findfilemsg(FILE_DUT_SET_INI,"[DutPidVid]",1);
	edtVID->Text = value.SubString(5,4);
	edtPID->Text = value.SubString(14,4);
	//
	value = Findfilemsg(FILE_DUT_SET_INI,"[OverLoadTest]",1);
	ckbOverLoad->Checked = value == 1 ? true:false;
	pl_OverloadTitle->Visible = ckbOverLoad->Checked ? true:false;
	plOverLoad->Visible = ckbOverLoad->Checked ? true:false;
	plOverLoadRange->Visible = ckbOverLoad->Checked ? true:false;
	plOverLoadRangeTitle->Visible = ckbOverLoad->Checked ? true:false;
	pl_over_load_voltage->Visible = ckbOverLoad->Checked ? true:false;
	pl_NoloadTitle->Height = ckbOverLoad->Checked ? plVoltageBox->Height/3:plVoltageBox->Height/2;
	pl_read_NoLoad_voltage->Height = ckbOverLoad->Checked ? plVoltageBox->Height/3:plVoltageBox->Height/2;
	//
	edtSetCurrent->Text = Findfilemsg(FILE_DUT_SET_INI,"[SetCurrent]",1);
	edtSetOverLoadCurrent->Text = Findfilemsg(FILE_DUT_SET_INI,"[SetOverLoadCurrent]",1);
	//
	value = Findfilemsg(FILE_DUT_SET_INI,"[VoltageToleranceRange]",1);
	edt_min->Text	  = value.SubString(1,value.Pos("、")-1);
	edt_max->Text	  = value.SubString(value.Pos("、")+2,value.Pos("&")-value.Pos("、")-2);
	value = value.Delete(1,value.Pos("&"));
	edt_load_min->Text	  = value.SubString(1,value.Pos("、")-1);
	edt_load_max->Text	  = value.SubString(value.Pos("、")+2,value.Pos("-")-value.Pos("、")-2);
	value = value.Delete(1,value.Pos("-"));
	edt_overload_min->Text	  = value.SubString(1,value.Pos("、")-1);
	edt_overload_max->Text	  = value.SubString(value.Pos("、")+2,value.Length()-value.Pos("、")+2);
	//
	//
	edtLossVol->Text = Findfilemsg(FILE_DUT_SET_INI,"[LossVoltage]",1);
	//
	numNoLoadDelay = Findfilemsg(FILE_DUT_SET_INI,"[NoLoadDelay]",1).ToInt();
	numFullLoadDelay = Findfilemsg(FILE_DUT_SET_INI,"[FullLoadDelay]",1).ToInt();

	return true;
}
bool TFrmMain::SetInIVal(bool AutoTest,AnsiString DutVPID){
	AnsiString VoltageRange =
		AnsiString(edt_min->Text)
		+ "、"
		+ AnsiString(edt_max->Text)
		+ "&"
		+ AnsiString(edt_load_min->Text)
		+ "、"
		+ AnsiString(edt_load_max->Text)
		+ "-"
		+ AnsiString(edt_overload_min->Text)
		+ "、"
		+ AnsiString(edt_overload_max->Text);

	fstream fp;
	fp.open(FILE_DUT_SET_INI.c_str(), ios::out); // 開啟檔案

	fp << "[DutPluginAutoTest]" << endl;
	if(AutoTest) fp << "1" << endl;
	else fp << "0" << endl;
	fp << "[DutPidVid]" << endl;
	if(edtVID->Font->Color==clBlue&&edtPID->Font->Color==clBlue)
		fp << DutVPID.c_str() << endl;
	fp << "[OverLoadTest]" << endl;
	if(ckbOverLoad->Checked) fp << "1" << endl;
	else fp << "0" << endl;
	fp << "[SetOverLoadCurrent]" << endl;
	fp << AnsiString(edtSetOverLoadCurrent->Text).c_str() << endl;
	fp << "[SetCurrent]" << endl;
	fp << AnsiString(edtSetCurrent->Text).c_str() << endl;
	fp << "[VoltageToleranceRange]" << endl;
	fp << VoltageRange.c_str() << endl;
	fp << "[LossVoltage]" << endl;
	fp << AnsiString(edtLossVol->Text).c_str() << endl;
	fp << "[NoLoadDelay]" << endl;
	fp << AnsiString(numNoLoadDelay).c_str() << endl;
	fp << "[FullLoadDelay]" << endl;
	fp << AnsiString(numFullLoadDelay).c_str() << endl;

	fp.close(); // 關閉檔案
}
AnsiString TFrmMain::Findfilemsg(AnsiString filename, AnsiString findmsg,
	int rownum) { // 找檔案找到字串行回傳幾行後的字串
	ifstream lanfile(filename.c_str());
	std::string filemsg;
	if (lanfile.is_open()) {
		while (!lanfile.eof()) {
			getline(lanfile, filemsg);
			if (strstr(filemsg.c_str(), findmsg.c_str())) {
				for (int i = 0; i < rownum; i++)
					getline(lanfile, filemsg);
				lanfile.close();
				return(AnsiString)filemsg.c_str();
			}
		}
		lanfile.close();
		return NULL;
	}
	else
		return "FAIL";
}






void __fastcall TFrmMain::ckbOverLoadClick(TObject *Sender)
{
	pl_OverloadTitle->Visible = ckbOverLoad->Checked ? true:false;
	plOverLoad->Visible = ckbOverLoad->Checked ? true:false;
	plOverLoadRange->Visible = ckbOverLoad->Checked ? true:false;
	plOverLoadRangeTitle->Visible = ckbOverLoad->Checked ? true:false;
	pl_over_load_voltage->Visible =  ckbOverLoad->Checked ? true:false;

	plNoLoadTitle->Width   = ckbOverLoad->Checked ? (pl_set_title->Width-60)/3:(pl_set_title->Width-18)/2;
	plFullLoadTitle->Width = ckbOverLoad->Checked ? (pl_set_title->Width-60)/3:(pl_set_title->Width-18)/2;
	plSpace->Width = ckbOverLoad->Checked ? 18:0;
	plNoLoad->Width 	= ckbOverLoad->Checked ? (pl_set_title->Width-60)/3:(pl_set_title->Width-18)/2;
	plFullLoad->Width   = ckbOverLoad->Checked ? (pl_set_title->Width-60)/3:(pl_set_title->Width-18)/2;
	edt_min->Width 	 =(plNoLoad->Width-30)/2;
	edt_load_min->Width =(plFullLoad->Width-30)/2;
	edt_overload_min->Width	 =(plOverLoadRangeTitle->Width-30)/2;
	pl_NoloadTitle->Height = ckbOverLoad->Checked ? plVoltageBox->Height/3:plVoltageBox->Height/2;
	pl_read_NoLoad_voltage->Height = ckbOverLoad->Checked ? plVoltageBox->Height/3:plVoltageBox->Height/2;
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::FormResize(TObject *Sender)
{

	plNoLoadTitle->Width   = ckbOverLoad->Checked ? (pl_set_title->Width-60)/3:(pl_set_title->Width-18)/2;
	plFullLoadTitle->Width = ckbOverLoad->Checked ? (pl_set_title->Width-60)/3:(pl_set_title->Width-18)/2;
	plSpace->Width = ckbOverLoad->Checked ? 18:0;
	plNoLoad->Width 	= ckbOverLoad->Checked ? (pl_set_title->Width-60)/3:(pl_set_title->Width-18)/2;
	plFullLoad->Width   = ckbOverLoad->Checked ? (pl_set_title->Width-60)/3:(pl_set_title->Width-18)/2;
	edt_min->Width 	 =(plNoLoad->Width-30)/2;
	edt_load_min->Width =(plFullLoad->Width-30)/2;
	edt_overload_min->Width	 =(plOverLoadRangeTitle->Width-30)/2;
	plvotage->Width = Panel1->Width-120;
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::FormKeyPress(TObject *Sender, wchar_t &Key)
{
	if(Key==13||Key==32)
		btnStartClick(NULL);
}
//---------------------------------------------------------------------------

