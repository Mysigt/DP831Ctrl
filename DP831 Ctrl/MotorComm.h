#pragma once
#include "DP831.h"
#include <stdio.h>
#include <stdlib.h>
#include <atlstr.h>

struct Motor
{
	double cLim = 1.8; //Limit as written in documentation for the board
	double vLim = 10; // Default that will be overridden if 0.5 scaling is defined by from user
	double scale, vMin, vMax;
	double incr, measTime;
	int steps;
	CString strInc, strCLim, strVMin, strVLim;
};

void getUserInput(Motor &mtr);

void setStartVal(DPSrc Src, Motor mtr, CString Chan, CString Chan2, int instrNum); //Chan = positive output, Chan 2 = negative output

void angleConstraintX(Motor &mtr);

void angleConstraintY(Motor &mtr);

void dispConf(Motor mtr, Motor mtr2);

