#ifndef MBITMAP_H
#define MBITMAP_H

#include "Bitmap.h"
#include "layout.h"
#include <View.h>

class MBitmap :  public MView, public BView
{
	public:		MBitmap(BBitmap *image);
				virtual ~MBitmap();
				virtual	minimax	layoutprefs();
				virtual BRect layout(BRect rect);
				virtual void Draw(BRect rect);
				
	private:	BBitmap *graphic;
};

#endif
