#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <String.h>

#include "Reals.h"
#include "Register.h"

/* Internal inline functions */

static inline long Min(long X, long Y)	{return (X < Y ? X : Y);}
static inline long Max(long X, long Y)	{return (X > Y ? X : Y);}
static inline long ABS(long X) {return (X < 0 ? -X : X);}
static inline float ABS(float X) {return (X < 0.0 ? -X : X);}
static inline double ABS(double X) {return (X < 0.0 ? -X : X);}
static inline bool ODD(long X) {return (X & 0x1);}

static inline long ENTIER (double x) {
	long v = (long)x;           
  	if ((v > x) && (v-1 <= x)) v--;
  	return v;
} 
 
static inline long MOD(long x, long y) {
  	long rem = x % y;
  	if (rem && ((rem > 0) != (y > 0)))
    	return (rem+y);
    else
	 	return (rem); 
} 

/* Static variables */

/* Private */
bool Real::initialized = false;
short Real::numBits;
Real Real::seed;
float Real::xONE[8];

double Real::ZERO = 0.0;
double Real::ONE = 1.0;
double Real::HALF = 0.5;
double Real::invLn2 = 1.4426950408889633;
double Real::Ln2 = 0.693147180559945309;
double Real::log2 = 0.301029995663981195;

double Real::mpbbx = 4096.0;
double Real::radix = mpbbx * mpbbx;
double Real::mpbx2 = radix * radix;
double Real::mprbx = ONE / mpbbx;
double Real::invRadix = mprbx * mprbx;
double Real::mprx2 = invRadix * invRadix;
double Real::mprxx = 16 * mprx2;

/* Public */
errCodes Real::err;
short Real::sigDigs;
short Real::debug;
double Real::digsPerWord = 7.224719896;  	// number of digits per word
short Real::curMant = maxMant + 1;
Real Real::eps(curMant + 4);
Real Real::ln2(curMant + 4);
Real Real::pi(curMant + 4);
Real Real::ln10(curMant + 4);

/*
 * Constructors/destructors
 */

Real::Real(void) : len(0)
{
}

Real::Real(float *x, int size)
{
	for (int32 i = 0; i < size; i++) {
		val[i] = x[i];
	}
	len = size;
}

Real::Real(const Real& x)
{
	len = x.len;
	copy(x.val, val);
}

Real::Real(const char *str)
{
	toReal(str, val);
	len = curMant+3;
}

Real::Real(double x)
{	
	NumbExpToReal(x, 0, val);
	len = curMant+3;
}

Real::~Real(void)
{
	len = 0;
}

/*
 * Overloaded operators
 */

Real& Real::operator = (const Real& x)
{
	// check if assigning to self
	if (this == &x) return *this;
	
	// create a new number
	len = x.len;
		
	// copy and return the value
	copy(x.val, val);
	return *this;
}

/*
 * Internal (non-user) routines
 */
 
long Real::Sign(float x, float y)
{
	if (y<ZERO) return -ENTIER(ABS(x));
	else return ENTIER(ABS(x));
}

void Real::Zero(float *x)
{
	x[0] = x[1] = ZERO;
}

long Real::Int(double x)
{
	return (x < ZERO ? -ENTIER(-x) : ENTIER(x));
}

double Real::ipower(double x, short base)
{
	double y;
	bool neg;
	
	y = ONE;
	if(base < 0)
	{
		neg = true;
		base = -base;
	}
	else
		neg = false;
		
	while(1)
	{
		if(ODD(base))
			y *= x;
		base /= 2;
		if(base == 0)
			break;
		x *= x;
	}

	return (neg ? ONE / y : y);
}

void Real::Reduce(double &a, long &exp)
{
	const short maxIterations = 100;
	long k;
	
	if(a >= radix)
	{
		for(k = 1 ; k <= maxIterations ; k++)
		{
			a *= invRadix;
			if(a < radix)
			{
				exp += k;
				return;
			}
		}
	}
	else if(a < ONE)
	{
		for(k = 1 ; k <= maxIterations ; k++)
		{
			a *= radix;
			if(a >= ONE)
			{
				exp -= k;
				return;
			}
		}
	}
}

void Real::copy(const float *a, float *b)
{
	long ia, na, i;
	
	ia = Sign(ONE, a[0]);
	na = Min(ENTIER(ABS(a[0])), curMant);

	if(na == 0)
	{
		Zero(b);
		return;
	}

	b[0] = Sign(na, ia);
	
	for(i = 1 ; i <= na + 2 ; i++)
		b[i] = a[i];
}

void Real::OutRealDesc(const float *n)
{
	Real r(8);
	
	copy(n, r.val);
	OutReal(r);
}

void Real::WriteReal(const float *q)
{
	long i;
	
	for(i = 0 ; i <= ENTIER(ABS(q[0])) + 1 ; i++)
		printf("x[%ld]=%ld\n", i, Int(q[i]));
	printf("\n");
}

void WriteLongReal(const double *q, long len)
{
	long i;
	
	for(i = 0 ; i <= len-1; i++)
		printf("x[%ld]=%f\n", i, q[i]);
	printf("\n");
}

void Real::Round(float *a)
{
	float a2;
	bool allZeros;
	long ia, na, n4, i, k = 0;
	
	if(err != 0)
	{
		Zero(a);
		return;
	}
	
	a2 = a[1];
	a[1] = ZERO;
	ia = Sign(ONE, a[0]);
	na = Min(ENTIER(ABS(a[0])), curMant);
	n4 = na + 4;
	
	if(a[2] == ZERO)
	{
		allZeros = true;
		i = 4;
		while(allZeros && (i <= n4))
		{
			if(a[i - 1] != ZERO)
			{
				allZeros = false;
				k = i - 3;
			}
			i++;
		}
		
		if(allZeros)
		{
			Zero(a);
			return;
		}
		
		for(i = 2 ; i <= (n4 - k - 1) ; i++)
			a[i] = a[i + k];
			
		a2 -= k;
		na -= Max(k - 2, 0);
	}
	
	if((na == curMant) && (MPIRD >= 1))
	{
		if(((MPIRD == 1) && (a[na + 2] >= HALF * radix)) ||
		   ((MPIRD == 2) && (a[na + 2] >= ONE)))
		   a[na + 1] += ONE;

		i = na + 1;
		
		while(1)
		{
			if(i < 2)
			{
				a[2] = a[1];
				na = 1;
				a2 += ONE;
				break;
			}
			
			if(a[i] < radix)
				break;
				
			a[i] = (float) ((double) a[i] - radix);
			a[i - 1] += ONE;
			i--;
		}
	}
	
	if(a[na + 1] == ZERO)
	{
		i = na + 2;
		while(i >= 3)
		{
			if(a[i - 1] != ZERO)
			{
				na = i - 2;
				i = 1;
			}
			i--;
		}
		if(i != 0)
		{
			Zero(a);
			return;
		}
	}

	if(a2 < -maxExp)
	{
		puts("*** Round: Exponent underflow!");
		err = errExpUnderflow;
	}
	else if(a2 > maxExp)
	{
		puts("*** Round: Exponent overflow!");
		err = errExpOverflow;
	}
	
	if(a[2] == ZERO)
		Zero(a);
	else
	{
		a[0] = Sign(na, ia);
		a[1] = a2;
		a[na + 2] = ZERO;
		a[na + 3] = ZERO;
	}
}

void Real::Normalize(double *d, float *a)
{
	float a2;
	double t1, t2, t3;
	long ia, na, n4, i;
	
	if(err != 0)
	{
		Zero(a);
		return;
	}
	
	ia = Sign(ONE, (float) d[0]);
	na = Min(ENTIER(ABS(d[0])), curMant);

	if(na == 0)
	{
		Zero(a);
		return;
	}
	
	n4 = na + 4;
	a2 = (float) d[1];
	d[1] = ZERO;
	
	while(1)
	{
		t1 = ZERO;
		
		for(i = n4 - 1 ; i >= 2 ; i--)
		{
			t3 = t1 + d[i];
			t2 = invRadix * t3;
			t1 = (double) Int(t2);
			
			if((t2 < ZERO) && (t1 != t2))
				t1 -= ONE;
			d[i] = t3 - t1 * radix;
		}
		d[1] += t1;

		if(d[1] < ZERO)
		{
			ia = -ia;
			d[2] += radix * d[1];
			d[1] = ZERO;
			
			for(i = 1 ; i <= n4 - 1 ; i++)
				d[i] = -d[i];
		}
		else if(d[1] > ZERO)
		{
			for(i = n4 - 1 ; i >= 2 ; i--)
				a[i] = (float) d[i - 1];
			na = Min(na + 1, curMant);
			a2 += ONE;
			break;
		}
		else
		{
			for(i = 2 ; i <= n4 - 1 ; i++)
				a[i] = (float) d[i];
			break;
		}
	}
	
	a[0] = Sign(na, ia);
	a[1] = a2;
	Round(a);
}

