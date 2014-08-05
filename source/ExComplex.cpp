#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <String.h>

#include "ExComplex.h"
#include "Reals.h"
#include "ExIntegers.h"
#include "Settings.h"

/* Private static variables */
Real Complex::Zero;
Real Complex::One;
Real Complex::loge, Complex::log2c;
Real Complex::ToDegrees;
Real Complex::ToGradians;
Real Complex::FromDegrees;
Real Complex::FromGradians;
bool Complex::initialized = false;

/* Public static variables */
Complex Complex::Ex0;
Complex Complex::Ex1;

#define ABS(x) (x < 0 ? -x : x)

static const char *DECPOINT 		= "Decimals:";
static const char *NUMDIGITS 	= "Digits:";
static const char *DIGSEP			= "Complex-digsep";
static const char *FRACSEP		= "Complex-fracsep";
static const char *ANGLE			= "Complex-angle";
static const char *FORMAT			= "Complex-format";
static const char *BASE				= "Complex-base";

void Complex::InitSettings()
{
	Settings::SetFloat(DECPOINT, 0);
	Settings::SetFloat(NUMDIGITS, 16);
	Settings::SetInt(DIGSEP, 0);
	Settings::SetInt(FRACSEP, 0);
	Settings::SetInt(ANGLE, 0);
	Settings::SetInt(FORMAT, 0);
	Settings::SetInt(BASE, 10);
}

void Complex::SetDecPoint (int32 dp)
{
	Settings::SetFloat(DECPOINT, dp);
}

void Complex::SetDigits (int32 digits)
{
	Settings::SetFloat(NUMDIGITS, digits);
	Real::SetDigits(digits);
}

void Complex::SetAngle (msAngle angle)
{
	Settings::SetInt(ANGLE, angle);
}

void Complex::SetFPFormat (fpFormat format)
{
	Settings::SetInt(FORMAT, format);
}

void Complex::SetBase (int32 base)
{
	Settings::SetInt(BASE, base);
}

void Complex::SetDigitSep (char ds)
{
	Settings::SetInt(DIGSEP, ds);
}

int32 Complex::GetBase()
{
	return Settings::GetInt(BASE);
}

msAngle Complex::GetAngle()
{
	return (msAngle)Settings::GetInt(ANGLE);
}

int32 Complex::GetDecPoint()
{
	return (int32)Settings::GetFloat(DECPOINT);
}

fpFormat Complex::GetFPFormat()
{
	return (fpFormat)Settings::GetInt(FORMAT);
}

int32 Complex::GetDigits()
{
	return (int32)Settings::GetFloat(NUMDIGITS);
}
char Complex::GetDigitSep()
{
	return Settings::GetInt(DIGSEP);
}

void Complex::SetFracSep (char fs)
{
	Settings::SetInt(FRACSEP, fs);
}

char Complex::GetFracSep()
{
	return Settings::GetInt(FRACSEP);
}

// constructor definitions
Complex::Complex(const Real& realPart, const Real& imagPart)
{
	// make copies of arguments for this number
	re = realPart; im = imagPart;
}


// overloaded operator for assignments
Complex::Complex(const Complex& x)
{
	// make new copies of real & imaginary parts
	re = x.re; im = x.im;
}


Complex& Complex::operator = (const Complex& x)
{
	// check if assigning to self
	if (this == &x) return *this;
	
	// make new copies of real & imaginary parts
	re = x.re; im = x.im;
	return *this;
}


// routines to return the real or imaginary parts
Real Complex::RealPart ()
{
	Real t = re;
	return t;
}

Real Complex::ImagPart ()
{
	Real t = im;
	return t;
}

void Complex::InitComplex()
{
	if(initialized)
		return;
		
	initialized = true;

	Real::Init();
	Integer::Init();

	Zero = Real::Long(0);
	One = Real::Long(1);
	Ex1.re = One;
	Ex1.im = Zero;
	Ex0.re = Zero;
	Ex0.im = Zero;
	
	ToDegrees = Real::div(Real::Long(180), Real::pi);
	FromDegrees = Real::div(Real::pi, Real::Long(180));
	ToGradians = Real::div(Real::Long(200), Real::pi);
	FromGradians = Real::div(Real::pi, Real::Long(200));
	
	loge = Real::div(One, Real::ln10);
	log2c = Real::ln(Real::Long(2));
}

