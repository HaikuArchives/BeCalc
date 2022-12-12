/*
 * Scanner.cpp - Scanner class methods
 *
 */

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <String.h>
#include "ExComplex.h"
#include "Errors.h"
#include "ExIntegers.h"
#include "Reals.h"
#include "Scanner.h"

#define isalphanum (isalnum(ch[0]))

// static variables
Complex Scanner::val;
BString Scanner::var;
bool Scanner::error;

// private variables
unsigned char Scanner::ch[8], Scanner::prevch[8];     // need to do this to recognize UTF characters
short Scanner::errpos;
short Scanner::pos, Scanner::prev;
short Scanner::nkw;
BString Scanner::pline;
Scanner::keyTabType Scanner::keyTab[KW];

bool Scanner::initialized = false;

/* Constructor */

void Scanner::InitScanner(void)
{
	if (initialized) return;
	initialized = true;
	error = false;
	errpos = 0;
	pos = 0;
	nkw = 0;
	
	Real::Init();
	Complex::InitComplex();
	Integer::Init();

	EnterKW(nCr, "nCr");
	EnterKW(nPr, "nPr");
	EnterKW(FracPart, "frac");
	EnterKW(Mod, "mod");
	EnterKW(tMax, "max");
	EnterKW(tMin, "min");
	EnterKW(ImagPart, "im");
	EnterKW(Count, "cnt");
	EnterKW(IntPart, "int");
	EnterKW(Or, "or");
	EnterKW(Nor, "nor");
	EnterKW(Xor, "xor");
	EnterKW(Complement, "not");
	EnterKW(Pi, "pi");
	EnterKW(And, "and");
	EnterKW(Nand, "nand");
	EnterKW(AShiftRight, "asr");
	EnterKW(Abs, "abs");
	EnterKW(Log, "log10");
	EnterKW(Log2, "log2");
	EnterKW(NaturalLog, "log");
	EnterKW(RealPart, "re");
	EnterKW(Root, "root");
	EnterKW(RotateLeft, "rol");
	EnterKW(RotateRight, "ror");
	EnterKW(Rand, "rand");
	EnterKW(Div, "div");
	EnterKW(SetBit, "sbit");
	EnterKW(ShiftRight, "shr");
	EnterKW(ShiftLeft, "shl");
	EnterKW(SquareRoot, "sqrt");
	EnterKW(SignOf, "sign");
	EnterKW(ClearBit, "cbit");
	EnterKW(Fib, "fib");
	EnterKW(GCD, "gcd");
	EnterKW(CubeRoot, "cbrt");
	EnterKW(ToggleBit, "tbit");
	EnterKW(InvConvert, "convi");
	EnterKW(Convert, "conv");
	EnterKW(Conj, "conj");
	EnterKW(ArcSinh, "asinh");
	EnterKW(ArcCosh, "acosh");
	EnterKW(ArcTanh, "atanh"); 
	EnterKW(ArcCoth, "acoth");
	EnterKW(ArcSin, "asin");
	EnterKW(ArcCos, "acos");
	EnterKW(ArcTan, "atan"); 
	EnterKW(ArcCot, "acot"); 
	EnterKW(Sinh, "sinh");
	EnterKW(Cosh, "cosh");
	EnterKW(Tanh, "tanh");
	EnterKW(Coth, "coth");
	EnterKW(Sin, "sin");
	EnterKW(Cos, "cos");
	EnterKW(Tan, "tan");
	EnterKW(Cot, "cot");
}


void Scanner::Mark(errType errid)
{
	if(pos > errpos) Error(errid, pos, pline.String());
	errpos = pos;
	error = true;
}


void Scanner::Read(void)
{
	short lpos = 0;
	
	// scan and recognize UTF characters
	do {
		ch[lpos++] = pline.ByteAt(pos++);		// Get next character from input
	} while ((pline.ByteAt(pos) & 0xC0) == 0x80);
	ch[lpos] = 0;
}


