/*
Write file description here later :3
*/


#include "MotorComm.h"
#include "DP831.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

using namespace std;
void errorHandling(Motor &mtr);

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
		cout << "Define the mechanical movement range: (Enter min then max in degrees)" << endl << "Max angles: +12.5...." << endl; //finish comment about info
		cin >> degMin >> degMax;
		cout << "Define the mechanical degree increment: (Min value: 0.004 degrees)"; //max direction in x is +/- 8 degrees for 5mm beam diameter
		cin >> deg;

		mtr.incr = deg * mtr.scale;
		mtr.vMin = degMin * mtr.scale;
		mtr.vMax = degMax * mtr.scale;
	}
	else
	{
		cout << "Define the voltage range: (Enter min then max) \n"; 
		cin >> mtr.vMin >> mtr.vMax;
		cout << "Define the voltage increment: (Min value:" << mtr.scale*0.004 << ")" << endl;
		cin >> mtr.incr;
	}

	errorHandling(mtr); //check user inputs

	mtr.steps = nearbyint((mtr.vMax - mtr.vMin) / mtr.incr)+1; //Calc. amount of steps - round to nearest even
	if (mtr.vMin < 0 && mtr.vMax > 0) //If 2 channels are needed to handle positive and negative
	{
		mtr.stepCross= floor((abs(mtr.vMin))/mtr.incr)+1; //find increments to crossover point
		mtr.vCross = mtr.vMin + (mtr.incr*mtr.stepCross);
	}
	else
	{
		mtr.vCross = mtr.vMin;
	}

	mtr.strInc.Format("%.*f;", 3, mtr.incr); //Converts double to CString with 3 sig. digs
	mtr.strCLim.Format("%.*fA;", 3, mtr.cLim);
	mtr.strVLim.Format("%.*f;", 3, mtr.vLim);
	mtr.strVMin.Format("%.*fV,", 3, mtr.vMin);
	mtr.strVCross.Format("%.*fV,", 3, mtr.vCross);
}

void setStartVal(DPSrc Src, Motor mtr, CString Chan, CString Chan2)
{
	Src.Send(":SOUR" + Chan + ":CURR:PROT " + mtr.strCLim + ":VOLT:PROT " + mtr.strVLim); // Apply volt/curr protection to chan
	if (mtr.vCross == mtr.vMin)
	{
		Src.Send(":APPL CH" + Chan + "," + + mtr.strVMin + mtr.strCLim); // Set starting output voltage for chan
	}
	else
	{
		Src.Send(":APPL CH" + Chan + "," + mtr.strVCross + mtr.strCLim); // Set starting output voltage for chan
	}
		Src.Send(":SOUR" + Chan + ":VOLT:IMM:STEP " + mtr.strInc); // Set voltage increment

	//to handle the negative part of the voltage sweep
	if (Chan2 != "0")
	{
		Src.Send(":SOUR" + Chan2 + ":CURR:PROT " + mtr.strCLim + ":VOLT:PROT -" + mtr.strVLim); //Negative is necessary as limit is defined as a positve 
		Src.Send(":APPL CH" + Chan2 + "," + mtr.strVMin + mtr.strCLim);
		Src.Send(":SOUR" + Chan2 + ":VOLT:IMM:STEP " + mtr.strInc);
	}
}


void errorHandling(Motor &mtr)
{
	if (abs(mtr.scale - 0.5) >= 0.00001 || abs(mtr.scale - 0.8) >= 0.00001)
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
		cout << "Minimum voltage exceeds maximum voltage, setting min to -10V"<< endl;
		mtr.vMin = -mtr.vLim;
	}
	if (mtr.vMax > mtr.vLim)
	{
		cout << "Maximum voltage exceeds voltage limit, setting max to "<< mtr.vLim << endl;
		mtr.vMax = mtr.vLim;
	}
	if (abs(mtr.vMin) > mtr.vLim && mtr.vMin < 0)
	{
		cout << "Minimum voltage exceeds voltage limit, setting min to " << -mtr.vLim << endl;
		mtr.vMax = -mtr.vLim;
	}
	if (mtr.incr < 0.004 * mtr.scale)
	{
		cout << "Increment is below defined limit, setting increment to 0.004 degrees"<< endl;
		mtr.incr = 0.004 * mtr.scale;
	}
	if (fmod((mtr.vMax - mtr.vMin), mtr.incr) > 0.00000000000000000000000001) 
	{
		int temp;
		temp = nearbyint((mtr.vMax - mtr.vMin) / mtr.incr);
		mtr.incr = (mtr.vMax - mtr.vMin) / temp;
		cout << "Increment did not provide integer number of steps, increment was modified to " << mtr.incr << endl;
	}
}

//To do:
//Fix increment - so that precision is at 3
//Before running increment output all the user selected params
//Do number of steps instead of increment