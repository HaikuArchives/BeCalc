#ifndef MYITEM_H
#define MYITEM_H

#include <ListItem.h>
#include <Rect.h>
#include <String.h>
#include <View.h>

#include "Preference.h"

class MyItem : public BListItem {
public:
	friend class MyListView;
	MyItem (const char *label);
	MyItem (int32 label);
	MyItem (const Preference *pref);
	virtual ~MyItem() {};
	
	virtual void DrawItem(BView *owner, BRect frame, bool complete = false);
	const char* LabelStr() const;
	int32 Label() const;
	virtual void Update(BView *owner, const BFont *font);
	
	virtual const char* ContentStr() const;
	virtual const char* DragStr() const;

	// override these two routines to store a local max list width
	virtual float MaxWidth() {return 34.0;};
	virtual void SetMaxWidth(float w) {};
	
	virtual bool Write (Preference *pref);
private:
	int32 index;
	static int32 count;
	BString lab;
};

#endif
