#ifndef BECALCAPP_H
#define BECALCAPP_H

#include "MApplication.h"
#include "BeCalcWin.h"

class BeCalcApp : public MApplication
{
	public: BeCalcApp(char *);
			virtual void MessageReceived(BMessage*);	
			BeCalcWin *window;
};

#endif
