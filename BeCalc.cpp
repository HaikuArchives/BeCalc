#include "BeCalcApp.h"
#include "BeCalcWin.h"
#include "Colors.h"
#include "Errors.h"
#include "Register.h"
#include "Equations.h"
#include "Settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <Alert.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <Path.h>
#include <Roster.h>
#include "About.h"
//#include "Entry.h"

//
// main program
//
int main()
{
	BeCalcApp app("application/x-vnd.CI-BeCalc");
			
	Equations::InitEquations();
#ifdef REGISTER
//don't check registration
	// initialize registration module
	Register::InitRegister();
	
	// initialize the equations and dependent modules
	Equations::InitEquations();
	if (1) {	// dont bother checking registration
//	if (!Register::Registered) {
//		Error("Unregistered BeCalc.  Please ensure your key file is installed in the BeCalc/misc directory.");
//		exit(1);
	} else {
		if (Register::EndDate < 0) {
			Error("We hope you enjoyed your free evaluation of BeCalc.  For a permanent key file please register BeCalc today.  For details check our web site at:\nhttp://www.comp-inspirations.com.");
			exit(1);
		} else if (Register::EndDate < 5) {
			char text[160];
			sprintf(text, "There are only %ld days left in your BeCalc evaluation.  Register today to ensure uninterrupted use of BeCalc.  For details check our web site at:\nhttp://www.comp-inspirations.com.", Register::EndDate);
			Error(text);
		}
	}
#endif
	// set default settings for all windows
	BeCalcWin::InitSettings();
	Settings::Load();
	
	// create the main window
	app.window=new BeCalcWin(BECALCNAME, B_TITLED_WINDOW);
	
	// the MWindow that our "window" really is will take care of everything.	
	app.window->Show();
	
	// create links to certain directories -- if not already there
	BPath path;
	entry_ref ref;
//	Register::GetAppDir(path);
	path.Append("misc/CreateLinks");		// the book mark
	if (get_ref_for_path(path.Path(), &ref) == B_OK) {
		be_roster->Launch(&ref);		// start up link program
		BEntry entry(&ref);				// delete the link program
		entry.Remove();					// poof -- it's gone
	}
	app.Run();
	Settings::Save();
	return(0);
}