void Scanner::Init(const char *str)
{
	pline.SetTo(str);		// Copy input string
	errpos = 0;
	pos = 0;
	error = false;			// No errors (yet...)
	Read();			// Read the first character
}


long Scanner::LocateChar(const char *str, char ch, long start)
{	
	// 'start' is ignored...

	char *ps;
	long pos;

	if((ps = strchr(str, ch)) != NULL)
	{
		pos = ps - str;
		return pos;
	}
	return -1;
}


void Scanner::number(Tokens &sym)
{
	BString constant("");
	char numchars[32] = "";
	BString numberchars(".E0123456789ABCDEF");
	char punctuationchars[] = ",\'_";
	long ps = 0;
	Real num(0.0);
	Real zero(0.0);
	int32 base = Complex::GetBase();

	sym = Number;
	
	// Check if decimal point is a comma
	if ((Complex::GetDigitSep() == '.') || (Complex::GetFracSep() == '.'))
	{
		punctuationchars[0] = '.';
		numberchars.ReplaceFirst('.', ',');
	}
	
	// check if first digit is zero or this is a based number
	if (ch[0] == '.') base = 10;
	else if (ch[0] == '0') {
		Read();
		switch (ch[0]) {
			case 'b': Read(); base = 2; break;
			case 't': Read(); base = 3; break;
			case 'o': Read(); base = 8; break;
			case 'd': Read(); base = 10; break;
			case 'u': Read(); base = 12; break;
			case 'x': Read(); base = 16; break;
			default: break; // just a number
		}
	}

	// Valid number characters
	if (base == 10) numberchars.CopyInto(numchars, 0, 12);
	else numberchars.CopyInto(numchars, 2, base);
		
	// Get a number string from the input
	// number() procedure start
	while(1)
	{
		if(LocateChar(numchars, ch[0], 0) >= 0)
		{
			constant.Append(ch[0], 1);
			if ((ch[0] == 'E') && (Complex::GetBase() == 10)) {
				strcpy(numchars, "+-0123456789");
			} else if ((ch[0] == '+') || (ch[0] == '-'))
				strcpy(numchars, "0123456789");
			Read();
		}
		else if(LocateChar(punctuationchars, ch[0], 0) >= 0)
			Read();
		else
			break;

		if(ch[0] == '\0')
			break;
	}
	// number() procedure end

	constant.Append('\0', 1);	// Terminate new string
	
	// Exchange the decimal point -- if needed
	if(numberchars[0] == ',')
	{
		ps = LocateChar(constant.String(), ',', 0);
		if(ps >= 0)
			constant.ReplaceFirst(',', '.');
	}

	// Convert to a Complex number
	if (constant.Length() > 0) {
		// UnsignInt() procedure start
		if (base == 10)
			num = Real::ToReal(constant.String());
		else
			Integer::StrToInt(constant.String(), base, num);
		
		if (Real::err == errNone)
			val = Complex(num, zero);
		else {
			val = Complex::Ex0;
			Mark(IllegalNumber);
		}
		// UnsignInt() procedure end

		if (ch[0] == 'i') {
			val = Complex(zero, val.RealPart());
			Read();
		}
	} else {
		val = Complex::Ex0;
		Mark(IllegalNumber);	// Illegal number or constant
	}
}


void Scanner::Illegal(void)
{
	Mark(IllegalOperator);
	Read();
}


void Scanner::Variable(Tokens &sym)
{
	short k = 0;
	const char *lch;
	
	var.SetTo("");
	do {
		short ic = 0;
		while (ch[ic] != 0) var.Append(ch[ic++], 1);
		Read();
	} while (isalphanum);
	
	var.Append('\0', 1);	// Terminate variable name
	lch = var.String();
	
	while((k < nkw) && (var.Compare(keyTab[k].id) != 0))
		k++;
		
	sym = (k < nkw) ? keyTab[k].sym : Var;
}


void Scanner::UngetToken ()
{
	errpos = 0; pos = prev;
	for (int32 i = 0; i<8; i++) ch[i]=prevch[i];
	error = false;
}