bool Complex::IsZero(const Real& x)
{
	return (Real::sign(x) == 0);
}


Real Complex::fromRadians(const Real& angle)
{
	if (GetAngle() == Degrees)
		return Real::mul(ToDegrees, angle);
	else if (GetAngle() == Gradians)
		return Real::mul(ToGradians, angle);
	else
		return Real(angle);
}


Real Complex::toRadians(const Real& angle)
{
	if (GetAngle() == Degrees)
		return Real::mul(FromDegrees, angle);
	else if (GetAngle() == Gradians)
		return Real::mul(FromGradians, angle);
	else
		return Real(angle);
}


void Complex::NumToStr(const Real& A, short Decimal, short Digits, BString &Str, char Mode)
{
	char s[1024];
	long ds = 3;
	if (GetBase() == 10)
		Real::ToString(A, s, Digits, Decimal, Mode);
	else {
		if (GetBase() != 8) ds = 4;
		Integer::IntToStr(A, GetBase(), s);
	}
	Str.SetTo(s);
	
	// format the number with separators
	char cs[2];
	short dp, exp = 0;
	long i;
	
	cs[1] = 0x0;
	dp = Str.FindFirst('.');
	if (dp == B_ERROR)
		dp = Str.CountChars();
	else if (GetDigitSep() == '.')
		Str[dp] = ',';
	
	if ((GetDigitSep() != 0x0) && (Mode == Normal)) {
		i = dp - ds;
		cs[0] = GetDigitSep();
		while (i > 0) {
			if (GetBase() == 10) {
				if (isdigit(Str[i-1]) && isdigit(Str[i])) Str.Insert(cs, i);
			} else {
				if (isxdigit(Str[i-1]) && isxdigit(Str[i])) Str.Insert(cs, i);
			}
			i -= ds;
			dp++;
		}
	}
	
	if (GetFracSep() != 0x0) {
 		exp = Str.FindFirst('E');
 		if (exp == B_ERROR) exp = Str.CountChars();
		i = dp + ds + 1;
		cs[0] = GetFracSep();
		while (i < exp) {
			Str.Insert(cs, i);
			i += (ds + 1);
			exp++;
		}
	}
}

void Complex::ComplexToStr(const Complex& A, BString &Str)
{
	bool iZero, rZero;
	short ExpWidth;
	bool EngFormat;
	Real Aim;
	
	iZero = IsZero(A.im);
	rZero = IsZero(A.re);
	EngFormat = (GetFPFormat() == Engineering);
	ExpWidth = GetFPFormat() > Normal ? 1 : 0;
	if (iZero || !rZero)
		NumToStr(A.re, GetDecPoint(), GetDigits(), Str, GetFPFormat());
	else
		Str.SetTo("");
		
	if (!iZero) {
		Aim = Real::abs(A.im);
		if (Real::sign(A.im) >= 0) {
			if (!rZero)
				Str.Append("+");
		} else
			Str.Append("-");
			
		if (Real::cmp(Aim, One) != 0) {
			BString Result;
			NumToStr(Aim, GetDecPoint(), GetDigits(), Result, GetFPFormat());
			Str.Append(Result);
		}
		Str.Append("i");
	}
}

void Complex::Read(const Preference *r, const char *name, Complex&x)
{
	BString imag(name); imag.Append("-im");
	Real::Read(r, name, x.re);
	Real::Read(r, imag.String(), x.im);
}

void Complex::Write(Preference *w, const char *name, const Complex& x)
{
	BString imag(name); imag.Append("-im");
	Real::Write(w, name, x.re);
	Real::Write(w, imag.String(), x.im);
}


void Complex::Conj(Complex& x)
{
	x = Complex(x.re, Real::negate(x.im));
}


void Complex::PolarMag(Real &A, const Complex& B)
{
	Real r, i, h, ONE(1.0);
	r=Real::abs(B.re); i = Real::abs(B.im);
	if (Real::cmp(i, r) > 0) {
		h = i; i = r; r = h;
	}
	if (IsZero(i)) {A = r; return;}
	h = i/r;
	A = r*Real::sqrt(ONE+h*h);
}


