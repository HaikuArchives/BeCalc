#include <Path.h>

#include "BinaryRider.h"
#include "Menus.h"
#include "Preferences.h"
#include "Settings.h"

static BMenuItem *prefItem;
static BMenuItem *commandItem;
static BMenuItem *aboutItem;
static BMenuItem *convertItem;
static BMenuItem *removePlots;
static BMenuItem *mortgageItem[3];
static BMenuItem * edit[4];
static BMenu *window;
static Preference *pref = NULL;

static const char *PLOTCOUNT 	= "plotcnt";
static const char *PLOTS			= "plots";

void SetMortgageEnabled(bool enabled)
{
	mortgageItem[1]->SetEnabled(enabled);
	mortgageItem[2]->SetEnabled(enabled);
}

void SetEditEnabled(bool enabled, BTextView *v)
{
	bool redo;
	int32 sels, sele;
	BMessage *clip = (BMessage *)NULL;
	int32 textlen;
	const char *text;
	undo_state state = v->UndoState(&redo);
	for (int32 i = 0; i < 4; i++) edit[i]->SetEnabled(enabled);	
	if (enabled) {
		if (redo) edit[0]->SetLabel("Redo");
		else if (state == B_UNDO_TYPING) edit[0]->SetLabel("Undo Typing");
		else if (state == B_UNDO_CUT) edit[0]->SetLabel("Undo Cut");
		else if (state == B_UNDO_PASTE) edit[0]->SetLabel("Undo Paste");
		else if (state == B_UNDO_CLEAR) edit[0]->SetLabel("Undo Clear");
		else if (state == B_UNDO_CLEAR) edit[0]->SetLabel("Undo Drop");
		else edit[0]->SetEnabled(false);
	}
	v->GetSelection(&sels, &sele);
	if (sels == sele) {
		// nothing to cut or copy
		edit[1]->SetEnabled(false); 
		edit[2]->SetEnabled(false); 
	}
	edit[3]->SetEnabled(false);
	if (be_clipboard->Lock()) {
		if ((clip = be_clipboard->Data()))
			clip->FindData("text/plain", B_MIME_TYPE, (const void **)&text, &textlen);
		be_clipboard->Unlock();
		if (textlen > 0) edit[3]->SetEnabled(true);
	}
	
	// ghost/unghost the Remove Plots menu
	removePlots->SetEnabled(Settings::GetInt("BeCalc-maxplots") > 0);
}