void Real::RealToNumbExp(const float *a, double &b, long &n)
{
	double aa;
	long na;
	
	if(err != 0)
	{
		b = ZERO;
		n = 0;
		return;
	}
	
	if(a[0] == ZERO)
	{
		b = ZERO;
		n = 0;
		return;
	}
	
	na = ENTIER(ABS(a[0]));
	aa = a[2];
	
	if(na >= 2)
		aa += invRadix * a[3];
	if(na >= 3)
		aa += mprx2 * a[4];
	if(na >= 4)
		aa += invRadix * mprx2 * a[5];
		
	n = NBT * Int(a[1]);
	
	b = a[0] < 0 ? -aa : aa;
}

void Real::NumbExpToReal(double a, long n, float *b)
{
	double aa;
	long n1, n2, i;
	
	if(a == ZERO)
	{
		Zero(b);
		return;
	}
	
	n1 = Int(n / NBT);
	n2 = n - NBT * n1;
	aa = ABS(a) * ipower(2.0, n2);
	
	Reduce(aa, n1);
	
	b[1] = n1;
	b[2] = Int(aa); aa = radix * (aa - b[2]);
	b[3] = Int(aa);	aa = radix * (aa - b[3]);
	b[4] = Int(aa);	aa = radix * (aa - b[4]);
	b[5] = Int(aa);
	b[6] = ZERO;
	b[7] = ZERO;
	
	for(i = 5 ; i >= 2 ; i--)
	{
		if(b[i] != ZERO)
		{
			b[0] = Sign(i - 1, a);
			return;
		}
	}
	
	b[0] = ZERO;
}

void Real::Add(float *c, const float *arg_a, const float *arg_b)
{
	FixedReal a, b;
	long i, ia, ib, na, nb, nsh;
	long ixa, ixb, ixd, ish, m1, m2, m3, m4, m5, nd;
	double db;
	FixedLReal d;
	
	if(err != 0)
	{
		Zero(c);
		return;
	}
	
	copy(arg_a, a);
	copy(arg_b, b);
	
	if(debug >= 9)
	{
		puts("Add 1");
		WriteReal(a);
		puts("Add 2");
		WriteReal(b); 
	}
	
	ia = Sign(ONE, a[0]);
	ib = Sign(ONE, b[0]);
	na = Min(ENTIER(ABS(a[0])), curMant);
	nb = Min(ENTIER(ABS(b[0])), curMant);
	
	if(na == 0)
	{
		c[0] = Sign(nb, ib);
		for(i = 1 ; i <= nb + 1 ; i++)
			c[i] = b[i];
	}
	else if(nb == 0)
	{
		c[0] = Sign(na, ia);
		for(i = 1 ; i <= na + 1 ; i++)
			c[i] = a[i];
	}
	else
	{
		db = ia == ib ? ONE : -ONE;
		ixa = Int(a[1]);
		ixb = Int(b[1]);
		ish = ixa - ixb;
		
		if (ish >= 0)
		{
			m1 = Min(na, ish);
			m2 = Min(na, nb + ish);
			m3 = na;
			m4 = Min(Max(na, ish), curMant + 1);
			m5 = Min(Max(na, nb + ish), curMant + 1);
			d[0] = ZERO;
			d[1] = ZERO;
			for(i = 1 ; i <= m1 ; i++)
				d[i + 1] = a[i + 1];
			for(i = m1 + 1 ; i <= m2 ; i++)
				d[i + 1] = a[i + 1] + db * b[i + 1 - ish];
			for(i = m2 + 1 ; i <= m3 ; i++)
				d[i + 1] = a[i + 1];
			for(i = m3 + 1 ; i <= m4 ; i++)
				d[i + 1] = ZERO;
			for(i = m4 + 1 ; i <= m5 ; i++)
				d[i + 1] = db * b[i + 1 - ish];
			nd = m5;
			ixd = ixa;
			d[nd + 2] = ZERO;
			d[nd + 3] = ZERO;
		}
		else
		{
			nsh = -ish;
			m1 = Min(nb, nsh);
			m2 = Min(nb, na + nsh);
			m3 = nb;
			m4 = Min(Max(nb, nsh), curMant + 1);
			m5 = Min(Max(nb, na + nsh), curMant + 1);
			d[0] = ZERO;
			d[1] = ZERO;
			for(i = 1 ; i <= m1 ; i++)
				d[i + 1] = db * b[i + 1];
			for(i = m1 + 1 ; i <= m2 ; i++)
				d[i + 1] = a[i + 1 - nsh] + db * b[i + 1];
			for(i = m2 + 1 ; i <= m3 ; i++)
				d[i + 1] = db * b[i + 1];
			for(i = m3 + 1 ; i <= m4 ; i++)
				d[i + 1] = ZERO;
			for(i = m4 + 1 ; i <= m5 ; i++)
				d[i + 1] = a[i + 1 - nsh];
			nd = m5;
			ixd = ixb;
			d[nd + 2] = ZERO;
			d[nd + 3] = ZERO;
		}
		d[0] = Sign(nd , ia);
		d[1] = ixd;
		Normalize(d, c);
	}
	if(debug >= 9)
	{
		puts("Add 3");
		WriteReal(c);
	}
}

void Real::Sub(float *c, const float *arg_a, const float *arg_b)
{
	FixedReal a, b;
	
	if(err != 0)
	{
		Zero(c);
		return;
	}
	
	copy(arg_a, a);
	copy(arg_b, b);
	
	b[0] = -b[0];
	Add(c, a, b);
}

void Real::Mul(float *c, const float *arg_a, const float *arg_b)
{
	FixedReal a, b;
	long ia, ib, na, nb, nc, i, j, i1, i2, n2, j3;
	double d2, t1, t2;
	FixedLReal d;
	
	if(err != 0)
	{
		Zero(c);
		return;
	}
	
	copy(arg_a, a);
	copy(arg_b, b);
	
	if(debug >= 8)
	{
		puts("Mul 1");
		WriteReal(a);
		puts("Mul 2");
		WriteReal(b);
	}

	ia = Sign(ONE, a[0]);
	ib = Sign(ONE, b[0]);
	na = Min(ENTIER(ABS(a[0])), curMant);
	nb = Min(ENTIER(ABS(b[0])), curMant);
	
	if((na == 0) || (nb == 0))
	{
		Zero(c);
		return;
	}
	
	if((na == 1) && (a[2] == ONE))
	{
		c[0] = Sign(nb, ia * ib);
		c[1] = a[1] + b[1];
		for(i = 2 ; i <= nb + 1 ; i++)
			c[i] = b[i];
		return;
	}
	else if((nb == 1) && (b[2] == ONE))
	{
		c[0] = Sign(na, ia * ib);
		c[1] = a[1] + b[1];
		for(i = 2 ; i <= na + 1 ; i++)
			c[i] = a[i];
		return;
	}
	
	nc = Min(na + nb, curMant);
	d2 = a[1] + b[1];
	for(i = 0 ; i <= nc + 3 ; i++)
		d[i] = ZERO;
		
	for(j = 3 ; j <= na + 2 ; j++)
	{
		t1 = a[j - 1];
		j3 = j - 3;
		n2 = Min(nb + 2, curMant + 4 - j3);
		for(i = 2 ; i <= n2 - 1 ; i++)
			d[i + j3] += t1 * b[i];
			
		if (MOD(j - 2,  NPR) == 0)
		{
			i1 = Max(3, j - NPR);
			i2 = n2 + j3;
			
			for(i = i1 ; i <= i2 ; i++)
			{
				t1 = d[i - 1];
				t2 = Int(invRadix * t1);
				d[i - 1] = t1 - radix * t2;
				d[i - 2] += t2;
			}
		}
	}
	
	if(d[1] != ZERO)
	{
		d2 += ONE;
		for(i = nc + 3 ; i >= 2 ; i--)
			d[i] = d[i - 1];
	}
	
	d[0] = Sign(nc, ia * ib);
	d[1] = d2;
	
	Normalize(d, c);
	if(debug >= 9)
	{
		puts("Mul 3");
		WriteReal(c);
	}
}

