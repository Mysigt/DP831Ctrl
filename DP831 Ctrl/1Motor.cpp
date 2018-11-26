#include <stdio.h>
#include <iostream>
#include <atlstr.h>
#include <sstream>
#include <iomanip>
#include "DP831.h"
#include "MotorComm.h"
#include "SerialPort.h"

//To control motor 1, make sure that 2nd arg in send func is 0
//For motor 2, 2nd arg is 1
//Channel 2 is negative for both sources 

using namespace std;

int main()
{
	DPSrc Src;	//Declare source using the class DPSrc
	Src.Connect(); //Connect to instrument
	Src.Send(":SYST:REM;",0); // Send command to switch system from local to remote
	Src.Send("SYST:VERS?;",0); //Query SCPI version - output is year 
	Src.Read(0); //Displays output from query

	Src.Send(":SYST:REM;", 1); // Send command to switch system from local to remote
	Src.Send("SYST:VERS?;", 1); //Query SCPI version - output is year 
	Src.Read(1); //Displays output from query

	//const char *portName = "\\\\.\\COM3"; //Keep backslashes  portname + change if using diff port
	//SerialPort *arduino;

	//arduino = new SerialPort(portName);
	//const char* input = "1";
	//if (arduino->isConnected())
	//	cout << "Connection eablished" << endl;
	//else
	//	cout << "ERROR: Check port name" << endl << endl;

	Motor Motor1, Motor2;
	char Confirm = 'N'; 
	char UserDef; //Change to another character if you want to define during run-time
	while (Confirm != 'Y' && Confirm != 'y')
	{
		char resp;
		cout << "USER DEFINED PARAMS: Supply limits, voltage range, voltage degree scale factor, degree increment, measurement time" 
		        "\nMotor 1 controls X-axis movement, Motor 2 controls Y-axis movement"
				"\nWould you like to use default params(1) or set them yourself?";
		cin >> UserDef;
		if (UserDef == '1')
		{
			//FILL IN DEFAULT TEST SCRIPT
			Motor1.measTime = 0; //Delay between increment in ms
			Motor1.cLim = 1; // Amps
			Motor1.incr = 0.5; //Volts
			Motor1.scale = 1; //Volt/ degree
			Motor1.steps = 10;  
			Motor1.vMin = -3; //Volts
			Motor1.vMax = 3; //Volts

			Motor2.measTime = 0;
			Motor2.cLim = 1;
			Motor2.incr = 0.1;
			Motor2.scale = 0.8;
			Motor2.steps = 10;
			Motor2.vMin = 1;
			Motor2.vMax = 10;
		

			Motor1.strInc.Format("%.*f;", 3, Motor1.incr); //Converts double to CString with 3 sig. digs
			Motor1.strCLim.Format("%.*fA;", 3, Motor1.cLim);
			//Motor1.strVLim.Format("%.*f;", 3, Motor1.vLim);

			Motor2.strInc.Format("%.*f;", 3, Motor2.incr); //Converts double to CString with 3 sig. digs
			Motor2.strCLim.Format("%.*fA;", 3, Motor2.cLim);
			//Motor2.strVLim.Format("%.*f;", 3, Motor2.vLim);
		}
		else 
		{ 
			cout << "\nWill the same parameters be used for both mirrors? Y/N" << endl;
			cin >> resp;
			if (resp == 'Y' || resp == 'y')
			{
				getUserInput(Motor1); //Get user defined params
				Motor2 = Motor1; // Copy params to second motor
			}
			else
			{
				getUserInput(Motor1); //Get user defined params for motor 1
				getUserInput(Motor2); //Get user defined params for motor 2
			}
		}
		angleConstraintX(Motor1); //constrain x-direction
		angleConstraintY(Motor2); //constrain y-direction
		dispConf(Motor1, Motor2);

		cout << "\nConfirm to proceed: \nEnter Y to confirm \nElse press another key to prompt re-entry of parameters" << endl;
		cin >> Confirm;
	}

	setStartVal(Src, Motor1, "1", "2",0); //Initialize all starting values and limits for motor 1 - note: this does not turn on any of the outputs 
	setStartVal(Src, Motor2, "1", "2",1); //Initialize all starting values and limits for motor 2

	int ChConf = 2; //Default is 2
	//Based on user defined range find the case for motor 2 
	//We do this here instead of later as we do not want to turn the channels off during measurement
	if (Motor2.vMin < 0 && Motor2.vMax <= 0) //Negative to negative voltage
	{
		ChConf = 1;
		Src.Send(":SOUR2:VOLT:IMM:STEP " + Motor2.strInc,1); // Set voltage increment
		Src.Send(":OUTP CH2,ON;",1);//Turn on output from CH2 - should be a negative volt min
	}
	else if (Motor2.vMin < 0 && Motor2.vMax > 0) //Negative to positive voltage
	{
		Src.Send(":OUTP CH2,ON;",1);//Turn on output from CH2 - should be a negative volt min
		Src.Send(":OUTP CH1,ON;",1); //Turn on output from CH1 - should be 0
	}
	else //Positive to positive voltage
	{
		ChConf = 3;
		Src.Send(":OUTP CH1,ON;",1);//Turn on output from CH1 - should be a positive volt min
	}
	Src.Send(":OUTP CH1,ON",0); //Turn on output from CH1 of source 1 - should be 0 volts or positive
	Src.Send(":OUTP CH2,ON", 0); //Turn on output from CH2 of source 1 

	//Loop to handle raster scan
	cout << "\n\nSTARTING SCAN \n";
	for (int i = 0; i < Motor1.steps; i++) //Iterate through x-axis positions
	{	
		//arduino->writeSerialPort("1", MAX_DATA_LENGTH); //Turns on pin 8 - Line marker
		cout << "X Direction - Step: " << i * Motor2.steps 
			<< "/" << Motor1.steps*Motor2.steps << endl; // Just to keep track
		if (i != 0) // Turn on positive supply during initial step
		{
			Src.Send(":SOUR1:VOLT UP;",0); // Increment for each change in i
		}

		for (int j = 0; j < Motor2.steps+1; j++) //MOTOR 2 LOOP
		{
			cout << right << setfill(' ') <<setw(27) << "Y Direction - Step: " << (i*Motor2.steps) + j
				 << "/" << Motor1.steps*Motor2.steps << endl; //Just to keep track

			//arduino->writeSerialPort("2", MAX_DATA_LENGTH); //Turns on pin 8 - Line marker//start of frame  - trigger marker 1 (pin 9) 

			if (j != 0) // increment
			{
				switch (ChConf)
				{
				case 1: // Negative volt to negative or zero volt 
					if (Motor2.vMin + (j-1) * Motor2.incr < Motor2.vMax)
					{
						//arduino->writeSerialPort("2", MAX_DATA_LENGTH); //trigger pin 9 - frame marker
						Src.Send(":SOUR2:VOLT DOWN;",1); //increase voltage out from ch3
					}
					else //step down voltage to initial before next line of scanning starts
					{
						for (int k = 0; k < j - 1; k++)
						{
							Src.Send(":SOUR2:VOLT UP;",1); //decrease voltage out from ch3 to start val
						}
					}
					break;
				default:	// Positive/zero volt to positive volt
					if (Motor2.vMin + (j-1) * Motor2.incr < Motor2.vMax)
					{
						//arduino->writeSerialPort("2", MAX_DATA_LENGTH); //trigger pin 9 - frame marker
						Src.Send(":SOUR1:VOLT UP;",1);
					}
					else //step down voltage to initial before next line of scanning starts
					{
						for (int k = 0; k < j - 1; k++)
						{
							Src.Send(":SOUR1:VOLT DOWN;",1);
						}
					}
					break;
				}
			}
			Sleep(Motor2.measTime); //Wait time until voltage increment for motor 2 - change in y
		}
		Sleep(Motor1.measTime); // Wait time until voltage increment for motor 1 - change in x 
		//arduino->writeSerialPort("3", MAX_DATA_LENGTH); //trigger pin 10 - end of line marker
	}

	for (int k = 0; k < Motor1.steps -1; k++)
	{
		Src.Send(":SOUR1:VOLT DOWN;",0);
	}

	Src.Send(":OUTP CH1,OFF; :OUTP CH2,OFF;", 0); //Turn off output from all CH 
	Src.Send(":OUTP CH1,OFF; :OUTP CH2,OFF;", 1);
}

//Right now fully functional so long as voltage range is perfectly divisible by increment
//Need to fix for all scenarios