void Complex::PolarAngle(Real &A, const Complex& B)
{
	Real t;
	t = Real::arctan2(B.re, B.im);
	A = t;
}


void Complex::ToRectangular(Complex& A, const Real& r, const Real& theta)
{	
	Complex t;
	
	// compute more accurately
	t = Complex(r * Real::cos(theta), r * Real::sin(theta));

	// truncate very small numbers
	int32 rexp = A.re.exponent();
	int32 iexp = A.im.exponent();
	if ((rexp < 0) & (-rexp > Real::sigDigs)) t = Complex(0.0, t.im);
	if ((iexp < 0) & (-iexp > Real::sigDigs)) t = Complex(t.re, 0.0);
	A = t;	
}


void Complex::LongRealToComplex(double x, Complex& xc)
{
	xc = Complex(x, Zero);
}


double Complex::ComplexToLongReal(const Complex& xc)
{
	return Real::Short(xc.re);
}


void Complex::ChgSign(Complex& x)
{
	x = Complex(-x.re, -x.im);
}


void Complex::Add(Complex& A, const Complex& B, const Complex& C)
{
	A = Complex(B.re + C.re, B.im + C.im);
}


void Complex::Sub(Complex& A, const Complex& B, const Complex& C)
{
	A = Complex(B.re - C.re, B.im - C.im);
}


void Complex::Mult(Complex& A, const Complex& B, const Complex& C)
{
	A = Complex(B.re*C.re - B.im*C.im, B.re*C.im + B.im*C.re);
}


void Complex::Div(Complex& A, const Complex& B, const Complex& C)
{
	Real h,d;
	if (Real::cmp(Real::abs(C.re), Real::abs(C.im)) > 0) {
		h = C.im/C.re; d = C.re + h*C.im;
		A = Complex((B.re+h*B.im)/d, (B.im-h*B.re)/d);
	} else {
		h = C.re/C.im; d = C.im + h*C.re;
		A = Complex((B.im+h*B.re)/d, (-B.re+h*B.im)/d);
	}
}


void Complex::xtoi(Complex& Result, const Complex& x, long i)
{
	Complex ix = x, Y;
	bool negative;
	
	Y = Ex1;
	negative = (i < 0);
	i = abs(i);
	while(1)
	{
		if(i & 1)
			Mult(Y, Y, ix);
		i /= 2;
		if(i == 0)
			break;
		Mult(ix, ix, ix);
	}
	if(negative)
		Div(Result, Ex1, Y);
	else
		Result = Complex(Y.re, Y.im);
}


void Complex::Root(Complex& Result, const Complex& x, long i)
{
	Real r, theta;
	Complex ix = x;
	bool izero;
	
	izero = IsZero(ix.im);
	if (izero && (Real::sign(ix.re) >= 0))
	{
		Result = Complex(Real::root(ix.re, i), Zero);
	}
	else if (izero && (i &1))
	{
		Result = Complex(-Real::root(Real::abs(ix.re), i), Zero);
	}
	else
	{		
		PolarMag(r, ix);
		PolarAngle(theta, ix);

		ix.re = Real::Long(i);
		r = Real::root(r, i);
		theta = Real::div(theta, ix.re);
		ToRectangular(Result, r, theta);
	}
}


void Complex::FromRadians(Complex& radianAngle)
{
	radianAngle = Complex(fromRadians(radianAngle.re), fromRadians(radianAngle.im));
}


void Complex::ToRadians(Complex& radianAngle)
{
	radianAngle = Complex(toRadians(radianAngle.re), toRadians(radianAngle.im));
}


void Complex::sqrt(Complex& Result, const Complex& x)
{
	Real u, v, HALF(0.5), ZERO(0.0);
	PolarMag(v, x);
	u = Real::sqrt((v + Real::abs(x.re)) * HALF);
	if (!IsZero(x.im)) {v = (HALF*x.im)/u;} else {v = ZERO;}
	if (Real::cmp(x.re, ZERO) >= 0) {Result = Complex(u, v); return;}
	else if (Real::cmp(x.im, ZERO) >= 0) {Result = Complex(v, u); return;}
	else {Result = Complex(-v, -u);}
}


