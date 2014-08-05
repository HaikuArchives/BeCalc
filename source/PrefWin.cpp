
/*
     Mortgage window for BeCalc.
*/
#include <Slider.h>
#include <Application.h>
#include <ScrollView.h>
#include <ColorControl.h>
#include <Rect.h>

#include "layout-all.h"

#include "PrefWin.h"
#include "Buttons.h"
#include "Colors.h"
#include "Equations.h"
#include "Variables.h"
#include "ExComplex.h"

static const char *PREFLIST = "Pref-list";

// The MWindow receives font-change messages from the application.
// It also takes care of color-drops. 
PrefWin::PrefWin(char *name,window_type type) : PWindow(name,type)
{
	MView *topview;
	
	// define a preferences list box
	MBorder *listBorder = 
		new MBorder(M_DEPRESSED_BORDER, 2, NULL, 
							list = new PListView(PREFLIST, B_SINGLE_SELECTION_LIST, minimax(150, 40, 10000, 10000)));
	list->SetSelectionMessage(new BMessage(PID_PREF_SELECTED));
	
	// add the preference items
	list->AddItem(new BStringItem("Digit Separators"));
	list->AddItem(new BStringItem("Numerical Format"));
	list->AddItem(new BStringItem("Equation & Result Settings"));
	list->AddItem(new BStringItem("Variable Settings"));
	
	// define the numbers preferences
	MBorder *digitSeps = new MBorder
		(
			M_LABELED_BORDER, 5, "Digit Separators",
			new VGroup
			(		
				lDP = new MPopup("Left of dp:","Apostrophe (1'234'567.0)","Comma (1,234,567.0)",
											"Underscore (1_234_567.0)","Period (1.234.567,0)", 
											"None (1234567.0)", 0),
				rDP = new MPopup("Right of dp:", "None", "Same as separator", 0),
			0)
		);
		
	// give these the same spacing
	DivideSame(lDP, rDP, 0);
	
	// number format	
	MBorder *numFormat = new MBorder
		(
			M_LABELED_BORDER, 5, "Number Format",
		 	new VGroup
			(
				digs = new PSpinButton("Digits:", SPIN_INTEGER),
				dec = new PSpinButton("Decimals:", SPIN_INTEGER),				
			0)
		);
		
	// set minimum and maximums
	DivideSame(dec, digs, 0);
	digs->SetMaximum(203); digs->SetMinimum(16);
	dec->SetMaximum(203); dec->SetMinimum(0);
	
	// equation and result settings
	MBorder *eqSettings = new MBorder
		(
			M_LABELED_BORDER, 5, "Equation & Result Settings",
			new VGroup 
			(
				uniquebox = new PCheckBox(EQUNIQUE, PID_UNIQUE, false),
				deletebox = new PCheckBox(EQDELETE, PID_DELETE_BOTH, false),
				unlimited = new PCheckBox(EQUNLIMITED, PID_UNLIMITED, true),
				eqLimit = new PSpinButton(EQLIMIT, SPIN_INTEGER),
			0)
		);
		
	// set minimum and maximums
	eqLimit->SetMaximum(10000); eqLimit->SetMinimum(0); eqLimit->SetStepSize(100);
	eqLimit->SetEnabled(false);
		
	// variable settings
	MBorder *varSettings = new MBorder
		(
			M_LABELED_BORDER, 5, "Variable Settings",
			new VGroup 
			(
				sortbox = new PCheckBox(VARSORT, PID_SORTED_VARS, false),
				casebox = new PCheckBox(VARCASE, PID_SENSITIVE_VARS, true),						
			0)
		);

	// bottom button group
	HGroup *buttons = new HGroup
		(
			new MButton("Factory Defaults", PID_DEFAULT),
			new MButton("Revert", PID_REVERT),				
		0);
		
	// Set up the preferences in a list-chosen view
	settings = new LayeredGroup(digitSeps, numFormat, eqSettings, varSettings, 0);
             		
	// Build up the final view
	topview= new MBorder
				(
					M_RAISED_BORDER, 3, "outer",
					new VGroup (
						new HGroup(listBorder, settings, 0),
						buttons,		
					0)
				);

	// because topview is an MView, we need to cast it to a BView.	
	AddChild(dynamic_cast<BView*>(topview));
	Show();
}

void PrefWin::InitSettings()
{
	Settings::SetPosition(PREFNAME, BRect(100,100, 300, 300));
	Settings::SetColor(PREFLIST, White);
}

void PrefWin::InitWindow()
{
	// activate the appropriate layer
	int32 active = list->CurrentSelection();
	if (active > 0) 
		settings->ActivateLayer(active);
	else {
		list->Select(0);
		settings->ActivateLayer(0);
	}
				
	// only enable appropriate items
	int32 total = Equations::Equation->CountItems();
	eqLimit->SetMinimum(total);
			
	// remember old settings
	olddigs = Complex::GetDigits();
	olddp = Complex::GetDecPoint();
	oldeqLimit = Equations::HistoryLimit(); 
	oldunlimited = Equations::IsUnlimited();
	oldleftSep = Complex::GetDigitSep();
	oldrightSep = Complex::GetFracSep();
	oldsorted = Variables::IsSorted();
	oldsensitive = Variables::IsSensitive();
	oldunique = Equations::IsUnique();
	olddelboth = Equations::IsDeleteBoth();
	
	// set default items
	SetState(olddigs, olddp, oldleftSep, oldrightSep, oldeqLimit,
			 	 oldsorted, oldsensitive, oldunique, olddelboth, oldunlimited);
}

bool PrefWin::QuitRequested() 
{	
	// unghost the preferences menu item
	PWindow::QuitRequested();
	if (target->Lock()) {
    	DispatchMessage(new BMessage(ID_CLOSE_PREFS), target);
    	target->Unlock();
	}
	
	// shut down this window
	Quit();  		// call the parent
	return true;
}

