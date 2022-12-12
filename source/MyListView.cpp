#include <strings.h>
#include <stdio.h>
#include <Message.h>

#include "MyItem.h"
#include "MyListView.h"
#include "TextRider.h"

// modified list view implementation

static const char *COUNT = "listcnt";

MyListView::MyListView(const char *name, const char *fname, BListItem (* New (Preference *r)), bool drag) : 
	PListView(name, B_SINGLE_SELECTION_LIST), maxWidth(32), listLimit(0), doDrags(drag)
{
	MyItem *item;
	BString pname("application/x-vnd.CI-BeCalc");
	pname.Append(fname);
	pref = new Preference((char *)pname.String());
	if (pref->Load() == B_OK) {
		int32 cnt = 0;
		MyItem::count = 0;
		pref->FindInt32(COUNT, &cnt);
		for (int32 i = 0; i<cnt; i++) {
			item = (MyItem *)New(pref);
			AddItem(item);
		}
	}
	pref->MakeEmpty();  // clear items
}

#ifdef OMITABH
bool MyListView::InitiateDrag(BPoint point, int32 index, bool wasSelected)
{
	BMessage msg(B_MIME_DATA);
	MyItem *item;
	
	if (wasSelected && doDrags) {
		item = cast_as(ItemAt(index), MyItem);
		const char * text = item->DragStr();
		msg.AddData("text/plain", B_MIME_TYPE, text, strlen(text));
		DragMessage(&msg, ItemFrame(index));  // drag an outline
		return true;
	} else {
		return false;
	}
}
#endif

bool MyListView::Write (const char *fname)
{
	BListItem* item;
		
	// remove from the parent to prevent flicker when removing items
	RemoveSelf();
	MyItem::count = 0;
	pref->AddInt32(COUNT, CountItems());
	while ((item = RemoveItem((int32) 0)) != NULL) {
		((MyItem *)item)->Write(pref);
		delete item;
	}
	bool stat = (pref->Save() == B_OK);
	delete pref;
	return stat;
}

bool MyListView::AddItem (BListItem *item, int32 index)
{
	bool ok = MListView::AddItem(item, index);
	int32 total = CountItems();	
	BListItem *rem;
	if (ok && (listLimit > 0)) {
		// remove some old list items
		while (total > listLimit) {
			rem = RemoveItem((int32)0); total--;
			delete rem;
		}
	}		
	return ok;
}

bool MyListView::AddItem (BListItem *item)
{
	bool ok = MListView::AddItem(item);
	int32 total = CountItems();
	BListItem *rem;
	if (ok && (listLimit > 0)) {
		// remove some old list items
		while (total > listLimit) {
			rem = RemoveItem((int32)0); total--;
			delete rem;
		}
	}		
	return ok;
}

bool MyListView::Export (TextWriter &w)
{
	BListItem* ptr;
	MyItem* item;
	int32 id = 0;
	
	if (w.Err() != brOK) return false;
	while ((ptr = ItemAt(id++)) != NULL) {
		item = (MyItem *)ptr;
		w.WriteString(item->LabelStr());
		w.WriteString("\t");
		w.WriteString(item->ContentStr());
		w.WriteLn();
	}
	return true;
}

int32 MyListView::FindMaxIndex ()
{
	BListItem* item;
	int32 max =  0;
	int32 index = 0;

	while ((item = ItemAt(index++)) != NULL) {
		int32 label = ((MyItem *)item)->Label();
		if (label > max) max = label;
	}
	return max;
}


int32 MyListView::GetMaxPages (BRect printArea)
{
	// assume pages always fit horizontally
	int32 maxItems = CountItems();
	BListItem *item = FirstItem();
	if (item == NULL) return 0;
	return (int32)ceil(maxItems*item->Height()/printArea.Height());
}