void Complex::ln(Complex& Result, const Complex& x)
{
	bool pos;
	
	if(IsZero(x.im))
	{
		pos = (Real::sign(x.re) > 0);
		Result.re = Real::ln(Real::abs(x.re));
		Result.im = pos ? Zero : Real::pi;
	}
	else if(IsZero(x.re))
	{
		pos = (Real::sign(x.im) > 0);
		Result.re = Real::ln(Real::abs(x.im));
		Result.im = Real::mul(Real::Long(0.5), Real::pi);
		if(!pos)
			Result.im = Real::negate(Result.im);
	}
	else
	{
		PolarMag(Result.re, x);
		PolarAngle(Result.im, x);
		Result.re = Real::ln(Result.re);
	}
}


void Complex::log(Complex& Result, const Complex& x)
{
	Complex ix;
	ln(ix, x);
	Result = Complex(loge * ix.re, loge * ix.im);
}

void Complex::log2(Complex& Result, const Complex& x)
{
	Complex ix;
	ln(ix, x);
	Result = Complex(ix.re/log2c, ix.im/log2c);
}

void Complex::factorial(Complex& Result, long n)
{
	Result = Complex(Real::factorial(n), Zero);
}


void Complex::exp(Complex& Result, const Complex& x)
{
	Real c, s, e;
	Real::sincos(x.im, s, c); e = Real::exp(x.re);
	Result = Complex(e*c, e*s);
}


void Complex::power(Complex& Result, const Complex& x, const Complex& y)
{
	double p;
	Real r;
	Complex ix(x);
	
	if(IsZero(y.im))
	{
		if(IsZero(x.im)) {
			Result = Complex(Real::power(x.re, y.re), Zero);
			return;
		} else {
			p = ComplexToLongReal(y);
			r = Real::fraction(y.re);
			if((ABS(p) < LONG_MAX) && IsZero(r)) {
				xtoi(Result, x, (long) p);
				return;
			}
		}
	}
	ln(Result, ix);
	Mult(ix, Result, y);
	exp(Result, ix);
}


void Complex::root(Complex& Result, const Complex& x, const Complex& y)
{
	double r;
	Complex t, ix = x;
	
	if(IsZero(y.im) && (Real::sign(y.re) > 0))
	{
		if(IsZero(ix.im) && (Real::sign(ix.re) > 0))
		{
			Result = Complex(Real::root(ix.re, (long) Real::Short(y.re)), Zero);
		}
		else
		{
			r = ComplexToLongReal(y);
			t.re = Real::fraction(y.re);
			if((ABS(r) < LONG_MAX) && IsZero(t.re))
				Root(Result, ix, (long) r);
			else
			{
				ln(Result, ix);
				Div(ix, Result, y);
				exp(Result, ix);
			}
		}
	}
	else
	{
		ln(Result, ix);
		Div(ix, Result, y);
		exp(Result, ix);
	}
}


void Complex::sin(Complex& Result, const Complex& x)
{
	Complex ix = x;
	Real s, c, sh, ch;
	ToRadians(ix); Real::sincos(ix.re, s, c); Real::sinhcosh(ix.im, sh, ch);
	Result = Complex(s * ch, c * sh);
}


void Complex::cos(Complex& Result, const Complex& x)
{
	Complex ix = x;
	Real s, c, sh, ch;
	ToRadians(ix); Real::sincos(ix.re, s, c); Real::sinhcosh(ix.im, sh, ch);
	Result = Complex(c * ch, -s * sh);
}


void Complex::tan(Complex& Result, const Complex& x)
{
	Real TWO(2), d, s, c, sh, ch;
	Complex ix = x;
	ToRadians(ix); 
	Real::sincos(TWO*ix.re, s, c); Real::sinhcosh(TWO*ix.im, sh, ch);
	d = c + ch;
	Result = Complex(s/d, sh/d);
}

void Complex::cot(Complex& Result, const Complex& x)
{
	Real TWO(2), d, s, c, sh, ch;
	Complex ix = x;	
	ToRadians(ix); 
	Real::sincos(TWO*ix.re, s, c); Real::sinhcosh(TWO*ix.im, sh, ch);
	d = ch - c;
	Result = Complex(s/d, -sh/d);
}