void Real::Muld(float *c, const float *arg_a, double b, long n)
{
	FixedReal a;
	double bb;
	FixedLReal d;
	long ia, ib, n1, n2, i, na;
	Real8 f;
	
	if(err != 0)
	{
		Zero(c);
		return;
	}
	
	copy(arg_a, a);
	
	if(debug >= 9)
	{
		puts("Muld 1");
		WriteReal(a);
		puts("Muld 2");
		printf("%f", b);
		printf("; n=%ld\n", n);
	}
	
	ia = Sign(ONE, a[0]);
	ib = Sign(ONE, b);
	na = Min(ENTIER(ABS(a[0])), curMant);
	if((na == 0) || (b == ZERO))
	{
		Zero(c);
		return;
	}
	
	n1 = Int(n / NBT);
	n2 = n - NBT * n1;
	bb = ABS(b) * ipower(2.0, n2);
	
	Reduce(bb, n1);
	
	if(bb != Int(bb))
	{
		bb = b < ZERO ? -ABS(b) : ABS(b);
		NumbExpToReal(bb, n1 * NBT, f);
		Mul(c, f, a);
	}
	else
	{
		for(i = 2 ; i <= na + 1 ; i++)
			d[i] = bb * a[i];
		d[0] = Sign(na, ia * ib);
		d[1] = a[1] + n1;
		d[na + 2] = ZERO;
		d[na + 3] = ZERO;
		Normalize(d, c);
	}
	
	if(debug >= 9)
	{
		puts("Muld 3");
		WriteReal(c);
	}
}

void Real::Div(float *c, const float *arg_a, const float *arg_b)
{
	FixedReal a, b;
	long ia, ib, na, nb, nc, i3, i2, i, j, j3, md, is, ij;
	double rb, ss, t0, t1, t2;
	bool useOldj;
	FixedLReal d;
	
	if(err != 0)
	{
		Zero(c);
		return;
	}
	
	copy(arg_a, a);
	copy(arg_b, b);

	if(debug >= 8)
	{
		puts("Div 1");
		WriteReal(a);
		puts("Div 2");
		WriteReal(b);
	}
	
	ia = Sign(ONE, a[0]);
	ib = Sign(ONE, b[0]);
	na = Min(ENTIER(ABS(a[0])), curMant);
	nb = Min(ENTIER(ABS(b[0])), curMant);
	
	if(na == 0)
	{
		Zero(c);
		return;
	}
	
	if((nb == 1) && (b[2] == ONE))
	{
		c[0] = Sign(na, ia * ib);
		c[1] = a[1] - b[1];
		for(i = 2 ; i <= na + 1 ; i++)
			c[i] = a[i];
		return;
	}
	
	if(nb == 0)
	{
		puts("*** Div: Divisor is zero!");
		err = DivisionByZero;
		return;
	}
	
	t0 = radix * b[2];
	if(nb >= 2)
		t0 += b[3];
	if(nb >= 3)
		t0 += invRadix * b[4];
	if(nb >= 4)
		t0 += mprx2 * b[5];
	rb = ONE / t0;
	md = Min(na + nb, curMant);
	d[0] = ZERO;
	for(i = 1 ; i <= na ; i++)
		d[i] = a[i + 1];
	for(i = na + 1 ; i <= md + 3 ; i++)
		d[i] = ZERO;
	for(j = 2 ; j <= na + 1 ; j++)
	{
		t1 = mpbx2 * d[j - 2] + radix * d[j - 1] + d[j] + invRadix * d[j + 1];
		t0 = Int(rb * t1);
		j3 = j - 3;
		i2 = Min(nb, curMant + 2 - j3) + 2;
		ij = i2 + j3;
		
		for(i = 3 ; i <= i2 ; i++)
		{
			i3 = i + j3 - 1;
			d[i3] -= t0 * b[i - 1];
		}
		
		if(MOD(j - 1, NPR) == 0)
		{
			for(i = j ; i <= ij - 1 ; i++)
			{
				t1 = d[i];
				t2 = Int(invRadix * t1);
				d[i] = t1 - radix * t2;
				d[i - 1] += t2;
			}
		}
		d[j - 1] += radix * d[j - 2];
		d[j - 2] = t0;
	}
	
	j = na + 2;
	useOldj = false;
	while(1)
	{
		if(j > curMant + 3)
			break;
		t1 = mpbx2 * d[j - 2] + radix * d[j - 1] + d[j];
		if(j <= curMant + 2)
			t1 += invRadix * d[j + 1];
		t0 = Int(rb * t1);
		j3 = j - 3;
		i2 = Min(nb, curMant + 2 - j3) + 2;
		ij = i2 + j3;
		ss = ZERO;
		
		for(i = 3 ; i <= i2 ; i++)
		{
			i3 = i + j3 - 1;
			d[i3] -= t0 * b[i - 1];
			ss += ABS(d[i3]);
		}	
		
		if(MOD(j - 1, NPR) == 0)
		{
			for(i = j ; i <= ij - 1 ; i++)
			{
				t1 = d[i];
				t2 = Int(invRadix * t1);
				d[i] = t1 - radix * t2;
				d[i - 1] += t2;
			}
		}
		
		d[j - 1] += radix * d[j - 2];
		d[j - 2] = t0;
		
		if(ss == ZERO)
		{
			useOldj = true;
			break;
		}
		
		if(ij <= curMant)
			d[ij + 2] = ZERO;
		j++;
	}
	
	if(!useOldj)
		j = curMant + 3;
	d[j - 1] = ZERO;
	is = d[0] == ZERO ? 1 : 2;
	nc = Min(j - 1, curMant);
	d[nc + 2] = ZERO;
	d[nc + 3] = ZERO;
	
	for(i = j ; i >= 2 ; i--)
		d[i] = d[i - is];
	d[0] = Sign(nc, ia * ib);
	d[1] = a[1] - b[1] + is - 2;
	
	Normalize(d, c);
	
	if(debug >= 8)
	{
		puts("Div 3");
		WriteReal(c);
	}
}

void Real::Divd(float *c, const float *arg_a, double b, long n)
{
	FixedReal a;
	double t1, bb, br, dd;
	FixedLReal d;
	long ia, ib, n1, n2, nc, na, j;
	bool ok;
	Real8 f;
	
	if(err != 0)
	{
		Zero(c);
		return;
	}
	
	copy(arg_a, a);
	
	ia = Sign(ONE, a[0]);
	ib = Sign(ONE, b);
	na = Min(ENTIER(ABS(a[0])), curMant);
	
	if(na == 0)
	{
		Zero(c);
		return;
	}
	
	if(b == ZERO)
	{
		puts("*** Divd: Divisor is zero!");
		err = DivisionByZero;
		return;
	}
	
	n1 = Int(n / NBT);
	n2 = n - NBT * n1;
	bb = ABS(b) * ipower(2.0, n2);
	
	Reduce(bb, n1);
	
	if(bb != Int(bb))
	{
		bb = b < ZERO ? -ABS(b) : ABS(b);
		NumbExpToReal(bb, n1 * NBT, f);
		Div(c, a, f);
	}
	else
	{
		br = ONE / bb;
		dd = a[2];
		j = 2;
		ok = true;
		
		while(ok && (j <= curMant + 3))
		{
			t1 = Int(br * dd);
			d[j] = t1;
			dd = radix * (dd - t1 * bb);
			if(j <= na)
				dd += a[j + 1];
			else if(dd == ZERO)
				ok = false;
			j++;
		}
		
		j--;
		nc = Min(j - 1, curMant);
		d[0] = Sign(nc, ia * ib);
		d[1] = a[1] - n1;
		if(j <= curMant + 2)
			d[j + 1] = ZERO;
		if(j <= curMant + 1)
			d[j + 2] = ZERO;
		Normalize(d, c);
	}
}

void Real::Abs(float *z, const float *x)
{
	copy(x, z);
	z[0] = ABS(x[0]);
}

void Real::IntPower(float *b, const float *arg_a, long n)
{
	FixedReal a;
	long na, nws, nn;
	FixedReal r, t;
	
	if(err != 0)
	{
		Zero(b);
		return;
	}
	
	copy(arg_a, a);
	
	na = Min(ENTIER(ABS(a[0])), curMant);
	
	if(na == 0)
	{
		if(n >= 0)
			Zero(b);
		else
		{
			puts("*** ipower: Argument is zero and n is <= 0.");
			err = errIllegalPowerArg;
		}
		return;
	}
	
	nws = curMant;
	curMant++;
	nn = ABS(n);
	if(nn == 0)
	{
		copy(xONE, b);
		curMant = nws;
		return;
	}
	else if(nn == 1)
		copy(a, b);
	else if(nn == 2)
	{
		Mul(t, a, a);
		copy(t, b);
	}
	else
	{
		copy(xONE, b);
		copy(a, r);
		while(1)
		{
			if(ODD(nn))
			{
				Mul(t, b, r);
				copy(t, b);
			}
			nn /= 2;
			if(nn == 0)
				break;
			Mul(r, r, r);
		}
	}
	
	if(n < 0)
	{
		Div(t, xONE, b);
		copy(t, b);
	}
	
	curMant = nws;
	Round(b);
}

