// Simple method of drawing a filled, color box

#include <SupportDefs.h>
#include "ColorBox.h"
#include "Rect.h"
#include "Colors.h"

ColorBox::ColorBox(rgb_color c, minimax size)
	: BView(BRect(0,0,1,1), "", B_WILL_DRAW, B_FOLLOW_NONE)
{	
	ct_mpm=size;
	color=c;  // store the bit map
};

ColorBox::~ColorBox() {
}

// A minimal layoutprefs()
// It simply returns the minimax you gave it.
minimax ColorBox::layoutprefs()
{
	return mpm=ct_mpm;
}

// Basic resizing. The top-level classes guarantee that your
// minimum and maximum values are respected, so you need not
// concern yourself with that.
BRect ColorBox::layout(BRect rect)
{
	ResizeTo(rect.Width(),rect.Height());
	MoveTo(rect.LeftTop());
	Draw(rect);
	return rect;
}

void ColorBox::SetBoxColor (rgb_color c)
{
	color = c;
	Draw(Bounds());
}

// just a simple draw-function, so there's something to look at.
void ColorBox::Draw(BRect)
{
	BRect r=Bounds();
	
	// fill the central box
	SetHighColor(color);
	r.InsetBy(2.0, 2.0);
	FillRect(r);
	
	// draw the shadow
	r.InsetBy(-2.0, -2.0);
	SetHighColor(BeDarkShadow);
	StrokeLine(r.LeftBottom(), r.LeftTop());
	StrokeLine(r.RightTop());
	r.InsetBy(1.0, 1.0);
	SetHighColor(BeShadow);
	StrokeLine(r.LeftBottom(), r.LeftTop());
	StrokeLine(r.RightTop());	
	
	// draw the highlight
	SetHighColor(BeLightShadow);
	StrokeLine(r.LeftBottom(), r.RightBottom());
	StrokeLine(r.RightTop());
	r.InsetBy(-1.0, -1.0);
	SetHighColor(BeHighlight);
	StrokeLine(r.LeftBottom(), r.RightBottom());
	StrokeLine(r.RightTop());	
}
