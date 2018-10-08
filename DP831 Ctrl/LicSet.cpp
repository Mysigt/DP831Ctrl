//#include "DP831.h"
//
//int main()
//{
//	DPSrc Src;	//Declare source using the class DPSrc
//	Src.Connect(); //Connect to instrument
//	Src.Send(":SYST:REM;"); // Send command to switch system from local to remote
//	Src.Send("SYST:VERS?;"); //Query SCPI version - output is year 
//	Src.Read();
//
//	Src.Send(":LIC:SET PutLicenseHere;");
//	Src.Send(":LIC:SET PutLicenseHere;");
//	Src.Send(":LIC:SET PutLicenseHere;");
//	Src.Send(":LIC:SET PutLicenseHere;");
//	Src.Send(":LIC:SET PutLicenseHere;");
//
//	Src.Send("*OPT?;"); //Query options
//	Src.Read(); //Displays installed options
//}