long Real::Cmp(const float *a, const float *b)
{
	long ia, ib, ma, mb, na, nb, i;
	
	ia = Sign(ONE, a[0]);
	if(a[0] == ZERO)
		ia = 0;
	ib = Sign(ONE, b[0]);
	if(b[0] == ZERO)
		ib = 0;
		
	if(ia != ib)
		return (Sign(ONE, ia - ib));
	
	ma = Int(a[1]);
	mb = Int(b[1]);
	
	if(ma != mb)
		return (ia * Sign(ONE, ma - mb));
		
	na = Min(ENTIER(ABS(a[0])), curMant);
	nb = Min(ENTIER(ABS(b[0])), curMant);
	
	for(i = 2 ; i <= Min(na, nb) + 1 ; i++)
	{
		if(a[i] != b[i])
			return (ia * Sign(ONE, a[i] - b[i]));
	}
	
	if(na != nb)
		return (ia * Sign(ONE, na - nb));
		
	return 0;
}

void Real::Sqrt(float *b, const float *arg_a)
{
	FixedReal a;
	double t1, t2;
	FixedReal k0, k1, k2;
	bool iq;
	long ia, na, nws, n2, n, k, nw1, nw2, mq;

	if(err != 0)
	{
		Zero(b);
		return;
	}
	
	copy(arg_a, a);

	ia = Sign(ONE, a[0]);
	na = Min(ENTIER(ABS(a[0])), curMant);
	
	if(na == 0)
	{
		Zero(b);
		return;
	}
	
	if(ia < 0)
	{
		puts("*** Sqrt: Argument is negative!");
		err = errNegArgument;
		return;
	}
	
	nws = curMant;
	
	t1 = curMant;
	mq = Int(invLn2 * ::log(t1) + ONE - mprxx);
	
	RealToNumbExp(a, t1, n);
	n2 = -(n / 2);
	t2 = ::sqrt(t1 * ipower(2.0, n + 2 * n2));
	t1 = ONE / t2;
	NumbExpToReal(t1, n2, b);
	curMant = 3;
	iq = false;
	
	for(k = 2 ; k <= mq - 1 ; k++)
	{
		nw1 = curMant;
		curMant = Min(2 * curMant - 2, nws) + 1;
		nw2 = curMant;
		
		while(1)
		{
			Mul(k0, b, b);
			Mul(k1, a, k0);
			Sub(k0, xONE, k1);
			curMant = nw1;
			Mul(k1, b, k0);
			Muld(k0, k1, HALF, 0);
			curMant = nw2;
			Add(b, b, k0);
			if((!iq) && (k == mq - NIT))
				iq = true;
			else
				break;
		}
	}
	
	Mul(k0, a, b);
	nw1 = curMant;
	curMant = Min(2 * curMant - 2, nws) + 1;
	nw2 = curMant;
	Mul(k1, k0, k0);
	Sub(k2, a, k1);
	curMant = nw1;
	Mul(k1, k2, b);
	Muld(k2, k1, HALF, 0);
	curMant = nw2;
	Add(k1, k0, k2);
	copy(k1, b);
	
	curMant = nws;
	Round(b);
}

void Real::Root(float *b, const float *arg_a, long n)
{
	FixedReal a;
	const long maxN = 0x40000000;
	double t1, t2, tn;
	FixedReal k0, k1, k2, k3;
	Real8 f2;
	bool iq;
	short nws;
	long ia, na, n2, k, mq, n1, n3;
	
	if(err != 0)
	{
		Zero(b);
		return;
	}
	
	copy(arg_a, a);
	
	ia = Sign(ONE, a[0]);
	na = Min(ENTIER(ABS(a[0])), curMant);
	
	if(na == 0)
	{
		Zero(b);
		return;
	}
	
	if(ia < 0)
	{
		if(ODD(n))
			a[0] = -a[0];
		else
		{
			puts("*** Root: Argument is negative!");
			err = errNegArgument;
			return;
		}
	}
	
	if((n <= 0) || (n > maxN))
	{
		puts("*** Root: Improper value of n!");
		err = errIllegalRoot;
		return;
	}
	nws = curMant;
	
	if(n == 1)
	{
		copy(a, b);
		b[0] = Sign(b[0], ia);
		return;
	}
	else if(n == 2)
	{
		Sqrt(b, a);
		return;
	}
	
	t1 = curMant;
	mq = Int(invLn2 * ::log(t1) + ONE - mprxx);
	
	Sub(k0, a, xONE);
	if(k0[0] == ZERO)
	{
		copy(xONE, b);
		return;
	}
	RealToNumbExp(k0, t1, n1);
	n2 = Int(invLn2 * ::log(ABS(t1)));
	t1 *= ipower(HALF, n2);
	n1 += n2;
	
	if(n1 <= -30)
	{
		t2 = n;
		n2 = Int(invLn2 * ::log(t2) + ONE + mprxx);
		n3 = -NBT * curMant / n1;
		
		if(n3 < Int(1.25 * n2))
		{
			curMant++;
			Divd(k1, k0, t2, 0);
			Add(k2, xONE, k1);
			k = 0;
			
			while(1)
			{
				k++;
				t1 = 1 - k * n;
				t2 = (k + 1) * n;
		        Muld(k2, k1, t1, 0);
		        Muld(k1, k3, t2, 0);
        		Mul(k3, k0, k1);
        		copy(k3, k1);
        		Add(k3, k1, k2);
        		copy(k3, k2);
				if((k1[0] == ZERO) || (k1[1] < -curMant))
					break;
			}
			copy(k2, b);
			Div(k0, xONE, k2);
			curMant = nws;
			Round(b);
			b[0] = Sign(b[0], ia);
			return;
		}
	}
	
	tn = n;
	RealToNumbExp(a, t1, n1);
	n2 = - Int(n1 / tn);
	t2 = ::exp(-ONE / tn * (::log(t1) + (n1 + tn * n2) * Ln2));
	NumbExpToReal(t2, n2, b);
	NumbExpToReal(tn, 0, f2);
	curMant = 3;
	iq = false;
	
	for(k = 2 ; k <= mq ; k++)
	{
		curMant = Min(2 * curMant - 2, nws) + 1;
		while(1)
		{
			IntPower(k0, b, n);
			Mul(k1, a, k0);
			Sub(k0, xONE, k1);
			Mul(k1, b, k0);
			Divd(k0, k1, tn, 0);
			Add(k1, b, k0);
			copy(k1, b);
			if(!iq && (k == mq - NIT))
				iq = true;
			else
				break;
		}
	}
	
	Div(k1, xONE, b);
	copy(k1, b);
	
	curMant = nws;
	Round(b);
	b[0] = Sign(b[0], ia);
}

void Real::Pi(float *pi)
{
	Real8 f;
	FixedReal An, Bn, Dn, t, r;
	long nws, mq;
	short k;
	double t1;
	
	if(err != 0)
	{
		Zero(pi);
		return;
	}
	
	nws = curMant;
	curMant++;
	
	t1 = nws * ::log10(radix);
	mq = Int(invLn2 * (::log(t1) - ONE) + ONE);
	
	copy(xONE, An);
	f[0] = ONE;
	f[1] = ZERO;
	f[2] = 2.0;
	Sqrt(t, f);
	Muld(Bn, t, HALF, 0);
	f[1] = -ONE;
	f[2] = HALF * radix;
	Sub(Dn, t, f);
	
	for(k = 1 ; k <= mq ; k++)
	{
	    Mul(t, An, Bn);
	    Add(r, An, Bn);
    	Sqrt(Bn, t);
    	Muld(An, r, HALF, 0);
    	Sub(t, An, Bn);
    	Mul(t, t, t);
    	t1 = ipower(2.0, k);
    	Muld(t, t, t1, 0);
    	Sub(Dn, Dn, t);
	}
	
	Add(t, An, Bn);
	Mul(t, t, t);
	Div(pi, t, Dn);
	
	curMant = nws;
	Round(pi);
}

void Real::Entier(float *b, const float *arg_a)
{
	FixedReal a;
	long ia, na, ma, nb, i;
	
	if(err != 0)
	{
		Zero(b);
		return;
	}
	
	copy(arg_a, a);
	
	ia = Sign(ONE, a[0]);
	na = Min(ENTIER(ABS(a[0])), curMant);
	ma = Int(a[1]);
	
	if(na == 0)
	{
		Zero(b);
		return;
	}
	
	if(ma >= curMant)
	{
		puts("*** Entier: Argument is too large!");
		err = errArgTooLarge;
		return;
	}
	
	nb = Min(Max(ma + 1, 0), na);
	if(nb == 0)
	{
		Zero(b);
		return;
	}
	else
	{
		b[0] = Sign(nb, ia);
		b[1] = ma;
		b[nb + 2] = ZERO;
		b[nb + 3] = ZERO;
		for(i = 2 ; i <= nb + 1 ; i++)
			b[i] = a[i];
	}
	
	if(ia == -1)
	{
		for(i = nb + 2 ; i <= na + 1 ; i++)
		{
			if(a[i] != ZERO)
			{
				Sub(b, b, xONE);
				return;
			}
		}
	}
}

