//---------------------------------------------------------------------------

#ifndef mainH
#define mainH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
#include <dbt.h> //DBT_常數 註冊要取得的裝置消息
#include "IO_Request.h"
#include <Buttons.hpp>
#include <Registry.hpp>
#include <ExtCtrls.hpp>
#include <Mask.hpp>
#include <Menus.hpp>
#include <Graphics.hpp>

#include "COM_control.h"
#include "USBDevConnectionInfo.h"
#include "RelayControl.h"
#include "LOGFile.h"
#include <pngimage.hpp>
#include <queue>
#include <fstream.h>
#include <string>
#include <winuser.h>
#include <direct.h>

const AnsiString APP_TITLE = "DLT Tool ver.3.1 (Dynamic load Test Tool, Action Star ,2020)";
const AnsiString PASSWORD  = "SET";

const float UNIT = 0.0195; //單位元電壓值

//GUID
const char GUID_USB_HUB[] = "{F18A0E88-C30C-11D0-8815-00A0C906BED8}";
const char GUID_USB_DEVICE[] = "{A5DCBF10-6530-11D2-901F-00C04FB951ED}";
const char GUID_HID[] = "{4d1e55b2-f16f-11cf-88cb-001111000030}";

//
#define GET_VALUE_TIMEOUT_MS 500
#define TIME_INTERVAL 200
#define HARDWARE_VOLTAGE_COMPENSATION 13

#define STEP_READ_MODE1_VOLTAGE_VALUE		1
#define STEP_WAITTING_AC_ON					2
#define STEP_READ_OTHER_MODE_VOLTAGE_VALUE	3
#define STEP_TEST_VOLTAGE_END				4
#define STEP_DL_NOT_FIND					5
#define STEP_WAITTING_AC_OFF				6
#define STEP_RELAY_NOT_FIND					7

#define TEST_PASS		    9
#define TEST_FAIL		    10
#define BACK_NO_LOAD_VOLTAGE 			12

#define CHECK_DEV_ONLINE	1
#define START_TEST			2
#define CHECK_DEV_OFFLINE	3

#define REBACK_NO_LOAD		    12

#define BARCODE_FINISH	1
#define BARCODE_CHANEL  2
#define STEP_TIMER_STOP	99


#define DEBUG(String)    FrmMain->moDebug->Lines->Add(String)

//---------------------------------------------------------------------------

