#include <stdio.h>
#include <String.h>

#include "ExIntegers.h"
#include "Reals.h"

uint32 Integer::LogZero[LogicalSize + 1];
Real Integer::One;
Real Integer::Two;
Real Integer::Zero;
short Integer::Cnt;
bool Integer::initialized = false;


int32 Integer::MaxBits()
{	
	char str[32];
	sprintf(str, "1E%d", Real::sigDigs);
	Real max = Real::ToReal(str) - One;	
	int32 bits = (int32)Real::Short(Real::log(max, Two));
	return (bits & (~0x3));  // multiple of four bits
}

Real Integer::Max ()
{	
	return (Real::power(Two, Real::Long(MaxBits())) - One);
}

void Integer::Init()
{
	if(initialized)
		return;
		
	initialized = true;

	Real::Init();
	
	Zero = Real::Long(0);
	One = Real::Long(1);
	Two = Real::Long(2);
	
	for (Cnt = 0; Cnt <= LogicalSize; Cnt++) LogZero[Cnt] = 0;
}


/*
 * Private methods
 */
inline uint32 Integer::AndSet(uint32 op1, uint32 op2)
{
	return op1 & op2;
}

inline uint32 Integer::NandSet(uint32 op1, uint32 op2)
{
	return ~(op1 & op2);
}

inline uint32 Integer::AndNotSet(uint32 op1, uint32 op2)
{
	return op1 & ~op2;
}


inline uint32 Integer::OrSet(uint32 op1, uint32 op2)
{
	return op1 | op2;
}

inline uint32 Integer::NorSet(uint32 op1, uint32 op2)
{
	return ~(op1 | op2);
}

inline uint32 Integer::XorSet(uint32 op1, uint32 op2)
{
	return op1 ^ op2;
}

inline uint32 Integer::NotSet(uint32 op1, uint32 op2)
{
	return ~op1;
}

inline bool Integer::IsZero(const Real& x)
{
	return Real::sign(x) == 0;
}

	
/* Misc. local functions */
void Integer::ConstrainNum(Real& Number)
{
	Real MaxNumber = Max();
	Real MinNumber = -MaxNumber-One;
		
	if (Real::cmp(Number, MaxNumber) > 0)
		Number = Real::Copy(MaxNumber);
	else if (Real::cmp(Number, MinNumber) < 0)
		Number = Real::Copy(MinNumber);
}


void Integer::NumToLogical(const Real& Numb, Logical & logical)
{
	Real DivScale;
	Real Scale;
	Real Temp;
	Real Temp2;
	Real iNumb(Numb);
	short LogCnt;
	
	ConstrainNum(iNumb);
	if (Real::sign(iNumb) < 0) iNumb = Max() + iNumb + One; 
	
	Scale = Real::Long(65536);
	DivScale = Real::div(One, Scale);
	
	LogCnt = 0;
	for (int32 i = 0; i <= LogicalSize; i++) logical[i] = LogZero[i];
	while(!IsZero(iNumb))
	{
		Temp2 = Real::entier(Real::mul(iNumb, DivScale));
		Temp = Real::sub(iNumb, Real::mul(Temp2, Scale));
		if (LogCnt > LogicalSize) return;
		logical[LogCnt] = (uint32) Real::Short(Temp);
		iNumb = Temp2;
		LogCnt++;
	}
}


void Integer::LogicalToNum(const Logical & logical, Real& Numb)
{
	Real Scale(65536.0);
	short LogCnt;
	int32 INumb;
	
	// determine the maximum representable bits in current Reals
	int32 words = MaxBits() / 16;
	
	Numb = Real::Copy(Zero);
	for (LogCnt = words-1; LogCnt >= 0; LogCnt--) {
		Numb = Real::mul(Numb, Scale);
		INumb = logical[LogCnt];
		if (INumb < 0) INumb += 0x10000;
		Numb = Real::add(Numb, Real::Long(INumb));
	}
}


void Integer::LOp(Real& Result, const Real& op1, LogicalProc Oper, const Real& op2)
{
	short i;
	Logical Lop1, Lop2;
	
	NumToLogical(op1, Lop1);
	NumToLogical(op2, Lop2);
	
	for(i = 0 ; i <= LogicalSize ; i++)
		Lop2[i] = Oper(Lop1[i], Lop2[i]);
		
	LogicalToNum(Lop2, Result);
}