void Real::RoundInt(float *b, const float *arg_a)
{
	FixedReal a;
	long ia, na, ma, ic, nc, mc, nb, i;
	Real8 f;
	FixedReal k0;
	
	if(err != 0)
	{
		Zero(b);
		return;
	}
	
	copy(arg_a, a);
	
	ia = Sign(ONE, a[0]);
	na = Min(ENTIER(ABS(a[0])), curMant);
	ma = Int(a[1]);
	
	if(na == 0)
	{
		Zero(b);
		return;
	}
	
	if(ma >= curMant)
	{
		puts("*** RoundInt: Argument is too large!");
		err = errArgTooLarge;
	}
	
	f[0] = ONE;
	f[1] = -ONE;
	f[2] = HALF * radix;
	if(ia == 1)
		Add(k0, a, f);
	else
		Sub(k0, a, f);
	ic = Sign(ONE, k0[0]);
	nc = ENTIER(ABS(k0[0]));
	mc = Int(k0[1]);
	
	nb = Min(Max(mc + 1, 0), nc);
	if(nb == 0)
		Zero(b);
	else
	{
		b[0] = Sign(nb, ic);
		b[1] = mc;
		b[nb + 2] = ZERO;
		b[nb + 3] = ZERO;
		for(i = 2 ; i <= nb + 1 ; i++)
			b[i] = k0[i];
	}
}

void Real::Exp(float *b, const float *arg_a)
{
	FixedReal a;
	const long NQ = 8;
	double t1, t2, tl;
	long ia, na, nws, n1, nz, l1, i;
	FixedReal k0, k1, k2, k3;
	
	if(err != 0)
	{
		Zero(b);
		return;
	} 
	
	copy(arg_a, a);
	
	ia = Sign(ONE, a[0]);
	na = Min(ENTIER(ABS(a[0])), curMant);
	RealToNumbExp(a, t1, n1);
	t1 *= ipower(2, n1);
	
	if((ABS(t1 - Ln2) > invRadix) && (ln2.val == NULL))
	{
		puts("*** Exp: ln2 must be precomputed!");
		err = errLn2ValueMissing;
		return;
	}
	
	if(t1 >= 1.0E9)
	{
		if(t1 > ZERO)
		{ 
			printf("*** Exp: Argument is too large %f x 10^%ld\n", t1, n1);
			err = errExpArgTooLarge;	
		}
		else
			Zero(b);
		return;
	}
	
	nws = curMant;
	curMant++;
	
	if(ABS(t1 - Ln2) > invRadix)
	{
	    Div(k0, a, ln2.val);
    	RoundInt(k1, k0);
    	RealToNumbExp(k1, t1, n1);
		nz = Int(t1 * ipower(2, n1) + Sign(mprxx, t1));
		Mul(k2, ln2.val, k1);
		Sub(k0, a, k2);
	}
	else
	{
		copy(a, k0);
		nz = 0;
	}
	
	tl = k0[1] - curMant;
	
	if(k0[0] == ZERO)
		copy(xONE, k0);
	else
	{
		Divd(k1, k0, ONE, NQ);
		
		copy(xONE, k2);
		copy(xONE, k3);
		l1 = 0;
		
		while(1)
		{
			l1++;
			if(l1 == 10000)
			{
				puts("*** Exp: Iteration limit exceeded!");
				err = errExpIterExceeded;
				curMant = nws;
				return;
			}
			t2 = l1;
			Mul(k0, k2, k1);
      		Divd(k2, k0, t2, 0);
      		Add(k0, k3, k2);
      		copy(k0, k3);

			if((k2[0] == ZERO) || (k2[1] < tl))
				break;
		}
		
		for(i = 1 ; i <= NQ ; i++)
			Mul(k0, k0, k0);
	}
	
	Muld(k1, k0, ONE, nz);
	copy(k1, b);
	
	curMant = nws;
	Round(b);
}

void Real::Ln(float *b, const float *arg_a)
{
	FixedReal a;
	long ia, na, n1, nws, mq, k;
	double t1, t2;
	FixedReal k0, k1, k2;
	bool iq;
	
	if(err != 0)
	{
		Zero(b);
		return;
	}
	
	copy(arg_a, a);
	
	ia = Sign(ONE, a[0]);
	na = Min(ENTIER(ABS(a[0])), curMant);
	if((ia < 0) || (na == 0))
	{
		puts("*** Ln: Argument is less than or equal to zero!");
		err = errIllegalLnArg;
		return;
	}
	
	RealToNumbExp(a, t1, n1);
	if(((ABS(t1 - 2.0) > 1.0E-3) || (n1 != 0)) && (ln2.val == NULL))
	{
		puts("*** Ln: Ln(2) must be precomputed!");
		err = errLn2ValueMissing;
		return;
	}
	
	if((a[0] == ONE) && (a[1] == ZERO) && (a[2] == ONE))
	{
		Zero(b);
		return;
	}
	
	nws = curMant;
	t2 = nws;
	mq = Int(invLn2 * ::log(t2) + ONE - mprxx);
	
	t1 = ::log(t1) + n1 * Ln2;
	NumbExpToReal(t1, 0, b);
	curMant = 3;
	iq = false;
	
	for(k = 2 ; k <= mq ; k++)
	{
		curMant = Min(2 * curMant - 2, nws) + 1;
		while(1)
		{
			Exp(k0, b);		
      		Sub(k1, a, k0);
      		Div(k2, k1, k0);
      		Add(k1, b, k2);
      		copy(k1, b);
			if((k == mq - NIT) && !iq)
				iq = true;
			else
				break;
		}	

	}
	
	curMant = nws;
	Round(b);
}

void Real::SinCos(float *sin, float *cos, const float *arg_a)
{
	FixedReal a;
	double t1, t2;
	short nws;
	Real8 f;
	long ia, na, ka, kb, n1, kc, l1;
	FixedReal k0, k1, k2, k3, k4, k5, k6;

	if(err != 0)
	{
		Zero(sin);
		Zero(cos);
		return;
	}

	copy(arg_a, a);
	
	ia = Sign(ONE, a[0]);
	na = Min(ENTIER(ABS(a[0])), curMant);
	
	if(na == 0)
	{
		copy(xONE, cos);
		Zero(sin);
		return;
	}
	
	if(pi.val == NULL)
	{
		puts("*** SinCos: pi must be precomputed!");
		err = errPiValueMissing;
		return;
	}
	
	nws = curMant;
	curMant++;
	
	Muld(k0, pi.val, 2.0, 0);
  	Div(k1, a, k0);
  	RoundInt(k2, k1);
  	Sub(k3, k1, k2);
  	
  	RealToNumbExp(k3, t1, n1);
  	if(n1 >= -NBT)
  	{
  		t1 *= ipower(2, n1);
  		t2 = 4 * t1;
  		ka = ENTIER(t2 < ZERO ? -ENTIER(HALF - t2) : ENTIER(t2 + HALF));
  		t1 = 8 * (t2 - ka);
  		kb = ENTIER(t1 < ZERO ? -ENTIER(HALF - t1) : ENTIER(t1 + HALF));
  	}
  	else
  	{
  		ka = 0;
  		kb = 0;
  	}
  	
  	t1 = (double) (8 * ka + kb) / 32.0;
	NumbExpToReal(t1, 0, k1);
  	Sub(k2, k3, k1);
  	Mul(k1, k0, k2);
  	  	
  	if(k1[0] == ZERO)
  		Zero(k0);
  	else
  	{
  		copy(k1, k0);
  		Mul(k2, k0, k0);
  		l1 = 0;
  		while(1)
  		{
  			l1++;
  			if(l1 == 10000)
  			{
  				puts("*** SinCos: Iteration limit exceeded!");
  				err = errSinIterExceeded;
  				curMant = nws;
  				return;
  			}
  			t2 = -(2.0 * l1) * (2.0 * l1 + ONE);
			Mul(k3, k2, k1);
      		Divd(k1, k3, t2, 0);
      		Add(k3, k1, k0);
      		copy(k3, k0);

			if((k1[0] == ZERO) || (k1[1] < k0[1] - curMant))
				break;
  		}
  	}

	copy(k0, k1);
	Mul(k2, k0, k0);
	Sub(k3, xONE, k2);
	Sqrt(k0, k3);
	
	kc = ABS(kb);
	f[0] = ONE;
	f[1] = ZERO;
	f[2] = 2.0;
	
	if(kc == 0)
	{
		copy(xONE, k2);
		Zero(k3);
	}
	else
	{
		switch(kc)
		{
			case 1:
				Sqrt(k4, f);
				Add(k5, f, k4);
				Sqrt(k4, k5);
				break;
			case 2:
				Sqrt(k4, f);
				break;
			case 3:
				Sqrt(k4, f);
				Sub(k5, f, k4);
				Sqrt(k4, k5);
				break;
			case 4:
				Zero(k4);
				break;
			default: /* do nothing */
				break;
		}
		Add(k5, f, k4);
		Sqrt(k3, k5);
		Muld(k2, k3, HALF, 0);
		Sub(k5, f, k4);
		Sqrt(k4, k5);
		Muld(k3, k4, HALF, 0);
	}
	
	if(kb < 0)
		k3[0] = -k3[0];
	
	Mul(k4, k0, k2);
	Mul(k5, k1, k3);
	Sub(k6, k4, k5);
	Mul(k4, k1, k2);
	Mul(k5, k0, k3);
	Add(k1, k4, k5);
	copy(k6, k0);
	
	switch(ka)
	{
		case 0:
			copy(k0, cos);
			copy(k1, sin);
			break;
		case 1:
			copy(k1, cos);
			cos[0] = -cos[0];
			copy(k0, sin);
			break;
		case -1:
			copy(k1, cos);
			copy(k0, sin);
			sin[0] = -sin[0];
			break;
		case 2:
		case -2:
			copy(k0, cos);
			cos[0] = -cos[0];
			copy(k1, sin);
			sin[0] = -sin[0];
			break;
	}
	curMant = nws;
	Round(cos);
	Round(sin);
}

