/*
    Errors - Error reporting.       
    Copyright (C) 1999 Computer Inspirations
    All rights reserved.
 
    This source code is part of the BeCalc program and is the exclusive 
    property of Computer Inspirations.  Duplication, alteration, distribution, 
    or copying of this source code in any form is not permitted.
*/

#include <String.h>
#include "Errors.h"

static char *ErrorS[] = {
	"Okay",
  	"Illegal expression",
 	"Illegal variable",
	"Too many variables", 
	"Illegal operator",
	"Undefined variable",
	"Mismatched braces",
 	"Illegal number entered", 
	"Arithmetic overflow",
	"Arithmetic underflow",
	"Division by zero",
 	"Too few digits selected",
	"Too many digits selected",
	
	"Illegal number",
	"Illegal factorial",
	"Illegal arctan argument",
	"Pi uninitialized",
	"Illegal arcsin argument",
	"Illegal arccos argument",
	"Exceeded iteration limit in SinCos",
	"Ln(2) uninitialized",
	"Exp argument is too large",
	"Exceeded iteration limit in Exp",
	"Illegal ln argument",
	"Entier/RoundInt argument is too large",
	"Illegal power argument",
	"Illegal root",
	"Exponent underflow",
	"Exponent overflow",
	"Illegal root argument"
};
static int32 prevError = -1;

void Error(const char *text, alert_type type)
{
	BAlert *alert = new BAlert("Error", text, "Continue", NULL, NULL, B_WIDTH_AS_USUAL, type);
	alert->Go();
};

void Error(errType err, int32 errpos, const char *str, alert_type type)
{	
	// if (errpos - prevError < 10) return;	
	
	BString eStr(str);
	eStr.Append("\n"); eStr.Append(ErrorS[err]);
	eStr.Insert("«", errpos-1);
	prevError = errpos;
	BAlert *alert = new BAlert("Error", eStr.String(), "Continue", NULL, NULL, B_WIDTH_AS_USUAL, type);
	alert->Go();
};
