#ifndef EXCOMPLEX_H
#define EXCOMPLEX_H

#include <BeBuild.h>

#ifndef IMPEXP_LIBREALS
#ifdef BUILDING_LIBREALS
#define IMPEXP_LIBREALS _EXPORT
#else
#define IMPEXP_LIBREALS _IMPORT
#endif
#endif

#include "BinaryRider.h"
#include "Preference.h"
#include "Reals.h"

#if __POWERPC__
#pragma simple_class_byval off
#endif

/* Floating point formats */
enum fpFormat {Normal, Scientific, Engineering};

// Angular measure
enum msAngle {Degrees, Radians, Gradians};

const Real Zero = 0.0;

class IMPEXP_LIBREALS Complex
{
public:
	// constructors
	Complex(void) {};
	Complex(const Complex& x);   // copy constructor
	Complex(const Real& realPart, const Real& imagPart = Zero);
	~Complex(void) {};
	
	static void InitSettings();
	static void SetDecPoint(int32 dp);
	static void SetDigits(int32 digits);
	static void SetDigitSep(char ds);
	static void SetFracSep(char ds);
	static void SetBase(int32 base);
	static void SetFPFormat(fpFormat format);
	static void SetAngle(msAngle angle);
	static int32 GetDigits();
	static char GetDigitSep();
	static char GetFracSep();
	static int32 GetBase();
	static msAngle GetAngle();
	static fpFormat GetFPFormat();
	static int32 GetDecPoint();
	
	// overloaded operators
	Complex& operator = (const Complex& x);
	
	// return real and imaginary parts of number
	Real RealPart ();
	Real ImagPart ();

	static Complex Ex0, Ex1;
	
	// I/O routines
	static void Read(const Preference *r, const char *name, Complex&x);
	static void Write(Preference *w, const char *name, const Complex& x);
	
	/* Public methods */
	static Real fromRadians(const Real& angle);
	static Real toRadians(const Real& angle);
	static void ComplexToStr(const Complex& A, BString &Str);
	static void Conj(Complex& x);
	static void PolarMag(Real& A, const Complex& B);
	static void PolarAngle(Real& A, const Complex& B);
	static void ToRectangular(Complex& A, const Real& r, const Real& theta);
	static void LongRealToComplex(double x, Complex& xc);
	static double ComplexToLongReal(const Complex& xc);
	static void ChgSign(Complex& x);
	static void Add(Complex& A, const Complex& B, const Complex& C);
	static void Sub(Complex& A, const Complex& B, const Complex& C);
	static void Mult(Complex& A, const Complex& B, const Complex& C);
	static void Div(Complex& A, const Complex& B, const Complex& C);
	static void xtoi(Complex& Result, const Complex& x, long i);
	static void Root(Complex& Result, const Complex& x, long i);
	static void FromRadians(Complex& radianAngle);
	static void ToRadians(Complex& radianAngle);
	static void sqrt(Complex& Result, const Complex& x);
	static void ln(Complex& Result, const Complex& x);
	static void log(Complex& Result, const Complex& x);
	static void log2(Complex& Result, const Complex& x);
	static void factorial(Complex& Result, long n);
	static void exp(Complex& Result, const Complex& x);
	static void power(Complex& Result, const Complex& x, const Complex& y);
	static void root(Complex& Result, const Complex& x, const Complex& y);
	static void sin(Complex& Result, const Complex& x);
	static void cos(Complex& Result, const Complex& x);
	static void tan(Complex& Result, const Complex& x);
	static void cot(Complex& Result, const Complex& x);
	static void arcsin(Complex& Result, const Complex& x);
	static void arccos(Complex& Result, const Complex& x);
	static void arctan(Complex& Result, const Complex& z);
	static void arccot(Complex& Result, const Complex& x);
	static void sinh(Complex& Result, const Complex& x);
	static void cosh(Complex& Result, const Complex& x);
	static void tanh(Complex& Result, const Complex& x);
	static void coth(Complex& Result, const Complex& x);
	static void arcsinh(Complex& Result, const Complex& x);
	static void arccosh(Complex& Result, const Complex& x);
	static void arctanh(Complex& Result, const Complex& x);
	static void arccoth(Complex& Result, const Complex& x);
	
	static void InitComplex(void);
	
private:
	// real & imaginary complex components
	Real	re, im;

	static Real Zero, One, loge, log2c;
	static Real ToDegrees, ToGradians, FromDegrees, FromGradians;

	static bool initialized;
	
	/* Private methods */	
	static void CalcAlphaBeta(const Complex& z, Real& a, Real& b);
	static bool IsZero(const Real& x);
	static void NumToStr(const Real& A, short Decimal, short Digits, BString &Str, char Mode);
};


#endif
