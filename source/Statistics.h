#ifndef STATISTICS_H
#define STATISTICS_H

#include "Buttons.h"
#include "Reals.h"
#include "ExComplex.h"
#include "MStringView.h"
#include "MTextView.h"

class Statistics
{
public:
 	// set up default state
 	static void InitState(MStringView *s);
 	static void SaveState();
 	
	// methods
	static void Clear (MStringView *s);
	static void Display(button_ids varID, MTextView *t);
	static void StoreToX(MTextView *t,  MStringView *s);
	static void StoreToXY(MTextView *t,  MStringView *s);
	
private:
	static const Real Zero;
	static Real x, y, Sumx, Sumy, Sumxy, Sumx2, Sumy2, Meanx, Meany;
	static Real SDevx, SDevy, Varx, Vary;
	static int32 n;

	static void Addxy (const Real &x1, const Real &y1);
	static Real StdDev (const Real &n, const Real &Sum2, const Real &Sum);
};

#endif
