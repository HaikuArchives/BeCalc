#include <stdio.h>
#include <String.h>
#include <Application.h>
#include <Clipboard.h>

#include "Equations.h"
#include "Variables.h"
#include "Statistics.h"

const Real Statistics::Zero(0.0);
Real Statistics::x, Statistics::y, Statistics::Sumx, Statistics::Sumy;
Real Statistics::Sumx2, Statistics::Sumy2, Statistics::Meanx;
Real Statistics::Meany, Statistics::SDevx, Statistics::SDevy;
Real Statistics::Varx, Statistics::Vary, Statistics::Sumxy;
int32 Statistics::n = 0;

static void UpdateN (MStringView *s, int32 n)
{
	char str[32];
	sprintf(str, "n = %ld", n);
	s->SetText(str);
}

void Statistics::InitState(MStringView *s)
{
	bool ok;
	Complex t;
	Clear(s);
	Variables::Get(".Sumx", t, ok); if (ok) Sumx = t.RealPart();
	Variables::Get(".Sumy", t, ok); if (ok) Sumy = t.RealPart();
	Variables::Get(".Sumx2", t, ok); if (ok) Sumx2 = t.RealPart();
	Variables::Get(".Sumy2", t, ok); if (ok) Sumy2 = t.RealPart();
	Variables::Get(".Meanx", t, ok); if (ok) Meanx = t.RealPart();
	Variables::Get(".Meany", t, ok); if (ok) Meany = t.RealPart();
	Variables::Get(".SDevx", t, ok); if (ok) SDevx = t.RealPart();
	Variables::Get(".SDevy", t, ok); if (ok) SDevy = t.RealPart();
	Variables::Get(".Varx", t, ok); if (ok) Varx = t.RealPart();
	Variables::Get(".Vary", t, ok); if (ok) Vary = t.RealPart();
	Variables::Get(".Sumxy", t, ok); if (ok) Sumxy = t.RealPart();
	Variables::Get(".n", t, ok); if (ok) {
		n = (int32)Real::Short(t.RealPart());
		UpdateN(s, n);
	}
}

void Statistics::SaveState()
{
	Variables::Set(".Sumx", Complex(Sumx));
	Variables::Set(".Sumy", Complex(Sumy));
	Variables::Set(".Sumx2", Complex(Sumx2));
	Variables::Set(".Sumy2", Complex(Sumy2));
	Variables::Set(".Meanx", Complex(Meanx));
	Variables::Set(".Meany", Complex(Meany));
	Variables::Set(".SDevx", Complex(SDevx));
	Variables::Set(".SDevy", Complex(SDevy));
	Variables::Set(".Varx", Complex(Varx));
	Variables::Set(".Vary", Complex(Vary));
	Variables::Set(".Sumxy", Complex(Sumxy));
	Variables::Set(".n", Complex(Real::Long(n)));
}

void Statistics::Clear (MStringView *s)
{
	x = y = Sumx = Sumy = Sumxy = Sumx2 = Zero;
	Sumy2 = Meanx = Meany = SDevx = SDevy = Zero;
	Varx = Vary = Zero; n = 0;
	UpdateN(s, n);
}

void Statistics::Display(button_ids varID, MTextView *t)
{
	Complex r(Zero, Zero);
	BString s;
	switch (varID) {
		case ID_AVGX: r = Complex(Meanx, Zero); break;
		case ID_AVGY: r = Complex(Meany, Zero); break;
		case ID_SUMXY: r = Complex(Sumxy, Zero); break;
		case ID_SUMX: r = Complex(Sumx, Zero); break;
		case ID_SUMY: r = Complex(Sumy, Zero); break;
		case ID_SIGX: r = Complex(Varx, Zero); break;
		case ID_SIGY: r = Complex(Vary, Zero); break;
		case ID_SUMX2: r = Complex(Sumx2, Zero); break;
		case ID_SUMY2: r = Complex(Sumy2, Zero); break;
		case ID_DEVX: r = Complex(SDevx, Zero); break;
		case ID_DEVY: r = Complex(SDevy, Zero); break;
		case ID_N: r = Complex(Real::Long(n), Zero); break;
		default: break;
	}
	Complex::ComplexToStr(r, s);
	t->MakeFocus(true);
	t->Insert(s.String());
	t->ScrollToOffset(strlen(t->Text()));
}

Real Statistics::StdDev (const Real &n, const Real &Sum2, const Real &Sum)
{
	return Real::sqrt((Sum2 - Sum*Sum/n)/(n-Real::Long(1)));
}

void Statistics::Addxy (const Real &x1, const Real &y1)
{
	Real t = Real::Long(++n); 
	Sumx = Sumx + x1; Sumy = Sumy + y1;
	Sumxy = Sumxy + x1*y1;
	Sumx2 = Sumx2 + x1*x1;
	Sumy2 = Sumy2 + y1*y1;
	Meanx = Sumx / t; Meany = Sumy / t;
	SDevx = StdDev(t, Sumx2, Sumx); SDevy = StdDev(t, Sumy2, Sumy);
	Varx = SDevx*SDevx; Vary = SDevy*SDevy;
}

void Statistics::StoreToX(MTextView *t, MStringView *s)
{
	Complex x;
	if (Equations::Evaluate(t->Text(), x)) {
		Addxy(x.RealPart(), Zero);
		t->MakeFocus(true);
		t->SelectAll();
		t->Cut(be_clipboard);
		UpdateN(s, n);
	}
}

void Statistics::StoreToXY(MTextView *t, MStringView *s)
{
	BString tstr, str(t->Text());
	int32 pos;
	Complex res, res2;
	if ((pos = str.FindFirst(";")) != B_ERROR) {
		str.MoveInto(tstr, pos, str.Length()-pos);
		tstr.RemoveFirst(";");
		if (Equations::Evaluate(str.String(), res) && Equations::Evaluate(tstr.String(), res2)) {
			Addxy(res.RealPart(), res2.RealPart());
			t->MakeFocus(true);
			t->SelectAll();
			t->Cut(be_clipboard);
			UpdateN(s, n);
		}
	}
}
