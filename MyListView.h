#ifndef MYLISTVIEW_H
#define MYLISTVIEW_H

#include <ListItem.h>

#include "TextRider.h"
#include "PWindow.h"
#include "Preference.h"

// declare a modified BListView which drags its selected item
class MyListView : public PListView
{
	friend class MyItem;
	public:
		MyListView(const char *name, bool drag = true, 
						  list_view_type type=B_SINGLE_SELECTION_LIST) 
						: PListView(name, type), maxWidth(32), listLimit(0), doDrags(drag) {};
		MyListView(const char *name, const char *fname, BListItem (* New (Preference *r)), 
						  bool drag = true);
											 
		// so we can drag and drop list contents
		virtual bool InitiateDrag(BPoint point, int32 index, bool wasSelected);
				
		// determine maximum index
		int32 FindMaxIndex ();
		
		// set a maximum limit to the list size
		void SetListLimit (int32 limit) {listLimit = limit;}
		virtual bool AddItem(BListItem *item);
		virtual bool AddItem(BListItem *item, int32 atIndex);
		
		// export function
		virtual bool Export (TextWriter &w);
		
		// print page count
		virtual int32 GetMaxPages (BRect printArea);
		
		// to save list elements
		virtual bool Write (const char *fname);
		
	protected:
		float maxWidth;  // maximum width of first column
		
	private:
		Preference *pref;
		int32 listLimit;
		bool doDrags;
};

#endif
