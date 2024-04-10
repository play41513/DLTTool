# DLTTool
 Automation of load machine operation for idle, full load, and overload conditions.

## Program 

### DLT Tool v3.3
- Notices :Copyright(c) 2021 Leno
- Compiler :Embarcadero RAD Studio 2010 Version 14.0.3615.26342
- OS :Windows 8 ver6.3 Build 9600
- Desc :
	- Addition of new load machine model 3316G.

### DLT Tool v3.2
- Notices :Copyright(c) 2021 Leno
- Compiler :Embarcadero RAD Studio 2010 Version 14.0.3615.26342
- OS :Windows 8 ver6.3 Build 9600
- Desc :
	- Correction to voltage judgment mechanism:
		- Previous method: After detecting normal voltage range, three consecutive normal readings were required for a PASS result; otherwise, it was considered a FAIL.
		- Revised method: If the voltage falls out of range in the middle, the counting is reset, and it continues until the timeout is reached, after which it is considered a FAIL.

### DLT Tool v3.1
- Notices :Copyright(c) 2020 Leno
- Compiler :Embarcadero RAD Studio 2010 Version 14.0.3615.26342
- OS :Windows 8 ver6.3 Build 9600
- Desc :
	- Implementation of overload detection mechanism (product uses polling mechanism; OCP_POLLING parameter can be added to the INI file).
	- Further UI adjustments.

### DLT Tool v3.0
- Notices :Copyright(c) 2020 Leno
- Compiler :Embarcadero RAD Studio 2010 Version 14.0.3615.26342
- OS :Windows 8 ver6.3 Build 9600
- Desc :
	- Integration of relay control.
	- UI adjustments.
	- Opening up of detailed test parameters for configuration in the INI file.
	- Addition of loop test functionality, configurable via the INI file.

### DLT Tool v2.2
- Notices :Copyright(c) 2021 Leno
- Compiler :Embarcadero RAD Studio 2010 Version 14.0.3615.26342
- OS :Windows 8 ver6.3 Build 9600
- Desc :
	- Addition of new load machine model 3316G.

### DLT Tool v2.1
- Notices :Copyright(c) 2020 Leno
- Compiler :Embarcadero RAD Studio 2010 Version 14.0.3615.26342
- OS :Windows 8 ver6.3 Build 9600
- Desc :
	- Modification of employee ID conditions in work order settings to accommodate new employee ID rules.

### DLT Tool v2.0
- Notices :Copyright(c) 2020 Leno
- Compiler :Embarcadero RAD Studio 2010 Version 14.0.3615.26342
- OS :Windows 8 ver6.3 Build 9600
- Desc :
	- Switch to barcode scanning for automatic testing.

### DLT Tool v1.2
- Notices :Copyright(c) 2018 Leno
- Compiler :Embarcadero RAD Studio 2010 Version 14.0.3615.26342
- OS :Windows 8 ver6.3 Build 9600
- Desc : 
	- Change in device plug-in/plug-out detection method.

### DLT Tool v1.1
- Notices :Copyright(c) 2018 Leno
- Compiler :Embarcadero RAD Studio 2010 Version 14.0.3615.26342
- OS :Windows 8 ver6.3 Build 9600
- Desc : 
	- After overload test completion, return to idle state.
	- Overload test PASS condition: Current < 0.5A.
	- Added support for additional load machine models 3310F to 3315F.

### DLT Tool v1.0
- Notices :Copyright(c) 2018 Leno
- Compiler :Embarcadero RAD Studio 2010 Version 14.0.3615.26342
- OS :Windows 8 ver6.3 Build 9600
- Desc : 
	- Automation of load machine operation for idle, full load, and overload conditions.