void Real::SinhCosh(float *sinh, float *cosh, const float *arg_a)
{
	FixedReal a;
	FixedReal k0, k1, k2;
	short nws;
	
	copy(arg_a, a);
	
	nws = curMant;
	curMant++;
	
	Exp(k0, a);
	Div(k1, xONE, k0);
  	Add(k2, k0, k1);
  	Muld(k2, k2, HALF, 0);
  	copy(k2, cosh);
  	Sub(k2, k0, k1);
  	Muld(k2, k2, HALF, 0);
  	copy(k2, sinh);

	curMant = nws;
	Round(cosh);
	Round(sinh);
}

void Real::ATan2(float *a, const float *arg_x, const float *arg_y)
{
	FixedReal x, y;
	double t1, t2, t3;
	bool iq;
	short nws;
	long ix, iy, nx, ny, mq, n1, n2, kk, k;
	FixedReal k0, k1, k2, k3, k4;
	
	if(err != 0)
	{
		Zero(a);
		return;
	}
	
	copy(arg_x, x);
	copy(arg_y, y);
	
	ix = Sign(ONE, x[0]);
	nx = Min(ENTIER(ABS(x[0])), curMant);
	iy = Sign(ONE, y[0]);
	ny = Min(ENTIER(ABS(y[0])), curMant);
	
	if((nx == 0) && (ny == 0))
	{
		puts("*** ATan2: Both arguments are zero!");
		err = errIllegalAtanArgs;
		return;
	}
	
	if(pi.val == NULL)
	{
		puts("*** ATan2: Pi must be precomputed!");
		err = errPiValueMissing;
		return;
	}
	
	if(nx == 0)
	{
		if(iy > 0)
			Muld(a, pi.val, HALF, 0);
		else
			Muld(a, pi.val, -HALF, 0);
		return;
	}
	else if(ny == 0)
	{
		if(ix > 0)
			Zero(a);
		else
			copy(pi.val, a);
		return;
	}
	
	nws = curMant;
	curMant++;
	
	mq = Int(invLn2 * ::log(nws) + ONE - mprxx);
	
	Mul(k0, x, x);
	Mul(k1, y, y);
	Add(k2, k0, k1);
	Sqrt(k3, k2);
  	Div(k1, x, k3);
  	Div(k2, y, k3);

	RealToNumbExp(k1, t1, n1);
	RealToNumbExp(k2, t2, n2);
	n1 = Max(n1, -66);
	n2 = Max(n2, -66);
	t1 *= ipower(2, n1);
	t2 *= ipower(2, n2);
	t3 = ::atan2(t2, t1);
	NumbExpToReal(t3, 0, a);
	
	if(ABS(t1) <= ABS(t2))
	{
		kk = 1;
		copy(k1, k0);
	}
	else
	{
		kk = 2;
		copy(k2, k0);
	}

	curMant = 3;
	iq = false;
	for(k = 2 ; k <= mq ; k++)
	{
		curMant = Min(2 * curMant - 2, nws) + 1;
		while(1)
		{
			SinCos(k2, k1, a);
			if(kk == 1)
			{
				Sub(k3, k0, k1);
				Div(k4, k3, k2);
				Sub(k1, a, k4);
			}
			else
			{
				Sub(k3, k0, k2);
				Div(k4, k3, k1);
				Add(k1, a, k4);
			}
			copy(k1, a);
			if(!iq && (k == (mq - NIT)))
				iq = true;
			else
				break;
		}
	}
	curMant = nws;
	Round(a);
}


/*
 * User functions
 */

Real Real::Long(double x)
{
	float r[maxMant+3];
	NumbExpToReal(x, 0, r);
	return Real(r, curMant+3);
}

Real Real::Copy(const Real& a)
{
	float b[maxMant+3];
	copy(a.val, b);
	return Real(b, curMant+3);
}

static long GetDigit(long *cc, const char *str, bool *isZero)
{
	char ch;

	// SkipBlanks
	while(str[*cc] == ' ')
		(*cc)++;

	ch = str[*cc];
	if(isdigit(ch))
	{
		(*cc)++;
		if(ch > '0')
			*isZero = false;
		return(ch - '0');
	}
	else
		return -1;
}

static long GetSign(long *cc, const char *str)
{
	// SkipBlanks
	while(str[*cc] == ' ')
		(*cc)++;

	if(str[*cc] == '+')
	{
		(*cc)++;
		return 1;
	}
	else if(str[*cc] == '-')
	{
		(*cc)++;
		return -1;
	}
	else
		return 1;
}

Real Real::ToReal(const char *str) 
{
	FixedReal x;
	toReal(str, x);
	return Real(x, curMant+4);
}

int32 Real::exponent () const
{
	const double log2 = 0.301029995663981195;
	double aa = val[2];
	int32 digs = Min(ENTIER(ABS(val[0])), curMant);
	if(digs >= 2)
		aa += invRadix * val[3];
	if(digs >= 3)
		aa += mprx2 * val[4];
	if(digs >= 4)
		aa += invRadix * mprx2 * val[5];
	double t1 = log2 * NBT * val[1] + ::log10(aa);
	return (t1 >= ZERO ? Int(t1) : Int(t1 - ONE));
}

void Real::toReal(const char *str, float *b)
{
	FixedReal s;
	long nexp, es, is, cc, dig, nws, dp;
	bool isZero;
	Real8 f;

	cc = 0;
	nws = curMant;
	curMant++;
	
	is = GetSign(&cc, str);
	
	Zero(s);
	f[0] = ONE;
	f[1] = ZERO;
	
	isZero = true;
	while(1)
	{
		dig = ::GetDigit(&cc, str, &isZero);
		if(dig < 0)
			break;
		if(!isZero)
			Muld(s, s, 10.0, 0);
		if(dig != 0)
		{
			f[2] = dig;
			Add(s, f, s);
		}
	}
	
	dp = 0;
	if(str[cc] == '.')
	{
		cc++;
		while(1)
		{
			dig = ::GetDigit(&cc, str, &isZero);
			if(dig < 0)
				break;
			Muld(s, s, 10.0, 0);
			if(dig != 0)
			{
				f[0] = ONE;
				f[2] = dig;
			}
			else
				f[0] = ZERO;
			dp++;
			Add(s, f, s);
		}
	}
	
	nexp = 0;
	if((str[cc] == 'E') || (str[cc] == 'D'))
	{
		cc++;
		es = GetSign(&cc, str);
		while(1)
		{
			dig = ::GetDigit(&cc, str, &isZero);
			if(dig < 0)
				break;
			nexp = nexp * 10 + dig;
		}
		nexp *= es;
	}
	
	s[0] *= is;
	nexp -= dp;
	f[0] = ONE;
	f[2] = 10.0;
	IntPower(b, f, nexp);
	Mul(s, b, s);
	copy(s, b);
	
	curMant = nws;
	Round(b);
}

#define AddChar(C)	{ str[pos] = C; pos++; }

static void AddInt(long n, char *str, long& pos)
{
	if(n < 10)
	{
		AddChar(n + '0');
	}
	else
	{
		AddInt(n / 10, str, pos);
		AddChar(n % 10 + '0');
	}
}

