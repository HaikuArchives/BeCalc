
/*
     About window for BeCalc.
*/

#include <stdio.h>
#include <InterfaceKit.h>
#include <AppKit.h>
#include <TranslationKit.h>
#include <StorageKit.h>

#include "layout-all.h"

#include "About.h"
#include "Buttons.h"
#include "Colors.h"
#include "Errors.h"
#include "MBitmap.h"
#include "Register.h"
#include "Settings.h"

char AboutWin::vers[32];

static const char *ABOUTLIST = "About-list";

void AboutWin::InitSettings()
{
	Settings::SetColor(ABOUTLIST, White);
	Settings::SetPosition(ABOUTNAME, BRect(100, 100, 300, 300));
}

const char * AboutWin::VERSION()
{
	app_info info;
	version_info v;
	BFile file;
	BAppFileInfo ainfo;
	char *s;
	if (be_app->GetAppInfo(&info) != B_OK) return "-.-.-";
	if (file.SetTo(&info.ref, B_READ_ONLY) != B_OK) return "-.-.-";
	ainfo.SetTo(&file);
	ainfo.GetVersionInfo(&v, B_APP_VERSION_KIND);
	if (v.variety == 1) s = "A";
	else if (v.variety == 2) s = "ß";
	else if (v.variety == 3) s = "G";
	else s = "";
	sprintf(vers, "%ld.%ld.%ld%s", v.major, v.middle, v.minor, s);
	return vers;
}

void AboutWin::GetAppDir (BPath &path)
{
	app_info info;
	if (be_app->GetAppInfo(&info) != B_OK) return;
	BEntry entry(&info.ref, true);
	path.SetTo(&entry);
	path.GetParent(&path);  // ignore executable filename
}

// The MWindow receives font-change messages from the application.
// It also takes care of color-drops. 
AboutWin::AboutWin(char *name, window_type type) : PWindow(name, type)
{
	MView *topview;
	char REGISTER[128] = "UNREGISTERED";
	char *DATE = "26 September 1999";
	char *REGID = "None";
	PTextView *tv;
	
	// read in a bitmap from a file
	BPath p;
	BBitmap *bmap = NULL;
	GetAppDir(p); p.Append("graphics/BeCalc-Medium.jpg");
	BEntry file(p.Path());
	if (file.Exists())
	  bmap = BTranslationUtils::GetBitmapFile(p.Path());
	else
	   Error("Couldn't find graphics/BeCalc-Medium.jpg!");
	
	// define the text box contents
	char str[4096];
	char format[] = "\nBeCalc V%s\n%s\n(%s)\n"\
				 "Registration #%s\n"\
				 "Copyright © 1999 Computer Inspirations\n"\
				 "All Rights Reserved\n"\
				 "Web: http://www.comp-inspirations.com\n"\
				 "Email: mgriebling@comp-inspirations.com\n"\
				 "\nSoftware Design: Michael Griebling\n"\
				 "Software Coding: Kjell V. Normann\n"\
				 "Layout Library Copyright © 1996-1999 Marco Nelissen\n"\
				 "Thanks to Marco Nelissen for PrefServer and\n"\
				 "Dianne Hackborn (& Be Dev Talk members) for FFont\n"\
				 "Extended number algorithms from MPFUN by David H. Bailey\n";

// no Register.h so comment this out
//	if (Register::Registered) {
//		sprintf(REGISTER, "Registered to %s", Register::RegName);
//		DATE = Register::RegDate;
//		REGID = Register::RegisterID;
//	}
//	sprintf(str, format, VERSION(), REGISTER, DATE, REGID);
    			           
    // define the about box contents
	VGroup *about;
    if (bmap == NULL)
		about = new VGroup
		(
			new MBorder(M_DEPRESSED_BORDER, 2, NULL, new Space()),
			new MScrollView(tv = new PTextView(ABOUTLIST, minimax(200, 220)), FALSE, TRUE),
		0);
    else
		about = new VGroup
		(
			new MBorder(M_DEPRESSED_BORDER, 2, NULL, new MBitmap(bmap)),
			new MScrollView(tv = new PTextView(ABOUTLIST, minimax(200, 220)), FALSE, TRUE),
		0);
	tv->SetText(str);
	tv->SetAlignment(B_ALIGN_CENTER);
	tv->MakeEditable(FALSE);
	tv->MakeSelectable(FALSE);
         		
	// Build up the final view
	topview= new MBorder
				(
					M_RAISED_BORDER, 3, "outer", about
				);

	// because topview is an MView, we need to cast it to a BView.
	AddChild(dynamic_cast<BView*>(topview));
	Show();
}


bool AboutWin::QuitRequested()
{
	// unghost the About menu item
	PWindow::QuitRequested();
	if (target->Lock()) {
    	DispatchMessage(new BMessage(ID_CLOSE_ABOUT), target);
    	target->Unlock();
	}	
	
	// shut down this window
	Quit();  		// call the parent
	return true;
}
