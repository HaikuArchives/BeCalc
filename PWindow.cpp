#include <stdio.h>
#include "PWindow.h"
#include "Settings.h"
#include "FFont.h"

static const char *ACTIVE = "-active";

PWindow::PWindow(const char *name, window_type type) 
	: MWindow(Settings::GetPosition(name), name, type, (uint32)0), initialized(false)
{
	// set the window & view attributes
	wname.SetTo(name);
}


void PWindow::WindowActivated(bool active)
{
	MWindow::WindowActivated(active);  // call the parent method
	if (!initialized) {
		MView *v = dynamic_cast<MView *>(ChildAt(0));
		if (v != NULL) {
			rgb_color c = Settings::GetColor(wname.String());
			v->setcolor(c);
			BFont *f[3];
			for (int32 i = M_PLAIN_FONT; i<=M_FIXED_FONT; i++) {			
				f[i] = Settings::GetFont(wname.String(), (fontspec)i);
				BMessage msg(M_FONT_CHANGED);
				if (i==0) AddMessageFont(&msg, "be_plain_font", f[i]);
				else if (i==1) AddMessageFont(&msg, "be_bold_font", f[i]);
				else AddMessageFont(&msg, "be_fixed_font", f[i]);
				DispatchMessage(&msg, this);
			}
			// v->reloadfont(f);  // causes weird behaviour since as labels disappearing
		}
		InitWindow();		
		initialized = true;
	}
}

bool PWindow::QuitRequested()
{
	// save the window & view attributes
	MWindow::QuitRequested();
	MView *mv = dynamic_cast <MView *> (ChildAt(0));
	Settings::SetPosition(wname.String(), Frame());
	if (mv != NULL) {
 		// save the color
		Settings::SetColor(wname.String(), mv->getcolor());
		
		// save the fonts
		const BFont *f[3];
		for (int32 i = M_PLAIN_FONT; i<=M_FIXED_FONT; i++) {
			f[i] = mv->getfont((fontspec)i);
			Settings::SetFont(wname.String(), f[i], (fontspec)i);
		}
	}
	return true;	
}

void PListView::AttachedToWindow()
{	
	 // call parent
	MListView::AttachedToWindow();  
	
	// save the active list item
	BString active(lname); active.Append(ACTIVE);
	Select(Settings::GetInt(active.String()));
	ScrollToSelection();  
		
	// set the font and view color
	SetViewColor(Settings::GetColor(lname.String()));
	SetFont(Settings::GetFont(lname.String(), M_PLAIN_FONT));
}

void PListView::DetachedFromWindow()
{
	MListView::DetachedFromWindow();  // call parent
	
	// save the active list item
	BString active(lname); active.Append(ACTIVE);
	Settings::SetInt(active.String(), CurrentSelection());
	
	// save the color
	Settings::SetColor(lname.String(), ViewColor());
	
	// save the fonts
	BFont font;
	GetFont(&font);
	Settings::SetFont(lname.String(), &font, M_PLAIN_FONT);
}

void PTextView::AttachedToWindow()
{	
	 // call parent
	MTextView::AttachedToWindow();
		
	// set the font and view color
	SetViewColor(Settings::GetColor(tname.String()));
	SetFont(Settings::GetFont(tname.String(), M_PLAIN_FONT));
}

void PTextView::DetachedFromWindow()
{
	MTextView::DetachedFromWindow();  // call parent
	
	// save the color
	Settings::SetColor(tname.String(), ViewColor());
	
	// save the fonts
	BFont font;
	GetFont(&font);
	Settings::SetFont(tname.String(), &font, M_PLAIN_FONT);
}

void PCheckBox::AttachedToWindow()
{	
	 // call parent
	MCheckBox::AttachedToWindow();  
	
	// restore the saved state
	SetValue(Settings::GetInt(cname.String()));
}

void PCheckBox::DetachedFromWindow()
{
	MCheckBox::DetachedFromWindow();  // call parent
	
	// save the check state
	Settings::SetInt(cname.String(), Value());
}

void PSpinButton::AttachedToWindow()
{	
	 // call parent
	SpinButton::AttachedToWindow();  
	
	// restore the saved state
	SetValue(Settings::GetFloat(sname.String()));
}

void PSpinButton::DetachedFromWindow()
{
	SpinButton::DetachedFromWindow();  // call parent
	
	// save the spin button state
	Settings::SetFloat(sname.String(), Value());
}