char Real::GetDigit(float *frac, long pos, long digs)
{
	long dig = 0;
	Real8 f;
	
	if(digs == 0)
		return '0';
	f[1] = 0;
	if(frac[1] == ZERO)
	{
		dig = Int(frac[2]);
		f[2] = dig;
		f[0] = 1;
	}
	else
	{
		f[0] = 0;
		dig = 0;
	}
	Sub(frac, frac, f);
	Muld(frac, frac, 10.0, 0);
	assert(dig < 10);
	return dig + '0';
}

void Real::ToString(const Real& a, char *str, long n, long dp, char mode)
{
	bool FixPoint, error;
	long Aexp, pos, digs, nws, cnt, round;
	Real8 ten, five;
	FixedReal frac, scale;
	
	pos = 0;
	error = false;
	
	if(a.val[0] < 0)
		AddChar('-');
	digs = Min(ENTIER(ABS(a.val[0])), curMant);
	
	nws = curMant;
	curMant++;
	
	ten[0] = ONE;
	ten[1] = ZERO;
	ten[2] = 10;
	five[0] = ONE;
	five[1] = ZERO;
	five[2] = 5;
	
	FixPoint = (dp != 0);
	if((dp > sigDigs) || !FixPoint)
		dp = sigDigs - 1;
	
	if(digs != 0)
	{
		Aexp = a.exponent();
		IntPower(frac, ten, Aexp);
		Div(frac, a.val, frac);
		frac[0] = ABS(frac[0]);
		
		while(frac[1] < 0)
		{
			Aexp--;
			Muld(frac, frac, 10.0, 0);
		}
		while(frac[2] >= 10.0)
		{
			Aexp++;
			Divd(frac, frac, 10.0, 0);
		}
		
		if(ABS(Aexp) > sigDigs)
			mode = 1;
			
		if(mode == 2)
		{
			cnt = Aexp;
			round = -dp - 1;
			while(MOD(ABS(cnt), 3) != 0)
			{
				cnt--;
				round--;
			}
		}
		else if(mode == 1)
			round = -dp - 1;
		else
			round = Max(-Aexp - dp - 1, -sigDigs-1);
		
		IntPower(scale, ten, round);
		Mul(scale, scale, five);
		Add(frac, frac, scale);
		
		while(frac[1] < 0)
		{
			Aexp--;
			Muld(frac, frac, 10.0, 0);
		}
		while(frac[2] >= 10.0)
		{
			Aexp++;
			Divd(frac, frac, 10.0, 0);
		}
	}
	else
		Aexp = 0;
	
	if(mode > 0)
	{
		AddChar(GetDigit(frac, pos, digs));
		if(mode == 2)
		{
			while(MOD(ABS(Aexp), 3) != 0)
			{
				Aexp--;
				AddChar(GetDigit(frac, pos, digs));
			}
		}
		AddChar('.');
		for(cnt = 1 ; cnt <= dp ; cnt++)
			AddChar(GetDigit(frac, pos, digs));
			
		AddChar('E');
		if(Aexp < 0)
		{
			AddChar('-');
			Aexp = -Aexp;
		}
		else
		{
			AddChar('+');
		}
		AddInt(Aexp, str, pos);
		AddChar('\0');
	}
	else
	{
		if(Aexp < 0)
		{
			AddChar('0');
			AddChar('.');
			cnt = 2;
			while((cnt <= ABS(Aexp)) && (dp > 0))
			{
				AddChar('0');
				dp--;
				cnt++;
			}
		}
		
		cnt = 0;
		while((dp > 0) && (cnt < sigDigs))
		{
			AddChar(GetDigit(frac, pos, digs));
			if(cnt > Aexp)
				dp--;
			else if(cnt == Aexp)
				AddChar('.');
			cnt++;
		}
		AddChar('\0');
		
		cnt = pos - 2;
		while((cnt > 1) && (str[cnt] == '0') && !FixPoint)
		{
			str[cnt] = '\0';
			cnt--;
		}
		
		if(str[cnt] == '.')
			str[cnt] = '\0';
	}
	
	curMant = nws;
}

double Real::Short(const float* q)
{
	double x;
	long exp;
	
	RealToNumbExp(q, x, exp);
	return(x * ipower(2, exp));
}

double Real::Short(const Real& q)
{
	double x;
	long exp;
	
	RealToNumbExp(q.val, x, exp);
	return(x * ipower(2, exp));
}

Real Real::entier(const Real& q)
{
	FixedReal r;	
	Entier(r, q.val);
	return Real(r, curMant+4);
}

Real Real::fraction(const Real& q)
{
	FixedReal r;
	Entier(r, q.val);	
	if(q.val[0] < ZERO)
		Add(r, q.val, xONE);
	Sub(r, q.val, r);
	return Real(r, curMant+4);
}

/*
 * Basic math routines
 */
 
Real Real::operator + (const Real& x) const
{
	return add(*this, x);
}

Real Real::operator - (const Real& x) const
{
	return sub(*this, x);
}

Real Real::operator * (const Real& x) const
{
	return mul(*this, x);
}

Real Real::operator / (const Real& x) const
{
	return div(*this, x);
}

Real Real::operator - (void) const
{
	return negate(*this);
}

float & Real::operator [] (int index) 
{
	if (index>len) 
		return val[len-1];
	else 
		return val[index];
}

float Real::operator [] (int index) const
{
	if (index>len) 
		return val[len-1];
	else 
		return val[index];
}

Real Real::add(const Real& z1, const Real& z2)
{
	FixedReal r;	
	Add(r, z1.val, z2.val);
	return Real(r, curMant+4);
}

Real Real::sub(const Real& z1, const Real& z2)
{
	FixedReal r;	
	Sub(r, z1.val, z2.val);
	return Real(r, curMant+4);
}

Real Real::mul(const Real& z1, const Real& z2)
{
	FixedReal r;	
	Mul(r, z1.val, z2.val);
	return Real(r, curMant+4);
}

Real Real::div(const Real& z1, const Real& z2)
{
	FixedReal r;
	Div(r, z1.val, z2.val);
	return Real(r, curMant+4);
}

Real Real::abs(const Real& z)
{
	FixedReal r;
	Abs(r, z.val);
	return Real(r, curMant+2);
}

Real Real::negate(const Real& z)
{
	FixedReal r;	
	copy(z.val, r);
	r[0] = -z.val[0];
	return Real(r, curMant+2);
}

long Real::sign(const Real& z)
{
	long sign = ENTIER(z.val[0]);
	if(sign < 0)
		return -1;
	else if(sign == 0)
		return 0;
	else
		return 1;
}

long Real::cmp(const Real& a, const Real& b)
{
	return(Cmp(a.val, b.val));
}

Real Real::power(const Real& x, const Real& exp)
{
	FixedReal r;
	Entier(r, exp.val);
	if (Cmp(r, exp.val) == 0)
		IntPower(r, x.val, Int(Short(r)));
	else {
		Ln(r, x.val);
		Mul(r, exp.val, r);
		Exp(r, r);
	}	
	return Real(r, curMant+4);
}

Real Real::root(const Real& z, long n)
{
	FixedReal r;
	Root(r, z.val, n);
	return Real(r, curMant+4);
}

Real Real::sqrt(const Real& z)
{
	FixedReal r;
	Sqrt(r, z.val);
	return Real(r, curMant+4);
}

Real Real::exp(const Real& z)
{
	FixedReal r;
	Exp(r, z.val);
	return Real(r, curMant+4);
}

Real Real::ln(const Real& z)
{
	FixedReal r;
	Ln(r, z.val);
	return Real(r, curMant+4);
}

Real Real::log(const Real& z, const Real& base)
{
	FixedReal r;
	FixedReal t;

	Ln(r, z.val);
	Ln(t, base.val);
	Div(r, r, t);
	return Real(r, curMant+4);
}

Real Real::sin(const Real& z)
{
	float s[maxMant + 4];
	FixedReal c;
	
	SinCos(s, c, z.val);
	return Real(s, curMant+4);
}

Real Real::cos(const Real& z)
{
	FixedReal s;
	float c[maxMant + 4];
	
	SinCos(s, c, z.val);
	return Real(c, curMant+4);
}

void Real::sincos(const Real& z, Real& sin, Real& cos)
{
 	sin.len=curMant+4; cos.len=curMant+4;
	SinCos(sin.val, cos.val, z.val);
}

Real Real::tan(const Real& z)
{
	FixedReal s, c;
	FixedReal r;
	
	SinCos(s, c, z.val);
	Div(r, s, c);
	return Real(r, curMant+4);
}

