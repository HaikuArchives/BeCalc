#include <limits.h>
#include <stdio.h>
#include <String.h>
#include "MListView.h"
#include "BinaryRider.h"
#include "Buttons.h"
#include "Errors.h"
#include "Reals.h"
#include "ExIntegers.h"
#include "ExComplex.h"
#include "Scanner.h"
#include "Equations.h"
#include "Variables.h"

/* Private static variables */
extern BPath prefDir;					// defined in BeCalc.cpp
BString Equations::CommandLine;
Real Equations::One;
Real Equations::Zero;
	
/* Public static variables */

Tokens Equations::Token;
MyListView * Equations::Equation;
MyListView * Equations::Results;
ConvertWin *Equations::convWin = NULL;
const char * Equations::EquationFile = "equations";
const char * Equations::ResultFile = "results"; 

float ListItem::maxWidth = 32.0;	

// define the constructor and writer for ListItem
ListItem::ListItem (const Preference *r) : MyItem(r)
{
	const char *str;
	r->FindString(LabelStr(), &str);
	content.SetTo(str);
}

bool ListItem::Write (Preference *w)
{
	MyItem::Write(w);   // write parent fields
	w->AddString(LabelStr(), content.String());
	return true;
}

static BListItem * New (Preference *r)
{
	return (new ListItem(r));
}

void Equations::InitSettings()
{	
	Settings::SetInt(EQUNLIMITED, B_CONTROL_ON);
	Settings::SetInt(EQLIMIT, 0);
	Settings::SetInt(EQUNIQUE, B_CONTROL_OFF);
	Settings::SetInt(EQDELETE, B_CONTROL_OFF);
}

// Equation - related methods
void Equations::InitEquations()
{
	Real::Init();
	Complex::InitComplex();
	Integer::Init();
	Scanner::InitScanner();

	Token = Empty;
	Zero = Real::Long(0);
	One = Real::Long(1);
	
	// create empty lists
	Equation = new MyListView(EQNAME, EquationFile, New);
	Results = new MyListView(RESNAME, ResultFile, New);
	if (!Settings::GetInt(EQUNLIMITED)) {
		int32 limit = Settings::GetInt(EQLIMIT);
		Equation->SetListLimit(limit);
		Results->SetListLimit(limit);
	}
	Variables::Read();
}

void Equations::CloseEquations()
{
	// output the equation and result contents to disk
	Equation->Write(EquationFile);
	Results->Write(ResultFile);
	Variables::Write();
}

/* Private static methods */

#define Next {Scanner::Get(Token); Factor(Result);}
#define Comma \
{ \
	Scanner::Get(Token); \
	if(Token != LeftBrace) \
		Scanner::Mark(MismatchBraces); \
	Scanner::Get(Token); \
	Factor(tmp); \
	if(Token != Semi) \
		Scanner::Mark(IllegalOperator); \
	Scanner::Get(Token); \
	Factor(Result); \
	if(Token != RightBrace) \
		Scanner::Mark(MismatchBraces); \
	Scanner::Get(Token); \
}
#define Fix {tmp = Complex(t, Zero);}
#define FixR {Result = Complex(t, Zero);}

static Real Min(const Real& x, const Real& y)
{
	return Real::cmp(x, y) < 0 ? x : y;
}

static Real Max(const Real& x, const Real& y)
{
	return Real::cmp(x, y) > 0 ? x : y;
}