void Integer::LOp1(Real& Result, LogicalProc Oper, const Real& op)
{
	short i;
	Logical Lop;
	
	NumToLogical(op, Lop);
	
	for(i = 0 ; i <= LogicalSize ; i++)
		Lop[i] = Oper(Lop[i], 0);
		
	LogicalToNum(Lop, Result);
}

void Integer::LBit(Real& Result, const Real& number, LogicalProc Oper, short bitnum)
{
	Real inumber(number);
	ConstrainNum(inumber);
	
	if(bitnum > MaxBits())
	{
		Result = inumber;
		return;
	}
	
	LOp(Result, inumber, Oper, Real::power(Two, Real::Long(bitnum)));
}


bool Integer::Bit(Real& number, short bitnum)
{
	ConstrainNum(number);
	
	if (bitnum >= MaxBits()) return false;
		
	And(number, number, Real::power(Two, Real::Long(bitnum)));
	
	return !IsZero(number);
}


void Integer::LShift(Real& Result, const Real& number, ExNumbProc ExOper, short bits)
{
	Real inumber(number);
	ConstrainNum(inumber);
	
	if (bits > MaxBits()) {
		Result = Real::Copy(Zero);
		return;
	}
	
	Result = ExOper(inumber, Real::power(Two, Real::Long(bits)));
	
	ConstrainNum(Result);
}


void Integer::LRotate(Real& Result, const Real& number, bool Shiftright, short bits)
{
	short ShiftCnt;
	bool SavedBit;
	Real Half;
	Real inumber(number);
	
	ConstrainNum(inumber);
	
	bits = bits % (MaxBits() + 1);
	Half = Real::Long(0.5);
	
	for(ShiftCnt = 1 ; ShiftCnt <= bits ; ShiftCnt++)
	{
		if(Shiftright)
		{
			SavedBit = Bit(inumber, 0);
			
			inumber = Real::entier(Real::mul(inumber, Half));
			if(SavedBit)
				SetBit(inumber, inumber, MaxBits() - 1);
		}
		else
		{
			SavedBit = Bit(inumber, MaxBits() - 1);
			
			inumber = Real::mul(inumber, Two);
			
			if(SavedBit)
				SetBit(inumber, inumber, 0);
		}
	}
	
	Result = inumber;
	ConstrainNum(Result);
}



/*
 * Public methods
 */
 
void Integer::Fib(Real& Result, const Real& number)
{
	Real rp(Zero), rn, u = Real::entier(number);
	if (Real::sign(u) <= 0) Result = Zero;
	else {
		// iterative Fibonacci series
		Result = One;
		for (;;) {
			u = u - One;
			if (Real::sign(u) == 0) break;
			rn = Result + rp;
			rp = Result; Result = rn;
		}
	}
}

void Integer::GCD(Real& Result, const Real& op1, const Real& op2)
{
	Real nop1 = Real::entier(Real::abs(op1));
	Real nop2 = Real::entier(Real::abs(op2));
	while (Real::sign(nop2) != 0) {
		Mod(Result, nop1, nop2);
		nop1 = nop2; nop2 = Result;
	}
	Result = nop1;
}

void Integer::SetBit(Real& Result, const Real& number, short bitnum)
{
	LBit(Result, number, OrSet, bitnum);
}


void Integer::ClearBit(Real& Result, const Real& number, short bitnum)
{
	LBit(Result, number, AndNotSet, bitnum);
}


void Integer::ToggleBit(Real& Result, const Real& number, short bitnum)
{
	LBit(Result, number, XorSet, bitnum);
}


void Integer::And(Real& Result, const Real& op1, const Real& op2)
{
	LOp(Result, op1, AndSet, op2);
}

void Integer::Nand(Real& Result, const Real& op1, const Real& op2)
{
	LOp(Result, op1, NandSet, op2);
}

void Integer::Or(Real& Result, const Real& op1, const Real& op2)
{
	LOp(Result, op1, OrSet, op2);
}

void Integer::Nor(Real& Result, const Real& op1, const Real& op2)
{
	LOp(Result, op1, NorSet, op2);
}

void Integer::Xor(Real& Result, const Real& op1, const Real& op2)
{
	LOp(Result, op1, XorSet, op2);
}

void Integer::Count(Real& Result, const Real& op1)
{
	int32 bcnt = 0;
	Real number(op1);
	Logical lop;
	
	NumToLogical(number, lop);
	for (int32 i = 0 ; i <= LogicalSize ; i++) {
		for (int32 j = 0; j < 16; j++)
			if (lop[i] & (1 << j)) bcnt++;
	}
	Result = Real(bcnt);
}

