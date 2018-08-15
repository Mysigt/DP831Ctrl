/*
Write file description here later :3
*/

#include "MotorComm.h"
#include "DP831.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <iomanip>

using namespace std;

void errorHandling(Motor &mtr); //local function 

void getUserInput(Motor &mtr)
{
	cout << "\nUSER DEFINED PARAMS FOR MOTOR \n";
	cout << "Define the time between measurements: (In milliseconds) \n";
	cin >> mtr.measTime;
	cout << "Define the voltage per degree scale factor: (Options: 0.5, 0.8, or 1) \n";
	cin >> mtr.scale;

	if (mtr.scale == 0.5)
	{
		mtr.vLim = 6.25; 
	}
	
	char select;
	cout << "Would you like to define in terms of voltage or degrees? V/D \n";
	cin >> select;

	if (select == 'D' || select == 'd') //not case sensitive check for degree pref
	{
		double deg, degMin, degMax;
		cout << "Define the mechanical movement range: " << endl << "Max angles: +12.5...." << endl; //finish comment about info
		cin >> degMin >> degMax;
		cout << "Define the mechanical degree increment: (Min value: 0.004 degrees)"; //max direction in x is +/- 8 degrees for 5mm beam diameter
		cin >> deg;

		mtr.incr = deg * mtr.scale;
		mtr.vMin = degMin * mtr.scale;
		mtr.vMax = degMax * mtr.scale;
	}
	else
	{
		cout << "Define the voltage range: \n"; 
		cin >> mtr.vMin >> mtr.vMax;
		cout << "Define the voltage increment: (Min value:" << mtr.scale*0.004 << ")" << endl;
		cin >> mtr.incr;
	}

	errorHandling(mtr); //check user inputs

	mtr.steps = nearbyint((mtr.vMax - mtr.vMin) / mtr.incr)+1; //Calc. amount of steps - round to nearest even

	mtr.strInc.Format("%.*f;", 3, mtr.incr); //Converts double to CString with 3 sig. digs
	mtr.strCLim.Format("%.*fA;", 3, mtr.cLim);
	mtr.strVLim.Format("%.*f;", 3, mtr.vLim);
	mtr.strVMin.Format("%.*fV,", 3, mtr.vMin);
}

void setStartVal(DPSrc Src, Motor mtr, CString Chan, CString Chan2)
{
	Src.Send(":SOUR" + Chan + ":CURR:PROT " + mtr.strCLim + ":VOLT:PROT " + mtr.strVLim); // Apply volt/curr protection to chan
	if (mtr.vMin > 0) //check for case where using only positive values
	{
		Src.Send(":APPL CH" + Chan + "," + + mtr.strVMin + mtr.strCLim); // Set starting output voltage for chan
	}
	else
	{
		Src.Send(":APPL CH" + Chan + ",0," + mtr.strCLim); // Set starting output voltage for chan
	}
		Src.Send(":SOUR" + Chan + ":VOLT:IMM:STEP " + mtr.strInc); // Set voltage increment

	//to handle the negative part of the voltage sweep
	if (Chan2 != "0")
	{
		Src.Send(":SOUR" + Chan2 + ":CURR:PROT " + mtr.strCLim + ":VOLT:PROT -" + mtr.strVLim); //Negative is necessary as limit is defined as a positve 
		Src.Send(":APPL CH" + Chan2 + "," + mtr.strVMin + mtr.strCLim);
	}
}

void errorHandling(Motor &mtr)
{
	if (abs(mtr.scale - 0.5) >= 0.00001 && abs(mtr.scale - 0.8) >= 0.00001)
	{
		if (abs(mtr.scale - 1) >= 0.00001)
		{ 
			cout << "Selected scale factor not defined, cannot predict value - rerun with proper parameter";
			Sleep(3500);
			exit(EXIT_FAILURE);
		}
	}
	if (mtr.vMin > mtr.vMax)
	{
		double temp;
		temp = mtr.vMin;
		mtr.vMin = mtr.vMax;
		mtr.vMax = temp;
	}
	if (mtr.vMax < mtr.vMin)
	{
		double temp;
		temp = mtr.vMax;
		mtr.vMax = mtr.vMin;
		mtr.vMin = temp;
	}
	if (mtr.vMax > mtr.vLim)
	{
		cout << endl <<"Maximum voltage exceeds voltage limit, setting max to "<< mtr.vLim << endl;
		mtr.vMax = mtr.vLim;
	}
	if (abs(mtr.vMin) > mtr.vLim && mtr.vMin < 0)
	{
		cout << endl << "Minimum voltage exceeds voltage limit, setting min to " << -mtr.vLim << endl;
		mtr.vMin = -mtr.vLim;
	}
	if (mtr.incr < 0.004 * mtr.scale)
	{
		cout << endl << "Increment is below defined limit, setting increment to 0.004 degrees"<< endl;
		mtr.incr = 0.004 * mtr.scale;
	}
	if (fmod((mtr.vMax - mtr.vMin), mtr.incr) > 0.00000000000000000000000001) 
	{
		int temp;
		temp = nearbyint((mtr.vMax - mtr.vMin) / mtr.incr);
		mtr.incr = (mtr.vMax - mtr.vMin) / temp;
		cout << endl << "Increment did not provide integer number of steps, increment was modified to " << mtr.incr << endl;
	}
}

void dispConf(Motor mtr, Motor mtr2)
{
	const char sep = ' ';
	const int nameWidth = 8;
	const int numWidth = 48;

	cout << fixed;
	cout.precision(3);
	cout << "\n\nSummary of parameters:" << endl << endl;
	cout << setw(25) << "Motor 1" << setw(25) << "| |" << setw(25) << "Motor 2" << endl;
	cout << left << setw(100) << setfill('=') << "=" << endl;
	cout << left << setw(nameWidth) << setfill(sep) << "Minimum voltage:" << mtr.vMin << "V  ";
	cout << left << setw(nameWidth) << setfill(sep) << "Maximum voltage:" << mtr.vMax << "V  ";
	cout << setw(2) << setfill(sep) << "|" << setw(nameWidth+1) << setfill(sep);

	cout << left << setw(nameWidth) << setfill(sep) << "Minimum voltage:" << mtr2.vMin << "V  ";
	cout << left << setw(nameWidth) << setfill(sep) << "Maximum voltage:" << mtr2.vMax << "V" << endl;

	cout << "Increment:" << mtr.incr << "V";
	cout << right << setw(numWidth - 14) << setfill(sep) << "| ";
	cout << left << "Increment:" << mtr2.incr << "V" << endl;

	cout << "Steps:" << mtr.steps;
	cout << right << setw(numWidth - 5) << setfill(sep) << "| ";
	cout << "Steps:" << left << mtr2.steps << endl;
}


//To do:
//Fix increment - so that precision is at 3
//Do number of steps instead of increment