class TFrmMain : public TForm
{
__published:	// IDE-managed Components
	TPanel *Panel1;
	TPanel *pl_Set;
	TPanel *pl_Debug;
	TMemo *moDebug;
	TPanel *plResult;
	TTimer *Timer1;
	TPanel *Panel6;
	TPanel *plTitle;
	TPanel *plBarcode;
	TLabel *lbBarcode;
	TPanel *Panel13;
	TBitBtn *btnBarcodeChanel;
	TBitBtn *btnBarcodeClear;
	TEdit *edtBarcodeMAC;
	TImage *ImgDisk;
	TTimer *Timer2;
	TLabel *lbTime;
	TPanel *plDUT_SNTitle;
	TPanel *Panel19;
	TPanel *plDUT_SN;
	TPopupMenu *popBackPage;
	TMenuItem *popItem;
	TPanel *plvotage;
	TPanel *plReadLoadVolModeTitle2;
	TPanel *plLoadMode2;
	TPanel *plMode2VolRange;
	TPanel *plVolRangeMode2;
	TEdit *edtMINVol_Mode2;
	TPanel *Panel50;
	TEdit *edtMAXVol_Mode2;
	TPanel *plReadLoadVolModeTitle3;
	TPanel *plLoadMode3;
	TPanel *plFullLoad;
	TPanel *plVolRangeMode3;
	TEdit *edtMINVol_Mode3;
	TPanel *Panel53;
	TEdit *edtMAXVol_Mode3;
	TPanel *plReadLoadVolModeTitle1;
	TPanel *plLoadMode1;
	TPanel *Panel55;
	TPanel *plVolRangeMode1;
	TEdit *edtMINVol_Mode1;
	TPanel *plTitle2;
	TEdit *edtMAXVol_Mode1;
	TPanel *plReadLoadVolModeTitle4;
	TPanel *plLoadMode4;
	TPanel *Panel20;
	TPanel *plVolRangeMode4;
	TEdit *edtMINVol_Mode4;
	TPanel *Panel25;
	TEdit *edtMAXVol_Mode4;
	TPanel *Panel5;
	TPanel *Panel72;
	TEdit *edtSetCurrent1;
	TPanel *Panel73;
	TPanel *Panel14;
	TPanel *Panel24;
	TEdit *edtSetCurrent2;
	TPanel *Panel26;
	TPanel *Panel27;
	TPanel *Panel28;
	TEdit *edtSetCurrent3;
	TPanel *Panel29;
	TPanel *Panel30;
	TPanel *Panel31;
	TEdit *edtSetCurrent4;
	TPanel *Panel32;
	TPanel *Panel8;
	TPanel *plCurrentRangeMode1;
	TEdit *edtMINCurr_Mode1;
	TPanel *Panel10;
	TEdit *edtMAXCurr_Mode1;
	TPanel *Panel11;
	TPanel *plCurrentRangeMode2;
	TEdit *edtMINCurr_Mode2;
	TPanel *Panel18;
	TEdit *edtMAXCurr_Mode2;
	TPanel *Panel21;
	TPanel *plCurrentRangeMode3;
	TEdit *edtMINCurr_Mode3;
	TPanel *Panel23;
	TEdit *edtMAXCurr_Mode3;
	TPanel *Panel33;
	TPanel *plCurrentRangeMode4;
	TEdit *edtMINCurr_Mode4;
	TPanel *Panel35;
	TEdit *edtMAXCurr_Mode4;
	TPanel *plReadLoadVolMode1;
	TPanel *plReadLoadVolMode2;
	TPanel *plReadLoadVolMode3;
	TPanel *plReadLoadVolMode4;
	TPanel *Panel3;
	TPanel *plLoadMode5;
	TPanel *Panel12;
	TPanel *plVolRangeMode5;
	TEdit *edtMINVol_Mode5;
	TPanel *Panel34;
	TEdit *edtMAXVol_Mode5;
	TPanel *Panel36;
	TPanel *Panel37;
	TEdit *edtSetCurrent5;
	TPanel *Panel38;
	TPanel *Panel39;
	TPanel *plCurrentRangeMode5;
	TEdit *edtMINCurr_Mode5;
	TPanel *Panel41;
	TEdit *edtMAXCurr_Mode5;
	TPanel *plReadLoadVolMode5;
	TPanel *Panel9;
	TPanel *plIniName;
	TPanel *plPowerOn;
	TPanel *Panel22;
	TPanel *plPowerOff;
	TPanel *plLossVol;
	TEdit *edtLossVol;
	TPanel *Panel4;
	TPanel *Panel75;
	TPanel *Panel15;
	TImage *Image1;
	TPanel *Panel16;
	TPanel *Panel17;
	TPanel *edtSetEmployeeID;
	TPanel *edtSetWorkOrderNumber;
	TPanel *Panel7;
	TPanel *plStart;
	TPanel *plLoopBack;
	TPanel *Panel44;
	TPanel *Panel45;
	TPanel *Panel43;
	TPanel *Panel46;
	TPanel *Panel47;
	TEdit *plLoopBackTotalCount;
	TPanel *plLoopBackNowCount;
	TPanel *Panel42;

	void __fastcall moDebugChange(TObject *Sender);
	void __fastcall plResultDblClick(TObject *Sender);
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall btnBarcodeChanelClick(TObject *Sender);
	void __fastcall edtBarcodeMACKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall Timer2Timer(TObject *Sender);
	void __fastcall btnBarcodeClearClick(TObject *Sender);
	void __fastcall popItemClick(TObject *Sender);
	void __fastcall plStartClick(TObject *Sender);
	void __fastcall plStartMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall plStartMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall plPowerOnClick(TObject *Sender);
	void __fastcall plPowerOffClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);



private:	// User declarations
	void Voltage_Test();
	void SetValueEnabled(bool Enabled);

	int  DL_CMD(DWORD dwIndex);

	void ClearUI();
	void ShowSettingForm();

	void SetForegroundWindowInternal(HWND hWnd);
	void SetCOMPortINISetting(AnsiString astrCOMPort);
	bool FindIniFile();
	bool ReadInISet();

	AnsiString FILE_DUT_SET_INI;
	AnsiString LOGFilePath,LOGDiskPath;

	DWORD numBarcodeResult;
	DWORD dwResultUI_Time;
	DWORD dwModeIndex;
	DWORD dwLoadTimeOut[5],dwModePowerResetDelay[5],dwPollingOCPdenominator[5];
	bool bModeEnable[5],bModePowerReset[5],bPollingOCP[5];

public:		// User declarations

	bool g_bError;
	cCOM 	 *CL_DEV_CONTROL;
	CLOGFile *LOGFile;
	cRelayControl *CL_RELAY;
	USBDevConnectionInfo *CL_USB;

	void Delay(ULONG iMilliSeconds);
	__fastcall TFrmMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFrmMain *FrmMain;
//---------------------------------------------------------------------------
#endif
