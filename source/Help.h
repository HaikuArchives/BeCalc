
#include "MyListView.h"
#include "PWindow.h"
#include "Handler.h"
#include "Settings.h"

#define HELPNAME "BeCalc Commands"

class HelpWin : public PWindow
{
	public:	
		HelpWin(char*, window_type);
		static void InitSettings();
		virtual bool QuitRequested();
		void AddHelp();
			
		MWindow *target;
		MyListView *resultView;
};