Real Real::arcsin(const Real& z)
{
	FixedReal t;
	FixedReal r;
	
	Abs(t, z.val);
	if (Cmp(t, xONE) > 0) {
		puts("*** Illegal arcsin argument!");
		err = errIllegalASinArg;
	} else {
		Mul(t, t, t);
		Sub(t, xONE, t);
		Sqrt(t, t);
		ATan2(r, t, z.val);
	}	
	return Real(r, curMant+4);
}

Real Real::arccos(const Real& z)
{
	FixedReal t;
	FixedReal r;
	
	Abs(t, z.val);
	if(Cmp(t, xONE) > 0)
	{
		puts("*** Illegal arccos argument!");
		err = errIllegalACosArg;
	}
	else
	{
		Mul(t, t, t);
		Sub(t, xONE, t);
		Sqrt(t, t);
		ATan2(r, z.val, t);
	}
	
	return Real(r, curMant+4);
}

Real Real::arctan(const Real& z)
{
	FixedReal r;
	ATan2(r, xONE, z.val);
	return Real(r, curMant+4);
}

Real Real::arctan2(const Real& xn, const Real& xd)
{
	FixedReal r;
	ATan2(r, xd.val, xn.val);
	return Real(r, curMant+4);
}

void Real::sinhcosh(const Real& z, Real& sinh, Real& cosh)
{
	sinh.len=curMant+4; cosh.len=curMant+4;
	SinhCosh(sinh.val, cosh.val, z.val);
}

Real Real::sinh(const Real& z)
{
	FixedReal c, s;	
	SinhCosh(s, c, z.val);
	return Real(s, curMant+4);
}

Real Real::cosh(const Real& z)
{
	FixedReal c, s;	
	SinhCosh(s, c, z.val);
	return Real(c, curMant+4);
}

Real Real::tanh(const Real& z)
{
	FixedReal r, sinh, cosh;
	SinhCosh(sinh, cosh, z.val);
	Div(r, sinh, cosh);
	return Real(r, curMant+4);
}

void Real::round(Real& z)
{
	Round(z.val);
}

void Real::nfactorial(long& prevn, long& currentn, float *Result)
{
	long i;
	Real8 xi;
	
	for(i = prevn + 1 ; i <= currentn ; i++)
	{
		NumbExpToReal(i, 0, xi);
		Mul(Result, Result, xi);
	}
	prevn = currentn;
}

/*
 * Misc. routines
 */
 
Real Real::factorial(const Real& x)
{
	long fact, n;
	FixedReal r;
	
	n = ENTIER(Short(x));
	if ((n < 0) || (n > MaxFactorial)) {
		err = errIllegalFactorial;
		return Long(0);
	}
	fact = 0;
	copy(xONE, r);
	nfactorial(fact, n, r);
	return Real(r, curMant+4);
}

Real Real::permutations(const Real& n, const Real& r)
{
	long ni, ri;
	FixedReal res;	

	ni = ENTIER(Short(n));
	ri = (long) (ni - ENTIER(Short(r)));
	if((ni < 0) || (ni > MaxFactorial) || (ri < 0))
	{
		err = errIllegalFactorial;
		return Long(0);
	}	
	copy(xONE, res);
	nfactorial(ri, ni, res);
	return Real(res, curMant+4);
}

Real Real::combinations(const Real& n, const Real& r)
{
	Real res = factorial(r);
	Real per = permutations(n, r);
	Div(res.val, per.val, res.val);
	return Real(res.val, res.len);
}

Real Real::random(void)
{
	Real8 five;
	FixedReal res, t;
	
	NumbExpToReal(5, 0, five);
	Add(t, seed.val, pi.val);
	Ln(t, t); Mul(t, t, five);
	Exp(t, t); Entier(res, t);
	Sub(seed.val, t, res); copy(seed.val, res);
	return Real(res, maxMant+4);
}

/*
 *		Read from or write to the preferences file
 */

void Real::Read(const Preference *r, const char *name, Real &x)
{
	int32 j, len;
	BString lenn(name); lenn.Append("-len");
	r->FindInt32(lenn.String(), &len);
	x.len = len;
	for (int32 i = 0; i < len; i++) {
		r->FindInt32(name, i, &j); x.val[i] = j;
	}
}

void Real::Write(Preference *w, const char *name, const Real &x)
{
	int32 len = Max(8, Min(ENTIER(ABS(x[0])), x.len));
	BString lenn(name); lenn.Append("-len");
	w->AddInt32(lenn.String(), len);
	for (int32 i = 0; i < len; i++) {
		w->AddInt32(name, Int(x[i]));
	}
}

void Real::OutReal(const Real& n)
{
	char str[256];
	ToString(n, str, outDigits, 0, 0);
}

void Real::Test()
{
	Real s;
	Real n;
	Real m;

  printf("pi="); OutReal(pi); puts("");
  printf("ln2="); OutReal(ln2); puts("");
  printf("ln10="); OutReal(ln10); puts("");
  printf("eps="); OutReal(eps); puts("");
  printf("log10(eps)="); OutReal(log(eps, Long(10))); puts("");

  n = "123456789012345678901234567890123456789";
  m = "0.123456789012345678901234567890123456790";
 
 	switch(cmp(n, m))
 	{
 		case 0:
 			printf("n=m");
 			break;
 		case 1:
 			printf("n>m");
 			break;
 		default:
 			printf("n<m");
 			break;
 	}
  puts("");
  printf("n="); OutReal(n); puts("");
  printf("m="); OutReal(m); puts("");
  s =n * m;
  printf("n*m="); OutReal(s); puts("");
  s = n + m;
  printf("n+m="); OutReal(s); puts("");
  s = n - m;
  printf("n-m="); OutReal(s); puts("");  
  s = n / m;
  printf("n/m="); OutReal(s); puts("");
  s = Long(1) / Long(3);
  printf("1/3="); OutReal(s); puts("");
  printf("1/3+1/3="); OutReal(s + s); puts("");
  printf("1/3*1/3="); OutReal(s * s); puts("");
  printf("1/3*3="); OutReal(s * 3); puts("");
  n = 2.0;
  s = power(n, 64);
  printf("2^64="); OutReal(s); puts("");
  n = "1.010E-10";
  printf("1.010E-10="); OutReal(n); puts("");
  n = "-12.0E+10";
  printf("-12.0E+10="); OutReal(n); puts("");
  n = "0.00045E-10";  
  printf("0.00045E-10="); OutReal(n); puts("");
  n = "-12 345 678";  
  printf("-12 345 678="); OutReal(n); puts("");  
  n = "1E10000";  
  printf("1E10000="); OutReal(n); puts("");    
  sincos(pi, m, n);
  printf("Sin(pi)="); OutReal(m); puts("");
  printf("Cos(pi)="); OutReal(n); puts("");
  sincos(pi / 8, m, n);  
  printf("Sin(pi/8)="); OutReal(m); puts("");
  printf("Cos(pi/8)="); OutReal(n); puts("");
  sincos(1, m, n);
  printf("Sin(1)="); OutReal(m); puts("");
  printf("Cos(1)="); OutReal(n); puts("");   
  printf("-8^(-1/3)="); OutReal(root(-8, 3)); puts("");
  printf("(2^64)^(-1/64)="); OutReal(root(power(2, 64), 64)); puts("");
  printf("4*arctan(1)="); OutReal(Long(4) * arctan(1)); puts("");
  printf("arcsin(sin(1))="); OutReal(arcsin(sin(1))); puts("");
  printf("ENTIER(3.6)="); OutReal(entier(3.6)); puts("");
  printf("ENTIER(-3.6)="); OutReal(entier(-3.6)); puts("");
  printf("69!="); OutReal(factorial(69)); puts("");
}

void Real::Init()
{
	typedef float LongFixed[2*(maxMant+4)];
	LongFixed t0, t1, t2, t3, t4;
	
	if (initialized)
		return;

	xONE[0] = ONE;
	xONE[1] = ZERO;
	xONE[2] = ONE;
	
	err = errNone;
	curMant = maxMant + 1;
	numBits = 22;
	debug = 0;

	Pi(t1);
	NumbExpToReal(2, 0, t0);
	Ln(t2, t0);
	ln2 = Real((int)(2 * (maxMant + 4)));
	copy(t2, ln2.val);
	NumbExpToReal(10, 0, t0);
	Ln(t3, t0);
	IntPower(t4, t0, log10eps);
	copy(t1, pi.val);
	copy(t2, ln2.val);
	copy(t3, ln10.val);
	copy(t4, eps.val);
	
	seed = Long(4);
	
	sigDigs = maxDigits;
	
	if(DEBUG)
		Test();
}

void Real::SetDigits(long digits)
{
	int32 words = ENTIER(digits / digsPerWord + HALF);
	curMant = Max(3, words) + 2;	
	if (curMant > maxMant) curMant = maxMant;
	sigDigs = digits;
}