void Integer::IntDiv(Real& Result, const Real& op1, const Real& op2)
{
	Real lop1(op1);
	Real lop2(op2);
	ConstrainNum(lop1);
	ConstrainNum(lop2);
	Result = Real::entier(Real::div(lop1, lop2));
}


void Integer::Mod(Real& Result, const Real& op1, const Real& op2)
{
	Real lop1(op1);
	Real lop2(op2);
	ConstrainNum(lop1);
	ConstrainNum(lop2);
	IntDiv(Result, lop1, lop2);
	Result = Real::sub(lop1, Real::mul(Result, lop2));
}


void Integer::OnesComp(Real& Result, const Real& number)
{
	LOp1(Result, NotSet, number);
}


void Integer::Shl(Real& Result, const Real& number, short numbits)
{
	LShift(Result, number, Real::mul, numbits);
	
	Result = Real::abs(Result);
	if (Bit(Result, MaxBits() - 1))
		Result = Real::negate(Result);
}


void Integer::Rol(Real& Result, const Real& number, short numbits)
{
	LRotate(Result, number, Left, numbits);
}


void Integer::Shr(Real& Result, const Real& number, short numbits)
{
	LShift(Result, number, Real::div, numbits);
	Result = Real::abs(Result);
}


void Integer::Ashr(Real& Result, const Real& number, short numbits)
{
	short ShiftCnt;
	bool SavedBit;
	Real inumber(number);
	
	ConstrainNum(inumber);
	
	if (numbits > MaxBits()) {
		Result = Real::Copy(One);
		return;
	}
	
	SavedBit = (Real::cmp(inumber, Zero) < 0);
	
	for (ShiftCnt = 1 ; ShiftCnt <= numbits ; ShiftCnt++) {
		inumber = Real::div(inumber, Two);
		
		if(SavedBit)
			SetBit(inumber, inumber, MaxBits() - 1);
	}
	
	Result = Real::entier(inumber);
}


void Integer::Ror(Real& Result, const Real& number, short numbits)
{
	LRotate(Result, number, Right, numbits);
}


static uint32 DigitIs(const char *S, long& InCnt, BaseType Base)
{
	long Digits;

	if(S[InCnt] > '9')
		Digits = S[InCnt] - 'A' + 10;
	else
		Digits = S[InCnt] - '0';

	InCnt++;
	if(Digits >= Base)
	{
		Real::err = errIllegalNumber;
		return 0;
	}
	
	return Digits;
}

void Integer::StrToInt(const char *S, BaseType Base, Real& A)
{
	long EndCnt, InCnt;
	Real Scale;

	A = Real::Copy(Zero);
	InCnt = 0;
	EndCnt = strlen(S);
	Scale = Real::Long(Base);
	
	while((InCnt < EndCnt) && (S[InCnt] == ' '))
		InCnt++;
		
	while((InCnt < EndCnt) && (Real::err != errIllegalNumber))
		A = Real::add(Real::mul(A, Scale), Real::Long(DigitIs(S, InCnt, Base)));
}

static void PutDigits(long Numb, BaseType Base, BString &S)
{
	long dig;
	short rc;
	char Res[81];
	
	for(rc = 3 ; rc >= 0 ; rc--)
	{
		dig = Numb % Base;
		if(dig > 9)
			Res[rc] = dig - 10 + 'A';
		else
			Res[rc] = dig + '0';
		Numb /= Base;
	}
	Res[4] = '\0';
	S.Insert(Res, 0);
}


void Integer::IntToStr(const Real& A, BaseType Base, char *S)
{
	short InCnt;
	Real Scale, Temp, Temp2, iA(A);
	BString s("");
	
	ConstrainNum(iA);
	if (Real::sign(iA) < 0) iA = Max() + iA + One;
	InCnt = 0;
	Scale = Real::power(Real::Long(Base), Real::Long(4));
	
	do {
		Temp2 = Real::entier(Real::div(iA, Scale));
		Temp = Real::sub(iA, Real::mul(Temp2, Scale));
		
		PutDigits((long) Real::Short(Temp), Base, s);
		
		iA = Temp2;
	} while(!IsZero(iA));
	
	while((s[1] != '\0') && (s[0] == '0')) s.Remove(0, 1);
	
	if(s[0] > '9') s.Prepend("0");
	strcpy(S, s.String());
}
