// Simple method of drawing a bitmap

#include "MBitmap.h"
#include "Rect.h"
#include <View.h>

MBitmap::MBitmap(BBitmap * image)
		: BView(image->Bounds(),"", B_WILL_DRAW,0)
{
	// ct_mpm is the size at construction. Layout will always take
	// place based on this value, so make sure it is initialized.
	BRect b = image->Bounds();
	
	ct_mpm=minimax(b.Width(), b.Height());
	graphic=image;  // store the bit map
};

MBitmap::~MBitmap() {
	delete graphic;  // dispose the bitmap
}

// A minimal layoutprefs()
// It simply returns the minimax you gave it.
minimax MBitmap::layoutprefs()
{
	return mpm=ct_mpm;
}

// Basic resizing. The top-level classes guarantee that your
// minimum and maximum values are respected, so you need not
// concern yourself with that.
BRect MBitmap::layout(BRect rect)
{
	BRect b = graphic->Bounds();
	float r = b.Height()/b.Width();
	float nw = rect.Width();
	float nh = r * nw;
	
	ResizeTo(nw, nh);
	MoveTo(rect.LeftTop());
	Draw(rect);
	return rect;
}

// just a simple draw-function, so there's something to look at.
void MBitmap::Draw(BRect)
{
	BRect r;

	r=Bounds();
	DrawBitmap(graphic, r);
}
