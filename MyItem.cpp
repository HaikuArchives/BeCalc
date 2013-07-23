#include <strings.h>
#include <stdio.h>

#include "Colors.h"
#include "MyItem.h"

// generic item representation

static const int32 UNUSED = -1;
static const char *INDEX = "itm";
static const char *NAME = "nam";
int32 MyItem::count = 0;

MyItem::MyItem(const char *label) : BListItem(), index(UNUSED)
{
	lab.SetTo(label);
}

MyItem::MyItem(int32 label) : BListItem(), index(label)
{
	lab << index;
}

void MyItem::Update(BView *owner, const BFont *font)
{
	// adjust the maximum width
	float w = font->StringWidth(LabelStr()) + 10;
	if (w > MaxWidth()) SetMaxWidth(w);
	BListItem::Update(owner, font);
}

MyItem::MyItem(const Preference *pref) : BListItem()
{
	pref->FindInt32(INDEX, count, &index);
	if (index == UNUSED) {
		const char *str;
		pref->FindString(NAME, count, &str);
		lab.SetTo(str);
	} else {
		lab << index;
	}
	count++;
}

bool MyItem::Write (Preference *pref)
{
	pref->AddInt32(INDEX, index);
	if (index == UNUSED) {
		pref->AddString(NAME, lab.String());
	}
	return true;
}

void MyItem::DrawItem(BView*owner, BRect frame, bool complete)
{
	// do item selection
	if (IsSelected() || complete) {	
		rgb_color color; 
		if (IsSelected()) { 
			color = BeListSelectGrey;		
		} else {
			color = owner->ViewColor();
		} 			
		owner->SetHighColor(color);
		owner->FillRect(frame); 
	}

	// draw the variable name
	if (IsSelected()) owner->SetDrawingMode(B_OP_OVER);  // IMPORTANT!	
	else owner->SetDrawingMode(B_OP_COPY);  
	owner->MovePenTo(frame.left+4, frame.bottom-2);
	if (IsEnabled()) {
		owner->SetHighColor(Blue); 
	} else {
		owner->SetHighColor(LightBlue); 
	}
	owner->DrawString(lab.String());
	
	// draw the variable value
	owner->MovePenTo(frame.left+4+MaxWidth(), frame.bottom-2);
	if (IsEnabled()) {
		owner->SetHighColor(Black); 
	} else {
		owner->SetHighColor(BeDarkShadow); 
	}
	owner->DrawString(ContentStr());
}

const char* MyItem::LabelStr() const
{
	return lab.String();
}

int32 MyItem::Label() const
{
	return index;
}

const char* MyItem::ContentStr() const
{
	return NULL;
}

const char* MyItem::DragStr() const
{
	return NULL;
}
