#ifndef ABOUTWIN_H
#define ABOUTWIN_H

#include "PWindow.h"
#include <Path.h>

#define ABOUTNAME "About BeCalc"

class AboutWin : public PWindow
{
	public:	
		AboutWin(char*, window_type);
		virtual bool QuitRequested();
		
		static void InitSettings();
		static void GetAppDir(BPath &path);
		static const char* VERSION();			
		MWindow *target;
			
	private:
		static char vers[32]; 
};

#endif
