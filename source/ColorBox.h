
#include <View.h>
#include "layout.h"

class ColorBox :  public MView, public BView
{
	public:		ColorBox(rgb_color c, minimax size=0);
				~ColorBox();
				void SetBoxColor(rgb_color c);
				virtual	minimax	layoutprefs();
				virtual BRect layout(BRect rect);
				virtual void Draw(BRect rect);
				
	private:	rgb_color color;
};
