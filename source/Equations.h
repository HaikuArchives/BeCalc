#ifndef EQUATIONS_H
#define EQUATIONS_H

#include <String.h>

#include "ListItem.h"
#include "MyListView.h"
#include "Reals.h"
#include "Conversion.h"
#include "ExIntegers.h"
#include "ExComplex.h"
#include "Scanner.h"
#include "Settings.h"


#define EQNAME  "Equations"
#define RESNAME  "Results"

#define EQLIMIT 			"History limit is:"
#define EQUNLIMITED 	"History is unlimited"
#define EQUNIQUE 		"Only add unique equations to list"
#define EQDELETE 		"Delete both equations and results"

class Equations
{
public:
	static Tokens Token;
	static MyListView *Equation, *Results;
	static ConvertWin *convWin;						// need to call conversion routine	
	
	static void Expression(Complex& Result);
	static ListItem* Found(const MListView* l, const char *arg);
	static bool Evaluate(const char *arg, Complex& Result, bool listUpdate = true);
	static void SetUnique (bool unique) {Settings::SetInt(EQUNIQUE, unique);}
	static bool IsUnique() {return Settings::GetInt(EQUNIQUE);}
	static bool IsDeleteBoth() {return Settings::GetInt(EQDELETE);}
	static bool IsUnlimited() {return Settings::GetInt(EQUNLIMITED);}
	static int32 HistoryLimit() {return Settings::GetInt(EQLIMIT);}
	static void InitSettings();
	static void InitEquations();	
	static void CloseEquations();			
	
private:	
	static BString CommandLine;					// main command line
	static const char * EquationFile;
	static const char * ResultFile;
	static Real One, Zero;
	
	static void Factor(Complex& Result);
	static void Powers(Complex& Result);
	static void Term(Complex& Result);
	static void Function(Complex & Result);
};

#endif
