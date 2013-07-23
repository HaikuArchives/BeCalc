#ifndef LISTITEM_H
#define LISTITEM_H

#include "MyItem.h"
#include "Preference.h"

class ListItem : public MyItem
{
public:
	ListItem (int32 index, const char * str) : MyItem(index), content(str) {};
	ListItem (const Preference *r);
	virtual const char* ContentStr() const {return content.String();};
	virtual void SetContent(const char *str) {content.SetTo(str);};
	virtual const char* DragStr() const {return content.String();};
	virtual float MaxWidth() {return maxWidth;};
	virtual void SetMaxWidth(float w) {maxWidth = w;};
	virtual bool Write (Preference *w);
private:
	BString content;
	static float maxWidth;
};

#endif