void Equations::Factor(Complex& Result)
{
	Complex tmp;
	Real t;
	bool ok, inv = false;
	
	switch(Token)
	{
		case LeftBrace:
			Scanner::Get(Token); Expression(Result);
			if(Token == RightBrace)
				Scanner::Get(Token);
			else
				Scanner::Mark(MismatchBraces);
			break;
		case Number:
			Scanner::Get(Token); Result = Scanner::val;
			if(Token == Number)
				Scanner::Mark(IllegalNumber);
			break;		
		case Minus:		Next; Complex::ChgSign(Result); break;
		case Pi:				Scanner::Get(Token); Result = Complex(Real::pi, Zero); break;
		case Complement:	Next; Integer::OnesComp(t, Result.RealPart()); FixR; break;
		case Sin: 			Next; Complex::sin(Result, Result); break;
		case Cos:			Next; Complex::cos(Result, Result); break;
		case Tan:			Next; Complex::tan(Result, Result); break;
		case Cot:			Next; Complex::cot(Result, Result); break;
		case ArcSin:		Next; Complex::arcsin(Result, Result); break;
		case ArcCos:		Next; Complex::arccos(Result, Result); break;
		case ArcTan:		Next; Complex::arctan(Result, Result); break;
		case ArcCot:		Next; Complex::arccot(Result, Result); break;
		case Sinh:			Next; Complex::sinh(Result, Result); break;
		case Cosh:		Next; Complex::cosh(Result, Result); break;
		case Tanh:		Next; Complex::tanh(Result, Result); break;
		case Coth:			Next; Complex::coth(Result, Result); break;
		case ArcSinh:		Next; Complex::arcsinh(Result, Result); break;
		case ArcCosh:	Next; Complex::arccosh(Result, Result); break;
		case ArcTanh:	Next; Complex::arctanh(Result, Result); break;
		case ArcCoth:	Next; Complex::arccoth(Result, Result); break;
		case SquareRoot:Next; Complex::sqrt(Result, Result); break;
		case CubeRoot:	Next; t = Real::Long(3); Fix; Complex::root(Result, Result, tmp); break;
		case NaturalLog:Next; Complex::ln(Result, Result); break;
		case Log:			Next; Complex::log(Result, Result); break;
		case Log2:		Next; Complex::log2(Result, Result); break;
		case PowerOfe:	Next; Complex::exp(Result, Result); break;
		case Count:		Next; Integer::Count(t, Result.RealPart()); FixR; break;

		case Fib:			Next; Integer::Fib(t, Result.RealPart()); FixR; break;
		case GCD:			Comma; Integer::GCD(t, tmp.RealPart(), Result.RealPart()); FixR; break;
		case InvConvert: inv = true;
		case Convert:	Next; t = Result.RealPart();
								if (convWin != NULL) convWin->DoConvert(t, inv);
								else Error("Conversion window needs to be open!");
								FixR; break;
		case Var:			Variables::Get(Scanner::var.String(), Result, ok); Scanner::Get(Token); 
						if (!ok) Scanner::Mark(UndefinedStorage);
						break;
		case iToken:
			Scanner::Get(Token);
			Result = Complex(Zero, One);
			break;
		case rToken:		Next; Complex::PolarMag(t, Result); FixR; break;
		case Theta:		Next; Complex::PolarAngle(t, Result); Result=Complex(t, Zero);
					Complex::FromRadians(Result); break;
		case ImagPart:	Next; Result = Complex(Result.ImagPart(), Zero); break;
		case RealPart:	Next; Result = Complex(Result.RealPart(), Zero); break;
		case IntPart:		Next; t = Real::entier(Result.RealPart()); FixR; break;
		case FracPart:	Next; t = Real::fraction(Result.RealPart()); FixR; break;
		case SignOf:
			Next;
			if(Real::sign(Result.RealPart()) >= 0)
				Result = Complex(One, Zero);
			else
			{
				Result = Complex(One, Zero);
				Complex::ChgSign(Result);
			}
			break;
		case Abs: Next; Complex::PolarMag(t, Result); FixR; break;
		case tMin:	Comma; t = Min(tmp.RealPart(), Result.RealPart()); FixR; break;
		case tMax:	Comma; t = Max(tmp.RealPart(), Result.RealPart()); FixR; break;
		case Conj:	Next; Complex::Conj(Result);
		case Rand:	Scanner::Get(Token); t = Real::random(); FixR; break;
		default:	Scanner::Mark(IllegalOperator); Result = Complex::Ex0; break;
	}
	
	if(Real::err != errNone)
		Scanner::Mark(Real::err);
}