void Complex::CalcAlphaBeta(const Complex& z, Real& a, Real& b)
{
	Real x, x2, y, r, t, HALF(0.5);
	
	x = z.re + One; x = x * x; y = z.im * z.im;
	x2 = z.re - One; x2 = x2 * x2;
	r = Real::sqrt(x + y); t = Real::sqrt(x2 + y);
	a = HALF * (r + t); b = HALF * (r - t);
}


void Complex::arcsin(Complex& Result, const Complex& x)
{
	Real a, b;
	
	CalcAlphaBeta(x, a, b);
	Result = Complex(Real::arcsin(b), Real::ln(a + Real::sqrt(a * a - One)));
	FromRadians(Result);
}


void Complex::arccos(Complex& Result, const Complex& x)
{
	Real a, b;	
	
	CalcAlphaBeta(x, a, b);
	Result = Complex(Real::arccos(b), -Real::ln(a + Real::sqrt(a * a - One)));
	FromRadians(Result);
}


void Complex::arctan(Complex& Result, const Complex& z)
{
	Real x, x2, y2, y, yp, t;
	Real ZERO(0.0), TWO(2.0), HALF(0.5), QUARTER(0.25);
		
	x = TWO * z.re; y = z.im + One; y = y * y;
	yp = z.im - One; yp = yp * yp;
	x2 = z.re * z.re; y2 = z.im * z.im;
	t = One-x2-y2;
	Result = Complex(HALF * Real::arctan2(x, t), QUARTER * Real::ln((x2+y) / (x2+yp)));
	FromRadians(Result);
}

void Complex::arccot(Complex& Result, const Complex& z)
{
	Div(Result, Ex1, z);
	arctan(Result, Result);
}

void Complex::sinh(Complex& Result, const Complex& x)
{
	Real sinh1, cosh1, sinh2, cosh2;
	Real::sinhcosh(x.re, sinh1, cosh1);
	Real::sinhcosh(x.im, sinh2, cosh2); 
	Result = Complex(sinh1 * cosh2, cosh1 * sinh2);
}


void Complex::cosh(Complex& Result, const Complex& x)
{
	Real sinh1, cosh1, sinh2, cosh2;
	Real::sinhcosh(x.re, sinh1, cosh1);
	Real::sinhcosh(x.im, sinh2, cosh2); 
	Result = Complex(cosh1 * cosh2, sinh1 * sinh2);
}


void Complex::tanh(Complex& Result, const Complex& x)
{
	Real sin, cos, sinh, cosh, two(2.0), den;
	Real::sinhcosh(two*x.re, sinh, cosh);
	Real::sincos(two*x.im, sin, cos);
	den = cosh + cos;
	Result = Complex(sinh/den, sin/den);
}

void Complex::coth(Complex& Result, const Complex& x)
{
	Real sin, cos, sinh, cosh, two(2.0), den;
	Real::sinhcosh(two*x.re, sinh, cosh);
	Real::sincos(two*x.im, sin, cos);
	den = cosh - cos;
	Result = Complex(sinh/den, -sin/den);
}

void Complex::arcsinh(Complex& Result, const Complex& x)
{
	Complex Temp;
	
	Mult(Temp, x, x);
	Add(Temp, Temp, Ex1);
	sqrt(Temp, Temp);
	Add(Temp, x, Temp);
	ln(Result, Temp);
}


void Complex::arccosh(Complex& Result, const Complex& x)
{
	Complex Temp;
	
	Mult(Temp, x, x);
	Sub(Temp, Temp, Ex1);
	sqrt(Temp, Temp);
	Add(Temp, x, Temp);
	ln(Result, Temp);
}


void Complex::arctanh(Complex& Result, const Complex& x)
{
	Complex Temp, Temp2;
	
	Add(Temp, Ex1, x);
	Sub(Temp2, Ex1, x);
	Div(Temp, Temp, Temp2);
	if (Real::err == errNone) {
		ln(Result, Temp);
		Temp = Complex(Real::Long(0.5), Zero);
		Mult(Result, Result, Temp);
	}
}

void Complex::arccoth(Complex& Result, const Complex& x)
{
	Div(Result, Ex1, x);
	arctanh(Result, Result);
}