MMenuBar *MakeMenu() 
{
	// Create and add a menu bar
	MMenuBar *menuBar = new MMenuBar(B_ITEMS_IN_ROW);
	BMenu *tempMenu, *subMenu;
	BMenuItem *tempMenuItem;
	
	tempMenu = new BMenu("File");
	tempMenuItem = new BMenuItem("Page Setup...", new BMessage(ID_PAGE_SETUP));
	tempMenu->AddItem(tempMenuItem);
	tempMenu->AddSeparatorItem();
	subMenu = new BMenu("Print");
	tempMenuItem = new BMenuItem("Equations...", new BMessage(ID_PRINT_EQUATIONS));
	subMenu->AddItem(tempMenuItem);
	tempMenuItem = new BMenuItem("Results...", new BMessage(ID_PRINT_RESULTS));
	subMenu->AddItem(tempMenuItem);
	tempMenuItem = new BMenuItem("Variables...", new BMessage(ID_PRINT_VARIABLES));
	subMenu->AddItem(tempMenuItem);
	tempMenuItem = new BMenuItem("All...", new BMessage(ID_PRINT_ALL), 'P');
	subMenu->AddItem(tempMenuItem);
	subMenu->AddSeparatorItem();
	mortgageItem[1] = new BMenuItem("Mortgage List...", new BMessage(ID_PRINT_MORTGAGE));
	subMenu->AddItem(mortgageItem[1]);
	tempMenuItem = new BMenuItem(subMenu);
	tempMenu->AddItem(tempMenuItem);
	tempMenu->AddSeparatorItem();	
	subMenu = new BMenu("Export");
	tempMenuItem = new BMenuItem("Equations...", new BMessage(ID_EXPORT_EQUATIONS));
	subMenu->AddItem(tempMenuItem);
	tempMenuItem = new BMenuItem("Results...", new BMessage(ID_EXPORT_RESULTS));
	subMenu->AddItem(tempMenuItem);
	tempMenuItem = new BMenuItem("Variables...", new BMessage(ID_EXPORT_VARIABLES));
	subMenu->AddItem(tempMenuItem);
	tempMenuItem = new BMenuItem("All...", new BMessage(ID_EXPORT_ALL), 'E');
	subMenu->AddItem(tempMenuItem);	
	subMenu->AddSeparatorItem();
	mortgageItem[2] = new BMenuItem("Mortgage List...", new BMessage(ID_EXPORT_MORTGAGE));
	subMenu->AddItem(mortgageItem[2]);
	tempMenuItem = new BMenuItem(subMenu);
	tempMenu->AddItem(tempMenuItem);
	tempMenu->AddSeparatorItem();		
	tempMenuItem = new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q');
	tempMenu->AddItem(tempMenuItem);
	menuBar->AddItem(tempMenu);
	
	tempMenu = new BMenu("Edit");	
	edit[0] = new BMenuItem("Undo", new BMessage(B_UNDO), 'Z');
	tempMenu->AddItem(edit[0]);	
	tempMenu->AddSeparatorItem();	
	edit[1] = new BMenuItem("Cut", new BMessage(B_CUT), 'X');
	tempMenu->AddItem(edit[1]);
	edit[2] = new BMenuItem("Copy", new BMessage(B_COPY), 'C');
	tempMenu->AddItem(edit[2]);
	edit[3] = new BMenuItem("Paste", new BMessage(B_PASTE), 'V');
	tempMenu->AddItem(edit[3]);
	tempMenu->AddSeparatorItem();
	subMenu = new BMenu("Delete");	
	tempMenuItem = new BMenuItem("Selected", new BMessage(ID_DELETE_SEL), 'D');
	subMenu->AddItem(tempMenuItem);
	tempMenuItem = new BMenuItem("All Equations", new BMessage(ID_DELETE_EQUATIONS));
	subMenu->AddItem(tempMenuItem);
	tempMenuItem = new BMenuItem("All Results", new BMessage(ID_DELETE_RESULTS));
	subMenu->AddItem(tempMenuItem);
	tempMenuItem = new BMenuItem("All Variables", new BMessage(ID_DELETE_VARIABLES));
	subMenu->AddItem(tempMenuItem);
	tempMenuItem = new BMenuItem(subMenu);
	tempMenu->AddItem(tempMenuItem);	
	tempMenu->AddSeparatorItem();
	prefItem = new BMenuItem("Preferences...", new BMessage(ID_PREFERENCES));
	tempMenu->AddItem(prefItem);	
	menuBar->AddItem(tempMenu);
	
	window = new BMenu("Window");
	mortgageItem[0] = new BMenuItem("Open Mortgage...", new BMessage(ID_MORTGAGE));
	window->AddItem(mortgageItem[0]);
	window->AddSeparatorItem();	
	convertItem = new BMenuItem("Convert Units...", new BMessage(ID_CONVERSIONS));
	window->AddItem(convertItem);
	window->AddSeparatorItem();
	tempMenuItem = new BMenuItem("Plot Function...", new BMessage(ID_PLOT));
	window->AddItem(tempMenuItem);
	removePlots = new BMenuItem("Remove Plots", new BMessage(ID_REMOVE_PLOTS));
	window->AddItem(removePlots);
	menuBar->AddItem(window);

	tempMenu = new BMenu("Help");	
	tempMenuItem = new BMenuItem("Buy BeCalc...", new BMessage(ID_REGISTER));
	tempMenu->AddItem(tempMenuItem);
	tempMenu->AddSeparatorItem();
	aboutItem = new BMenuItem("About BeCalc...", new BMessage(ID_ABOUT), 'A');
	tempMenu->AddItem(aboutItem);
	commandItem = new BMenuItem("Display commands...", new BMessage(ID_COMMANDS));
	tempMenu->AddItem(commandItem);	
	tempMenu->AddSeparatorItem();
	tempMenuItem = new BMenuItem("Show user guide...", new BMessage(ID_SHOW_GUIDE));
	tempMenu->AddItem(tempMenuItem);
	tempMenu->AddSeparatorItem();
	tempMenuItem = new BMenuItem("BeCalc online...", new BMessage(ID_WEB_PAGE));
	tempMenu->AddItem(tempMenuItem);
	tempMenuItem = new BMenuItem("Computer Inspirations online...", new BMessage(ID_HOME));
	tempMenu->AddItem(tempMenuItem);
	
	SetMortgageEnabled(false);    // disable Mortage print/export items

	menuBar->AddItem(tempMenu);
	return menuBar;
}

BMenuItem* AddPlotItem (const char *item, int32 code)
// add another plot window item
{
	BMessage *msg = new BMessage(ID_REOPEN_PLOT);
	msg->AddInt32("id", code);
	BMenuItem *tempMenuItem = new BMenuItem(item, msg);
	if (window->CountItems() == 5) window->AddSeparatorItem();
	window->AddItem(tempMenuItem);
	return tempMenuItem;
}

void RemovePlotItem (BMenuItem *item)
{
	window->RemoveItem(item);
	delete item;
}

void SavePlotMenu(BList &menu)
{
	BMenuItem* item;
	int32 index = 0;
	
	// remove from the parent to prevent flicker when removing items
	pref->AddInt32(PLOTCOUNT, menu.CountItems());
	while ((item = (BMenuItem*)menu.ItemAt(index++)) != NULL) {
		pref->AddString(PLOTS, item->Label());
	}
	pref->Save();
	delete pref;	
}

void ReadPlotMenu(BList &menu)
{	
	BString pname("application/x-vnd.CI-BeCalc");
	BMenuItem* item;
	int32 index;
	const char *str;
	
	pname.Append("plotMenu");
	pref = new Preference((char *)pname.String());
	if (pref->Load() == B_OK) {
		// read plot menu items
		pref->FindInt32(PLOTCOUNT, &index);
		for (int32 i = 0; i < index; i++) {
			pref->FindString(PLOTS, i, &str);
			item = AddPlotItem(str, i);
			menu.AddItem(item);
		}
	}
	pref->MakeEmpty();  // clear items
}
