/*
 * Scanner.h - Scanner class header file
 */

#ifndef SCANNER_H
#define SCANNER_H

#include "Errors.h"
#include "Reals.h"
#include "ExComplex.h"
#include "ExIntegers.h"

#define KW		100

enum Tokens {
	/* Symbols */
	Empty = 0,

	/* Expression tokens */
	Plus,
	Minus,
	Or,
	Nor,
	Xor,
	Store,

	/* Term tokens */
	Times = 10,
	Divide,
	Div,
	Mod,
	And,
	Nand,
	ShiftRight,
	AShiftRight,
	RotateRight,
	ShiftLeft,
	RotateLeft,
	ClearBit,
	SetBit,
	ToggleBit,
	nCr,
	nPr,
	
	/* Power tokens */
	Power = 30,
	Root,
	Squared,
	Cubed,
	Inverse,
	Factorial,
	PercentOf,
	PolarToRect,
	
	/* Factor tokens */
	LeftBrace = 40,
	RightBrace,
	Number,
	Complement,
	Sin,
	Cos,
	Tan,
	Cot,
	ArcSin,
	ArcCos,
	ArcTan,
	ArcCot,
	Sinh,
	Cosh,
	Tanh,
	Coth,
	ArcSinh,
	ArcCosh,
	ArcTanh,
	ArcCoth,
	SquareRoot,
	CubeRoot,
	NaturalLog,
	Log,
	Log2,
	PowerOfe,
	Var,
	Base,
	Digits,
	Decimals,
	Notation,
	DegRadGrad,
	Plot,
	Fib,
	GCD,
	iToken,
	rToken,
	Theta,
	ImagPart,
	RealPart,
	Convert,
	InvConvert,
	IntPart,
	FracPart,
	SignOf,
	Abs,
	Count,
	tMin,
	tMax,
	Conj,
	Rand,
	Semi,
	Pi,
	Equals
};


/* Scanner class definition */

class Scanner {

public:
	friend class Equations;
	static Complex val;
	static BString var;
	static bool error;

	static void InitScanner(void);		// Constructor

	/* Public functions */
	static void Mark(errType errid);
	static void Init(const char *str);
	static void Get(Tokens &sym);
	static void UngetToken ();
	
private:
	typedef 	struct {
		Tokens sym;
		char id[8];
	} keyTabType;

	/* private variables */
	static unsigned char ch[8], prevch[8];
	static short errpos, prev, pos, nkw;
	static BString pline;
	static keyTabType keyTab[KW];
	static bool initialized;

	/* Private functions */
	static void Read(void);
	static long LocateChar(const char *str, char ch, long start);
	static void EnterKW(Tokens sym, const char *name);
	
	/* Private functions used by Get() */
	static void number(Tokens &sym);
	static void Illegal(void);
	static void Variable(Tokens &sym);
};

#endif
