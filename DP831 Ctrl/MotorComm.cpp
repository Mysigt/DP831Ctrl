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
		cout << "Maximum scanning angle is +/- 12.5 degrees for SF 0.5 V/degree and 0.8V/degree"<< endl;
		cout << "Maximum scanning angle is +/- 10 degrees for SF of 1V/degree"<< endl << endl;
		cout << "Define the mechanical movement range: " << endl; //finish comment about info
		cin >> degMin >> degMax;
		cout << "Define the mechanical degree increment: (Min value: 0.004 degrees)"; //max direction in x is +/- 8 degrees for 5mm beam diameter
		cin >> deg;

		mtr.incr = deg * mtr.scale;
		mtr.vMin = degMin * mtr.scale;
		mtr.vMax = degMax * mtr.scale;
	}
	else
	{
		cout << "Voltage limit for 0.5V/degree is +/- 6.25V" << endl;
		cout << "Voltage limit for 0.8V/degree and  1V/degree is +/- 10V" << endl << endl;
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

void setStartVal(DPSrc Src, Motor mtr, CString Chan, CString Chan2, int instrNum)
{
	Src.Send(":SOUR" + Chan + ":CURR:PROT " + mtr.strCLim + ":VOLT:PROT " + mtr.strVLim, instrNum); // Apply volt/curr protection to chan
	if (mtr.vMin > 0) //check for case where using only positive values
	{
		Src.Send(":APPL CH" + Chan + "," + + mtr.strVMin + mtr.strCLim, instrNum); // Set starting output voltage for chan
	}
	else
	{
		Src.Send(":APPL CH" + Chan + ",0," + mtr.strCLim, instrNum); // Set starting output voltage for chan
	}
		Src.Send(":SOUR" + Chan + ":VOLT:IMM:STEP " + mtr.strInc, instrNum); // Set voltage increment

	//to handle the negative part of the voltage sweep
	if (Chan2 != "0")
	{
		Src.Send(":SOUR" + Chan2 + ":CURR:PROT " + mtr.strCLim + ":VOLT:PROT -" + mtr.strVLim, instrNum); //Negative is necessary as limit is defined as a positve 
		Src.Send(":APPL CH" + Chan2 + "," + mtr.strVMin + mtr.strCLim, instrNum);
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
	if (fmod((mtr.vMax - mtr.vMin), mtr.incr) > 0.0000000000000001) 
	{
		int temp;
		temp = nearbyint((mtr.vMax - mtr.vMin) / mtr.incr);
		mtr.incr = (mtr.vMax - mtr.vMin) / temp;
		cout << endl << "Increment did not provide integer number of steps, increment was modified to " << mtr.incr << endl;
	}
}

void angleConstraintX(Motor &mtr)
{
	if (mtr.vMax > 8 * mtr.scale)
	{
		mtr.vMax = 8 * mtr.scale;
	}
	if (mtr.vMin < -8 * mtr.scale)
	{
		mtr.vMin = -8 * mtr.scale;
	}
}

void angleConstraintY(Motor &mtr)
{
	if (mtr.vMin < -3 * mtr.scale)
	{
		mtr.vMin = -3 * mtr.scale;
	}
}

void dispConf(Motor mtr, Motor mtr2)
{
	int totalRunTime; //total time to execute measurement
	totalRunTime = ((mtr.measTime * mtr.steps) + (mtr2.measTime * mtr2.steps)) / 1000; //time in seconds

	cout << "\n\nSummary of parameters:" << endl << endl;
	cout << left << setw(107) << setfill('=') << "=" << endl;
	printf("|%29s%25s%29s%23s\n", "Motor 1", "| |", "Motor 2", "|");
	cout << left << setw(107) << setfill('=') << "=" << endl;

	printf("| Voltage sweep: %+ 7.3fV -> %+ 7.3fV %16s", mtr.vMin, mtr.vMax, "|");
	printf(" Voltage sweep: %+ 7.3fV -> %+ 7.3fV %16s\n", mtr2.vMin, mtr2.vMax, "|");

	printf("| Degree sweep: %+7.3f -> %+ 7.3f %19s", mtr.vMin*mtr.scale, mtr.vMax * mtr.scale, "|");
	printf(" Degree sweep: %+ 7.3f -> %+ 7.3f %19s\n", mtr2.vMin* mtr2.scale, mtr2.vMax* mtr2.scale, "|");

	printf("| Scale: %.1f V/degree %32s Scale: %0.1f V/degree %32s\n", mtr.scale, "|", mtr2.scale, "|");

	printf("| Increment: %.3fV = %.3f degrees %18s", mtr.incr, mtr.incr*mtr.scale, "|");
	printf(" Increment: %.3fV = %.3f degrees %18s\n", mtr2.incr, mtr2.incr*mtr2.scale, "|");

	printf("| Steps: %i %42s Steps: %i %42s\n", mtr.steps, "|", mtr2.steps, "|");
	cout << left << setw(107) << setfill('=') << "=" << endl << endl;

	cout << "Total number of combined steps: " << mtr.steps * mtr2.steps << endl;
	printf("Scan time: %ih:%im:%is \n", totalRunTime / 3600, (totalRunTime % 3600) / 60, (totalRunTime % 3600) % 60);
}


//To do:
//Do number of steps instead of increment