#ifdef Next
#undef Next
#define Next {Scanner::Get(Token); Factor(Result);}
#endif

void Equations::Powers(Complex& Result)
{
	Complex tmp;

	Factor(tmp);
	while((Token >= Power) && (Token <= PolarToRect))
	{
		switch(Token)
		{
			case Power:		Next; Complex::power(tmp, tmp, Result); break;
			case Root:			Next; Complex::root(tmp, Result, tmp); break;
			case Squared:	Scanner::Get(Token); Complex::Mult(tmp, tmp, tmp); break;
			case Cubed:		Scanner::Get(Token); Complex::xtoi(tmp, tmp, 3); break;
			case Inverse:		Scanner::Get(Token); Complex::Div(tmp, Complex::Ex1, tmp); break;
			case Factorial:	Scanner::Get(Token); Complex::factorial(tmp, floor(Real::Short(tmp.RealPart()))); break;
			case PercentOf:
				Scanner::Get(Token);
				tmp = Complex(Real::div(One, Real::Long(100)), Zero);
				Complex::Mult(Result, tmp, Result);
				Factor(tmp);
				Complex::Mult(tmp, tmp, Result);
				break;
			case PolarToRect:
				Next;
				Complex::ToRadians(Result);
				Complex::ToRectangular(Result, tmp.RealPart(), Result.RealPart());
				break;
			default:
				Scanner::Mark(IllegalOperator);
				Scanner::Get(Token);
				break;
		}
	}
	Result = tmp;
	if(Real::err != errNone)
		Scanner::Mark(Real::err);
}

#ifdef Next
#undef Next
#define Next {Scanner::Get(Token); Powers(Result);}
#endif

#ifdef Fix
#undef Fix
#define Fix {tmp = Complex(t, Zero);}
#endif

static int ToCard(Complex& Ex)
{
	return (int)floor(Real::Short(Ex.RealPart()));
}

void Equations::Term(Complex& Result)
{
	Complex tmp;
	Real t;
	
	Powers(tmp);
	while((Token >= Times) && (Token <= nPr))
	{
		switch(Token)
		{
			case Times:		Next; Complex::Mult(tmp, Result, tmp); break;
			case Divide:		Next; Complex::Div(tmp, tmp, Result); break;
			case Div:			Next; Integer::IntDiv(t, tmp.RealPart(), Result.RealPart()); Fix; break;
			case Mod:			Next; Integer::Mod(t, tmp.RealPart(), Result.RealPart()); Fix; break;
			case And:			Next; Integer::And(t, tmp.RealPart(), Result.RealPart()); Fix; break;
			case Nand:		Next; Integer::Nand(t, tmp.RealPart(), Result.RealPart()); Fix; break;
			case ShiftRight:	Next; Integer::Shr(t, tmp.RealPart(), ToCard(Result)); Fix; break;
			case AShiftRight:Next; Integer::Ashr(t, tmp.RealPart(), ToCard(Result)); Fix; break;
			case RotateRight:Next; Integer::Ror(t, tmp.RealPart(), ToCard(Result)); Fix; break;
			case ShiftLeft:		Next; Integer::Shl(t, tmp.RealPart(), ToCard(Result)); Fix; break;
			case RotateLeft:Next; Integer::Rol(t, tmp.RealPart(), ToCard(Result)); Fix; break;
			case ClearBit:		Next; Integer::ClearBit(t, tmp.RealPart(), ToCard(Result)); Fix; break;
			case SetBit:		Next; Integer::SetBit(t, tmp.RealPart(), ToCard(Result)); Fix; break;
			case ToggleBit:	Next; Integer::ToggleBit(t, tmp.RealPart(), ToCard(Result)); Fix; break;
			case nCr:			Next; t = Real::combinations(tmp.RealPart(), Result.RealPart()); Fix; break;
			case nPr:			Next; t = Real::permutations(tmp.RealPart(), Result.RealPart()); Fix; break;
			default:
				Scanner::Mark(IllegalOperator);
				Scanner::Get(Token);
				break;
		}
	}
	Result = tmp;
	if(Real::err != errNone) {
		Scanner::Mark(Real::err);
	}
}