void Scanner::Get(Tokens &sym)
{	
	/* Skip whitespace */
	prev = pos; for (int32 i = 0; i<8; i++) prevch[i]=ch[i];
	while((ch[0] != '\0') && (ch[0] <= ' ') && (ch[1] == 0)) Read();
	
	if(ch[0] == '\0') sym = Empty;
	else {
		if(isdigit(ch[0]) || (ch[0] == '.'))
			number(sym);
		else switch(ch[0])
		{
			case '+': 	Read(); sym = Plus; break;
			case '-': 	Read(); sym = Minus; break;
			case '=': Read(); sym = Equals; break;
			case '*': Read();
						if (strcmp((const char*)ch, "√") == 0) {
							Read(); sym = Root;
						} else if (ch[0] == '*') {
							Read(); sym = Power;
						} else {
							sym = Times;
						}
						break;
			case 0xCF:
						if (strcmp((const char*)ch, "π") == 0) {
							Read(); sym = Pi;
						}
						break;
			case 0xC3:
						if (strcmp((const char*)ch, "×") == 0) {
							Read(); sym = Times;
						} else if (strcmp((const char*)ch, "÷") == 0) {
							Read(); sym = Divide;
						} else if (strcmp((const char*)ch, "ø") == 0) {
							Read(); sym = Theta;
						}
						break;
			case 0xC2:
						if (strcmp((const char*)ch, "¯") == 0) {
							// start of inverse
							Read();
							if (strcmp((const char*)ch, "¹") == 0) {
								Read(); sym = Inverse;
							}
						} else if (ch[1] == 0xB2) {
							Read(); sym = Squared;
						} else if (ch[1] == 0xB3) {
							Read();
							if (strcmp((const char*)ch, "√") == 0) {
								sym = CubeRoot; Read();
							} else {
								sym = Cubed;
							}
						}
						break;
			case 0xE2:
						if (strcmp((const char*)ch, "√") == 0) {
							sym = SquareRoot; Read();
						} else if (strcmp((const char*)ch, "−") == 0) {
							sym = Minus; Read();
						}
						break;
			case 'i':	Read();
						if(isalphanum) { pos--; ch[0] = 'i'; Variable(sym); }
						else sym = iToken; break;
			case 'r':	Read();
						if(isalphanum) { pos--; ch[0] = 'r'; Variable(sym); }
						else sym = rToken; break;
			case '~':	Read(); sym = Complement; break;
			case ';':	Read(); sym = Semi; break;
			case '/':	Read(); sym = Divide; break;
			case '(':	Read(); sym = LeftBrace; break;
			case ')':	Read(); sym = RightBrace; break;
			case '^':	Read(); sym = Power; break;
			case '%':	Read(); sym = Mod; break;
			case '!':	Read(); sym = Factorial; break;
			case '&':	Read(); sym = And; break;
			case '|':	Read(); sym = Or; break;
			case 'e':	Read(); sym = Number;
						if(ch[0] == '^') { Read(); sym = PowerOfe; }
						else if(isalphanum) { pos--; ch[0] = 'e'; Variable(sym); }
						else val = Complex(Real::exp(Real::Long(1)), Real::Long(0)); break;
			default:	// Covers the rest of the characters
						Variable(sym);
						if ((sym == Sin) || (sym == Cos) || (sym == Tan) ||
								(sym == Sinh) || (sym == Cosh) || (sym == Tanh) ||
								(sym == Coth) || (sym == Cot)) {
									if (strcmp((const char*)ch, "¯") == 0) {
										Read();
										if (strcmp((const char*)ch, "¹") == 0) {
											// convert to arc function
											Read(); sym =  (Tokens)((long)sym + 4);
										}
									}
						}
		}
	}
}


void Scanner::EnterKW(Tokens sym, const char *name)
{
	keyTab[nkw].sym = sym;
	strcpy(keyTab[nkw].id, name);
	nkw++;
}