void PrefWin::SetState (int32 digits, int32 dp, char leftSep, char rightSep,
						int32 eqlimit, bool sorted, bool sensitive,
						bool unique, bool delboth, bool nolimit)
{	
	int32 item;
	
	// set digits and decimal point				
	digs->SetValue(digits);
	Complex::SetDigits(digits);										
	dec->SetValue(dp);
	Complex::SetDecPoint(dp);
	
	// set up separators
	switch (leftSep)
	{
		case '\'': item = 0; break;
		case ',' : item = 1; break;
		case '_' : item = 2; break;
		case '.' : item = 3; break;
		default  : item = 4; break;
	}
	lDP->SetActive(item);
	if (rightDP == '\0') item = 0;
	else item = 1;
	rDP->SetActive(item);
	
	// set active equation limit
	if (nolimit) {
		Equations::Equation->SetListLimit(0);
		Equations::Results->SetListLimit(0);
	} else {
		Equations::Equation->SetListLimit(eqlimit);
		Equations::Results->SetListLimit(eqlimit);
	}	
	eqLimit->SetValue(eqlimit);
	eqLimit->SetEnabled(!nolimit);
	
	//set up the various flags
	Variables::SetSorted(sorted);
	sortbox->SetValue(sorted);
	Variables::SetCaseSensitive(sensitive);
	casebox->SetValue(sensitive);
	Equations::SetUnique(unique);
	uniquebox->SetValue(unique);	
	deletebox->SetValue(delboth);
	unlimited->SetValue(nolimit);
	
	// force parent window to be updated
	if (target->Lock()) {
		DispatchMessage(new BMessage(ID_UPDATE_NUMS), target);
		DispatchMessage(new BMessage(ID_UPDATE_VARS), target);
		target->Unlock();
	}
}


// Always call the inherited MessageReceived(), unless you don't
// want your windows to react to font-changes and color-drops.
void PrefWin::MessageReceived(BMessage *message)
{
	int32 id;
	BString oldName, newName;
	void * ptr;
	bool flag;	
	
	switch(message->what)
	{
		case M_BUTTON_SELECTED:
			id = message->FindInt32("butID");
			switch (id)
			{
				case PID_DEFAULT:
					// factory defaults are hard-coded
					SetState();	
					break;
				case PID_REVERT:
					// use cached values
					SetState(olddigs, olddp, oldleftSep, oldrightSep,
							 oldeqLimit, oldsorted, oldsensitive,
							 oldunique, olddelboth, oldunlimited);
					break;		
				default:
					break;
			}			
			break;
		case M_SPIN_TICK:
			Complex::SetDigits(digs->Value());
			digs->SetValue((int32)Complex::GetDigits());
			if (digs->Value() < dec->Value()) id = (int32)digs->Value();
			else id = (int32)dec->Value();
			Complex::SetDecPoint(id);
			dec->SetValue(id);
			id = (int32)eqLimit->Value();
			eqLimit->SetValue(id);	
			Equations::Equation->SetListLimit(id);
			Equations::Results->SetListLimit(id);
			if (target->Lock()) {
				DispatchMessage(new BMessage(ID_UPDATE_NUMS), target);
				DispatchMessage(new BMessage(ID_UPDATE_VARS), target);
				target->Unlock();
			}
			break;
		case M_CHECKBOX_SELECTED:
			id = message->FindInt32("checkID");
			flag = message->FindInt32("be:value");
			switch (id) {
				case PID_SORTED_VARS:
					Variables::SetSorted(flag);
					if (target->Lock()) {
						DispatchMessage(new BMessage(ID_UPDATE_VARS), target);
						target->Unlock();
					}
					break;
				case PID_SENSITIVE_VARS: 
					Variables::SetCaseSensitive(flag);
					if (target->Lock()) {
						DispatchMessage(new BMessage(ID_UPDATE_VARS), target);
						target->Unlock();
					}
					break;
				case PID_UNIQUE: Equations::SetUnique(flag); break;
				case PID_DELETE_BOTH: Settings::SetInt(EQDELETE, flag); break;
				case PID_UNLIMITED: 
					eqLimit->SetEnabled(!flag);
					Settings::SetInt(EQUNLIMITED, flag);
					break;
				default: break;
			}
		case PID_PREF_SELECTED: {
			// change to a different preference view
			int32 active = list->CurrentSelection();
			if (active >= 0) settings->ActivateLayer(active); 
			}
			break;
		case M_POPUP_SELECTED:
			id = message->FindInt32("index");
			message->FindPointer("popup", &ptr);
			if (ptr == lDP) {
				// left digit separator selected
				bool same = (leftDP == rightDP);
				switch (id)
				{
					case 0: leftDP = '\''; break;
					case 1: leftDP = ','; break;
					case 2: leftDP = '_'; break;
					case 3: leftDP = '.'; break;
					default: leftDP = '\0'; break;
				}
				Complex::SetDigitSep(leftDP);
				if (same) rightDP = leftDP;
				Complex::SetFracSep(rightDP);
				if (target->Lock()) {
					DispatchMessage(new BMessage(ID_UPDATE_NUMS), target);
					target->Unlock();
				}
			} else if (ptr == rDP) {
				// right digit separator selected
				if (id==0) rightDP = '\0';
				else rightDP = leftDP;		
				Complex::SetFracSep(rightDP);
				if (target->Lock()) {
					DispatchMessage(new BMessage(ID_UPDATE_NUMS), target);
					target->Unlock();
				}
			}
			break;
		default:
			// printf("received by window:\n");
			// message->PrintToStream();
			// printf("----------\n");
			MWindow::MessageReceived(message);
			break;
	}
}