/* Public static methods */

#ifdef Next
#undef Next
#define Next(RESULT) {Scanner::Get(Token); Term(RESULT);}
#endif

#ifdef Fix
#undef Fix
#define Fix {tmp = Complex(t, Zero);}
#endif

void Equations::Expression(Complex& Result)
{
	Complex tmp;
	Real t;
	Term(tmp); 
	while((Token >= Plus) && (Token <= Store))
	{
		switch(Token)
		{
			case Plus:	Next(Result); Complex::Add(tmp, tmp, Result); break;
			case Minus:	Next(Result); Complex::Sub(tmp, tmp, Result); break;
			case Or:	Next(Result); Integer::Or(t, Result.RealPart(), tmp.RealPart()); Fix; break;
			case Nor:	Next(Result); Integer::Nor(t, Result.RealPart(), tmp.RealPart()); Fix; break;
			case Xor:	Next(Result); Integer::Xor(t, Result.RealPart(), tmp.RealPart()); Fix; break;
			case Store:
				Scanner::Get(Token);
				if(Token == Var)
					Variables::Set(Scanner::var.String(), tmp);
				else
					Scanner::Mark(IllegalVariable);
				Scanner::Get(Token);
				break;
			default:	Term(tmp); break;
		}
	}
	Result = tmp;
	if(Real::err != errNone) {
		Scanner::Mark(Real::err);
	}
}


ListItem* Equations::Found(const MListView* l, const char *arg)
{
	int32 pos = 0;
	ListItem *item;
	
	while (true) {
		item = cast_as(l->ItemAt(pos), ListItem);
		if (item == NULL) return NULL;
		if (strcmp(item->ContentStr(), arg) == 0) return item;
		pos++;
	}	
	return NULL;
}

void Equations::Function (Complex &Result)
{
	// variable or function definition
	if (Token == Var) { 
		BString name(Scanner::var); Scanner::Get(Token);
		if (Token == Equals) {
			// variable definition
			Scanner::Get(Token);
			Expression(Result);
			Variables::Set(name.String(), Result);
		} else {
			Scanner::UngetToken(); 
			Token = Var;
			Scanner::var.SetTo(name);
			Expression(Result);
		}
	} else Expression(Result);
}

static inline long Max(long X, long Y)	{return (X > Y ? X : Y);}

bool Equations::Evaluate(const char *arg, Complex& Result, bool listUpdate)
{	
 	ListItem *item;
	CommandLine.SetTo(arg);
	Real::err = errNone;
	Scanner::Init(arg);
	Scanner::Get(Token);
	if (Token == Empty) {
		Result = Complex::Ex0;
		return true;
	}
	Function(Result);
	if (Token != Empty) {
		Scanner::Mark(IllegalExpression);
		return false;
	} else if (listUpdate && (Real::err == errNone) && !Scanner::error) {
		int32 max = Max(Equation->FindMaxIndex (), Results->FindMaxIndex());
		item = Found(Equation, arg);
		if ((item == NULL) || (!IsUnique())) {
			// append a new equation to the list
			Equation->AddItem(new ListItem(max+1, arg));
			Equation->Select(Equation->CountItems()-1);
			Equation->ScrollToSelection();
		}
		
		// append a new result to the list
		BString str;
		Complex::ComplexToStr(Result, str);
		item = Found(Results, arg);
		if ((item == NULL) || (!IsUnique())) 
			Results->AddItem(new ListItem(max+1, str.String()));
		else {
			int32 index = Equation->IndexOf(item);
			item = (ListItem *)Results->ItemAt(index);
			item->SetContent(str.String());
			Results->InvalidateItem(index);
		}
		Results->Select(Results->CountItems()-1);
		Results->ScrollToSelection();
	} 	
	return ((Real::err == errNone) && (!Scanner::error));
}
