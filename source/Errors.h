/*
    Errors - Error reporting.       
    Copyright (C) 1999 Computer Inspirations
    All rights reserved.
 
    This source code is part of the BeCalc program and is the exclusive 
    property of Computer Inspirations.  Duplication, alteration, distribution, 
    or copying of this source code in any form is not permitted.
*/

#ifndef ERRORS_H
#define ERRORS_H

#include <Alert.h>

enum errType {
	// misc errors
	errNone = 0,
 	IllegalExpression,
  	IllegalVariable,
  	TooManyVariables,
  	IllegalOperator, 		// illegal operator was used 
  	UndefinedStorage,  		// undefined storage location was used
  	MismatchBraces,
  	IllegalNumber,
  	ArithmeticOverflow,
  	AirthmeticUnderflow,
  	DivisionByZero,
  	TooFewDigits,
  	TooManyDigits,
  	
  	// Real errors 
    errIllegalNumber,   	// illegal number format in string
    errIllegalFactorial,	// illegal factorial was used
    errIllegalAtanArgs,		// both ATan2 arguments are zero
    errPiValueMissing,		// pi value missing in ATan2
    errIllegalASinArg,		// illegal arcsin argument
    errIllegalACosArg,		// illegal arccos argument
    errSinIterExceeded,		// SinCos iteration limit was exceeded
    errLn2ValueMissing,		// ln(2) value is missing in exp
    errExpArgTooLarge,		// exp argument is too large
    errExpIterExceeded,		// exp iteration limit was exceeded
    errIllegalLnArg,		// ln argument <= 0
    errArgTooLarge,			// argument too large for integer in Entier/RoundInt
    errIllegalPowerArg,		// arg = 0 and power <= 0 in IntPower
    errIllegalRoot,			// illegal root <= 0 or >= maxN
    errExpUnderflow,		// exponent underflow
    errExpOverflow,			// exponent overflow
    errNegArgument			// argument is negative in Sqrt/Root
};
  
	void Error(const char *text, alert_type type = B_WARNING_ALERT);
	void Error(errType err, int32 errpos, const char *str, alert_type type = B_WARNING_ALERT);

#endif
