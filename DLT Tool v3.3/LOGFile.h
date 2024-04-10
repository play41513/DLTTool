//---------------------------------------------------------------------------

#ifndef LOGFileH
#define LOGFileH

#include <vcl.h>
#include <direct.h>
#include <stdio.h>
//---------------------------------------------------------------------------
class CLOGFile
{
	private:
		AnsiString LOGDiskPath,LOGFilePath,astrLOGContent;
		DWORD	   dwTestCount;
		bool SaveLogLine(AnsiString FileName,AnsiString FileLine);

	public:
		CLOGFile::CLOGFile(void);

		AnsiString astrWorkOrderNumber,astrEmployeeID,astrErrorMsg;
		AnsiString logName,logSN;
		void FindLogFile(TImage* ImgDisk);
		void NewFilePath(AnsiString Path);
		void ClearLOGContent();
		void AddLOGContent(AnsiString strClass,AnsiString strContent);

		bool CheckDiskName();
		bool writeLOG(AnsiString Msg,TImage* ImgDisk,TMemo* moDebug);
};
#endif
