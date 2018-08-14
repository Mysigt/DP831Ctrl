//#include "DP831.h"
//#include <stdio.h>
//#include <iostream>
//#include <string>
//#include <atlstr.h>
//
//using namespace std;
//
//int main()
//{	
//	DPSrc Src;	//Declare source using the class DPSrc
//	Src.Connect(); //Connect to instrument
//	Src.Send(":SYST:REM;"); // Send command to switch system from local to remote
//	Src.Send("SYST:VERS?;"); //Query SCPI version - output is year
//	Src.Read();
//
//	double vMin, vMax, cLim, vLim;
//	double incr, measTime;
//	int steps;
//	CString strInc, strCLim, strVMin, strVLim;
//
//	//User defined params
//	cout << "For multivalue definitions enter the first number press enter and repeat \n";
//	cout << "Define the voltage and current limit: (In that order) \n";
//	cin >> vLim >> cLim;
//	cout << "Define the voltage range: (Enter min then max) \n";
//	cin >> vMin >> vMax;
//	cout << "Define the voltage increment: \n";
//	cin >> incr;
//	cout << "Define the time between measurements: (In milliseconds) \n";
//	cin >> measTime;
//
//	steps = nearbyint((vMax - vMin) / incr) * 2; //Calc. amount of steps - round to nearest even
//	strInc.Format("%.*f;",3,incr); //Converts double to CString with 3 sig. digs
//	strCLim.Format("%.*fA;", 3, cLim);
//	strVLim.Format("%.*f;",3,vLim);
//	strVMin.Format("%.*fV,",3,vMin);
//
//	Src.Send(":SOUR2:CURR:PROT " + strCLim + ":VOLT:PROT " + strVLim);  //Apply cur/volt lim to CH2
//	Src.Send(":APPL CH2,"+ strVMin + strCLim); //Set output voltage for CH2
//	Src.Send("SOUR2:VOLT:IMM:STEP " + strInc); //Set voltage increment
//	
//	Src.Send(":SOUR3:CURR:PROT " + strCLim + ":VOLT:PROT -" + strVLim); //Apply cur/volt lim to CH3
//	Src.Send(":APPL CH3,-" + strVMin + strCLim); //Set output voltage for CH2
//	Src.Send("SOUR3:VOLT:IMM:STEP -" + strInc); //Set voltage increment 
//
//	for (int i = 0; i < steps+1; i++)
//	{
//		if (i == 0) //initial
//		{
//			Src.Send(":OUTP CH2,ON"); //Turn on output from CH2
//			Src.Send(":OUTP CH3,ON;"); //Turn on output from CH3
//			cout << "\nStarting voltage sweep process: \n";
//		}
//		else if (i%2 != 0) //odd iterations
//		{
//			cout << "Step:" << i+1 << '\n';
//			Src.Send(":SOUR2:VOLT UP;"); 
//		}
//		else // even iterations
//		{
//			cout << "Step:" << i+1 << '\n';
//			Src.Send(":SOUR3:VOLT UP;");
//		}
//		Sleep(measTime); // Delay in ms before next command adjust as necessary 
//						 //Transient response time of rigol is 50microseconds
//	}
//
//	Src.Send(":OUTP CH2,OFF; :OUTP CH3,OFF"); //Turn off output from both CH2 and CH3
//	
//}























/*TO DO:
Rewrite file to not use CString ideally... if switch to python is not done instead/before
*/

////Test params
//double vMin = 1;
//double vMax = 10;
//double incr = 1.2; //, measTime
//double cLim = 1.2;
//double vLim = 18;
//int steps;
//CString strInc, strCLim, strVMin, strVLim;
//double measTime = 2000;
//Src.Send(":MEAS:VOLT?;"); //Query voltage
//Src.Read();



//Old code - not used 

//double scale, vMin, vMax, cLim, vLim; //Inputs for first motor
//double scale2, vMin2, vMax2, cLim2, vLim2; //Inputs for second motor
//double incr, incr2, measTime; 
//int steps,deg,deg2;
//CString strInc, strCLim, strVMin, strVLim;
//CString strInc2, strCLim2, strVMin2, strVLim2;

//cout << "USER DEFINED PARAMS FOR MIRROR 1 \n";
//cout << "Define the voltage and current limit: (In that order)";
//cin >> vLim >> cLim;
//cout << "Define the voltage range: (Enter min then max) \n";
//cin >> vMin >> vMax;
//cout << "Define the voltage per degree scale factor: Enter 1 for 0.5 2 for 0.8, 3 for 1 \n";
//cin >> scale;
//cout << "Define the mechanical degree increment:";
//cin >> deg;


//cout << "USER DEFINED PARAMS FOR MIRROR 2 \n";
//cout << "Define the voltage and current limit: (In that order)";
//cin >> vLim2 >> cLim2;
//cout << "Define the voltage range: (Enter min then max) \n";
//cin >> vMin2 >> vMax2;
//cout << "Define the voltage per degree scale factor: Enter 1 for 0.5 2 for 0.8, 3 for 1 \n";
//cin >> scale2;
//cout << "Define the mechanical degree increment:";
//cin >> deg2;


//steps = nearbyint((vMax - vMin) / incr) * 2; //Calc. amount of steps - round to nearest even
//strInc.Format("%.*f;", 3, incr); //Converts double to CString with 3 sig. digs
//strCLim.Format("%.*fA;", 3, cLim);
//strVLim.Format("%.*f;", 3, vLim);
//strVMin.Format("%.*fV,", 3, vMin);

//Src.Send(":SOUR1:CURR:PROT " + Motor1.strCLim + ":VOLT:PROT " + Motor1.strVLim);  //Apply cur/volt lim to CH2
//Src.Send(":APPL CH1," + Motor1.strVMin + Motor1.strCLim); //Set output voltage for CH2
//Src.Send("SOUR1:VOLT:IMM:STEP " + Motor1.strInc); //Set voltage increment

//Src.Send(":SOUR2:CURR:PROT " + Motor2.strCLim + ":VOLT:PROT " + Motor2.strVLim);  //Apply cur/volt lim to CH2
//Src.Send(":APPL CH2," + Motor2.strVMin + Motor2.strCLim); //Set output voltage for CH2
//Src.Send("SOUR2:VOLT:IMM:STEP " + Motor2.strInc); //Set voltage increment

//Src.Send(":SOUR3:CURR:PROT " + Motor2.strCLim + ":VOLT:PROT -" + Motor2.strVLim); //Apply cur/volt lim to CH3
//Src.Send(":APPL CH3,-" + Motor2.strVMin + Motor2.strCLim); //Set output voltage for CH2
//Src.Send("SOUR3:VOLT:IMM:STEP -" + Motor2.strInc); //Set voltage increment 


//else // increment using positive channel
//{
//	cout << "Step:" << i << endl;
//	Src.Send(":OUTP CH2,ON"); //Turn on output from CH2
//	Src.Send(":OUTP CH3,OFF"); //Turn off output from CH3
//}

//if (i % 2 != 0) // odd iterations
//{
//	Src.Send(":SOUR3:VOLT UP;");
//}
//else // even iterations
//{
//
//}