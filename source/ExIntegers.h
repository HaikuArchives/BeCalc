#ifndef EXINTEGERS_H
#define EXINTEGERS_H

#include <BeBuild.h>

#ifndef IMPEXP_LIBREALS
#ifdef BUILDING_LIBREALS
#define IMPEXP_LIBREALS _EXPORT
#else
#define IMPEXP_LIBREALS _IMPORT
#endif
#endif

#include "Reals.h"

#if __POWERPC__
#pragma simple_class_byval off
#endif

typedef char BaseType;

class IMPEXP_LIBREALS Integer
{
public:
	static void SetBit(Real& Result, const Real& number, short bitnum);
	static void ClearBit(Real& Result, const Real& number, short bitnum);
	static void ToggleBit(Real& Result, const Real& number, short bitnum);
	static void And(Real& Result, const Real& op1, const Real& op2);
	static void Nand(Real& Result, const Real& op1, const Real& op2);
	static void Or(Real& Result, const Real& op1, const Real& op2);
	static void Nor(Real& Result, const Real& op1, const Real& op2);
	static void Xor(Real& Result, const Real& op1, const Real& op2);
	static void Count(Real& Result, const Real& op1);
	static void IntDiv(Real& Result, const Real& op1, const Real& op2);
	static void Mod(Real& Result, const Real& op1, const Real& op2);
	static void OnesComp(Real& Result, const Real& number);
	static void Shl(Real& Result, const Real& number, short numbits);
	static void Rol(Real& Result, const Real& number, short numbits);
	static void Shr(Real& Result, const Real& number, short numbits);
	static void Ashr(Real& Result, const Real& number, short numbits);
	static void Ror(Real& Result, const Real& number, short numbits);
	static void Fib(Real& Result, const Real& number);
	static void GCD(Real& Result, const Real& op1, const Real& op2);
	static void StrToInt(const char *S, BaseType Base, Real& A);
	static void IntToStr(const Real& A, BaseType Base, char *S);
	
	static void Init(void);
	
private:
	static const long MaxBase2Bits = 671;
	static const long LogicalSize = MaxBase2Bits / 16;
	static const bool Left = false;
	static const bool Right = true;
	
	typedef uint32 Logical[LogicalSize+1];
	typedef uint32 (* LogicalProc) (uint32 a, uint32 b);
	typedef Real (* ExNumbProc) (const Real& a, const Real& b);

	static uint32 LogZero[LogicalSize+1];
	static Real One, Two;
	static Real Zero;
	static short Cnt;
	
	static bool initialized;
	
	static uint32 AndSet (uint32 op1, uint32 op2);
	static uint32 AndNotSet (uint32 op1, uint32 op2);
	static uint32 OrSet (uint32 op1, uint32 op2);
	static uint32 XorSet (uint32 op1, uint32 op2);
	static uint32 NorSet (uint32 op1, uint32 op2);
	static uint32 NandSet (uint32 op1, uint32 op2);
	static uint32 NotSet (uint32 op1, uint32 op2 = 0);		
	static bool IsZero (const Real& x);
	
	/* Misc. local functions */
	static Real Max();
	static int32 MaxBits();
	static void ConstrainNum(Real& Number);
	static void NumToLogical(const Real& Numb, Logical& logical);
	static void LogicalToNum(const Logical& logical, Real& Numb);
	static void LOp(Real& Result, const Real& op1, LogicalProc Oper, const Real& op2);
	static void LOp1(Real& Result, LogicalProc Oper, const Real& op);
	static void LBit(Real& Result, const Real& number, LogicalProc Oper, short bitnum);
	static bool Bit(Real& number, short bitnum);
	static void LShift(Real& Result, const Real& number, ExNumbProc ExOper, short bits);
	static void LRotate(Real& Result, const Real& number, bool Shiftright, short bits);
};

#endif
