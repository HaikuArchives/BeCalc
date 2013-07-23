#include <InterfaceKit.h>
#include <stdio.h>
#include "layout-all.h"
#include <TranslationUtils.h>
#include <FilePanel.h>
#include <Roster.h>
#include <Application.h>
#include <Clipboard.h>

#include "BeCalcWin.h"
#include "Colors.h"
#include "Buttons.h"
#include "Equations.h"
#include "ExComplex.h"
#include "Menus.h"
#include "MBitmap.h"
#include "Printer.h"
#include "Reals.h"
#include "Statistics.h"

// preferences names
static const char* EDITSTR				= "BeCalc-editcolor";
static const char* KEYTAB				= "BeCalc-keytab";
static const char* PREFIX				= "BeCalc-prefix";
static const char* MAXPLOTS		= "BeCalc-maxplots";
static const char* LISTTAB				= "BeCalc-listtab";
static const char* VARLIST			= "BeCalc-varlist";
static const char* DISPSTATE		= "BeCalc-dispstate";
static const char* LASTRESULT 		= "BeCalc-lastresult";
static const char* LASTRESULTN	= ".lastResult";
static const char* OPENWINDOWS	= "BeCalc-openwins";

// local redefinition of MTextView
void MyTextView::KeyDown (const char *bytes, int32 numBytes)
{
	if (bytes[0] == B_ENTER) {
		// send a message back to main window
		BMessage msg(M_BUTTON_SELECTED);
		msg.AddInt32("butID", ID_ENTER);
		Window()->DispatchMessage(&msg, Window());
	} else {
		((BeCalcWin *)Window())->state = BeCalcWin::dsAppend;
		MTextView::KeyDown(bytes, numBytes);
	}
}

void MyTextView::MakeFocus (bool flag)
{
	MTextView::MakeFocus(flag);    // do the parent stuff
	MScrollView *v = (MScrollView *)Parent();
	v->SetBorderHighlighted(flag);
}

bool MyTextView::AcceptsDrop(const BMessage *message)
{
	((BeCalcWin *)Window())->state = BeCalcWin::dsAppend;
	return MTextView::AcceptsDrop(message);
}

void BeCalcWin::InitSettings()
{
	// default settings for BeCalc window
	Settings::SetPosition(BECALCNAME, BRect(100,100,200,200));
	Settings::SetColor(EDITSTR, White);
	Settings::SetColor(VARLIST, White);
	Settings::SetColor(EQNAME, White);
	Settings::SetColor(RESNAME, White);
	Settings::SetInt(KEYTAB, 0);
	Settings::SetInt(PREFIX, 0);
	Settings::SetInt(MAXPLOTS, 0);
	Settings::SetInt(LISTTAB, 0);
	Settings::SetInt(DISPSTATE, dsClear);
	Settings::SetString(LASTRESULT, "0");
	Settings::SetInt(OPENWINDOWS, wNone);
	
	// other window settings
	AboutWin::InitSettings();
	Equations::InitSettings();
	Variables::InitSettings();
	HelpWin::InitSettings();
	ConvertWin::InitSettings();
	Complex::InitSettings();
	MortgageWin::InitSettings();
	PrefWin::InitSettings();
}

// The MWindow receives font-change messages from the application.
// It also takes care of color-drops. 
BeCalcWin::BeCalcWin(char *name, window_type type) : PWindow(name,type), lastResult(0.0)
{
	const BFont *f = be_plain_font;
	float bwidth = f->StringWidth(" Expp ");
    minimax bsize = minimax(bwidth, -1, 1000, 1000, 1);
  	minimax b2size = minimax(2*bwidth, -1, 1000, 1000, 1);
	minimax lsize = minimax(f->StringWidth(" cosh¯¹ "), -1, 1000, 1000, 1);
	minimax psize = minimax(f->StringWidth(" nand "), -1,1000, 1000, 1);
	minimax ssize = minimax(-1, -1, -1, -1, 0.1);
	MView *topview;
	TabGroup *tab2;
			    
    // define the top display area
    plotCount = 0;  save = NULL;
    
    // clear windows
    pwin = NULL; cwin = NULL; awin = NULL; hwin = NULL; prefwin = NULL; win = NULL;
        
 	// read in a bitmap from a file
 	BPath p;
	MBorder *border;
	AboutWin::GetAppDir(p); p.Append("graphics/CI-Logo.jpg");	
	BEntry file(p.Path());
	if (file.Exists()) {
		BBitmap *bmap= BTranslationUtils::GetBitmapFile(p.Path());
		border = new MBorder(M_DEPRESSED_BORDER, 2, NULL, new MBitmap(bmap));
	} else {
		Error("Couldn't find graphics/CI-Logo.jpg!");
		border = new MBorder(M_DEPRESSED_BORDER, 2, NULL, new Space());
	}
	
    HGroup *displayArea =
    new HGroup
    (
    	new VGroup  // left-most group of pop-up menus
    	(
    		minimax(-1, -1, -1, -1, 1),
    		border,
    		new Space(),
    		angle  	= new MPopup("Angle:","Degrees","Radians","Gradians",0),
    		format	= new MPopup("Format:","Standard","Scientific","Engineering",0),
    		radix   = new MPopup("Radix:", "Binary", "Trinary", "Base 4", "Base 5", "Base 6", "Base 7", 
    										  "Octal", "Base 9", "Decimal", "Base 11", "Duodecimal", "Base 13",
    										  "Base 14", "Base 15", "Hexadecimal", 0),
    		new Space(), 		
    	0),
    	new VGroup // list display and equation display
    	(
    		ltab = new TabGroup
    		(
    			"Equations", 
    			s1 = new MScrollView
    			(
    				Equations::Equation, true, true
    			),
    			"Results",
    			s2 = new MScrollView
    			(
    				Equations::Results, true, true
    			),    			
    			"Variables",
    			s3 = new MScrollView
    			(
    				Vars = new MyListView(VARLIST), true, true
    			),    			
    		0),
    	0),
    0);
    
    // line up the pop-ups
    Variables::CopyVars(Vars);
    DivideSame(angle, format, radix, 0);
           
    // define the left, central, and right key clusters
	HGroup *buttons=
	new HGroup
	(
		new VGroup  // leftmost key cluster
		(
			tab = new TabGroup
			(
				"Trig",
				new VGroup  // 3 x 6 cluster of scientific functions
				(
					new HGroup
						(MakeButton(ID_NCR, lsize), MakeButton(ID_SIN, lsize), MakeButton(ID_ASIN, lsize), 0),
					new HGroup
					  	(MakeButton(ID_NPR, lsize), MakeButton(ID_COS, lsize), MakeButton(ID_ACOS, lsize), 0),
					new HGroup
					  	(MakeButton(ID_COT, lsize), MakeButton(ID_TAN, lsize), MakeButton(ID_ATAN, lsize), 0),
					new HGroup
					  	(MakeButton(ID_ACOT, lsize),  MakeButton(ID_SINH, lsize), MakeButton(ID_ASINH, lsize), 0),
					new HGroup
						(MakeButton(ID_ACOTH, lsize),  MakeButton(ID_COSH, lsize), MakeButton(ID_ACOSH, lsize), 0),
					new HGroup
						(MakeButton(ID_COTH, lsize),  MakeButton(ID_TANH, lsize), MakeButton(ID_ATANH, lsize), 0),
					0
				),
				"Prog",
				new VGroup  // 4 x 6 cluster of logic functions
				(
					new HGroup
						(MakeButton(ID_NAND, psize), MakeButton(ID_NOR, psize), MakeButton(ID_CNT, psize), MakeButton(ID_A, bsize), 0),
					new HGroup
						(MakeButton(ID_MOD, psize), MakeButton(ID_CBIT, psize), MakeButton(ID_TBIT, psize), MakeButton(ID_B, bsize),0),
					new HGroup
						(MakeButton(ID_DIV, psize), MakeButton(ID_SBIT, psize), MakeButton(ID_NEG, psize), MakeButton(ID_C, bsize),0),
					new HGroup
						(MakeButton(ID_OR, psize), MakeButton(ID_LSL, psize), MakeButton(ID_LSR, psize), MakeButton(ID_D, bsize),0),
					new HGroup
						(MakeButton(ID_AND, psize),  MakeButton(ID_ROL, psize), MakeButton(ID_ROR, psize), MakeButton(ID_E, bsize),0),
					new HGroup
						(MakeButton(ID_XOR, psize),  MakeButton(ID_NOT, psize), MakeButton(ID_ASR, psize), MakeButton(ID_F, bsize),0),						
					0
				),
				"Stat",
				new VGroup   // 3 x 5 cluster of statistical functions
				(	
					nStat = new MStringView("n = 0", B_ALIGN_CENTER),  // no border due to bugs in refresh
					new HGroup
						(MakeButton(ID_SETX, lsize), MakeButton(ID_SETXY, lsize), MakeButton(ID_CS, lsize), 0),
					new HGroup
						(MakeButton(ID_AVGX, lsize), MakeButton(ID_AVGY, lsize), MakeButton(ID_SUMXY, lsize), 0),
					new HGroup
						(MakeButton(ID_SUMX, lsize),  MakeButton(ID_SUMY, lsize), MakeButton(ID_SIGX, lsize), 0),
					new HGroup
						(MakeButton(ID_SUMX2, lsize),  MakeButton(ID_SUMY2, lsize), MakeButton(ID_SIGY, lsize), 0),						
					new HGroup
						(MakeButton(ID_DEVX, lsize),  MakeButton(ID_DEVY, lsize), MakeButton(ID_N, lsize), 0),					
					0
				),
				0
			),
			0
		),
		new Space(ssize),
		
		new MBorder
		(
			M_RAISED_BORDER,3,NULL,
			new VGroup  // buttons are arranged in a 6 x 6 grid
			( 
				new HGroup(
					prefix = new MPopup("Prefix:", "Binary (0b)", "Trinary (0t)", "Octal (0o)", "Decimal (0d)", "Duodec (0u)", "Hex (0x)", 0),
					MakeButton(ID_CE, lsize),
					0),
				new HGroup(  
					MakeButton(ID_EXPON, bsize),  MakeButton(ID_INV, bsize), MakeButton(ID_PI, bsize),
					MakeButton(ID_FACT, bsize), MakeButton(ID_SQUARED, bsize),  MakeButton(ID_SQRT, bsize),
					0),	
				new HGroup(
					MakeButton(ID_RE, bsize),  MakeButton(ID_IM, bsize),   MakeButton(ID_LBRACE, bsize),
					MakeButton(ID_RBRACE, bsize),  MakeButton(ID_X3, bsize), MakeButton(ID_CBRT, bsize),
					0),
				new HGroup(
					MakeButton(ID_7, bsize),  MakeButton(ID_8, bsize), MakeButton(ID_9, bsize),
					MakeButton(ID_DIVIDE, bsize),  MakeButton(ID_POWER, bsize), MakeButton(ID_ROOT, bsize),
					0),
				new HGroup(
					MakeButton(ID_4, bsize),  MakeButton(ID_5, bsize), MakeButton(ID_6, bsize),
					MakeButton(ID_TIMES, bsize),  MakeButton(ID_MEM, bsize), MakeButton(ID_MEMPLUS, bsize),
					0),
				new HGroup(
					MakeButton(ID_1, bsize),  MakeButton(ID_2, bsize), MakeButton(ID_3, bsize),
					MakeButton(ID_MINUS, bsize),  MakeButton(ID_I, bsize), MakeButton(ID_EQUAL, bsize),
					0),
				new HGroup(
					MakeButton(ID_0, bsize),  MakeButton(ID_PM, bsize), MakeButton(ID_DP, bsize),
					MakeButton(ID_PLUS, bsize),  MakeButton(ID_ENTER, b2size),
					0),				
			0)
		),
		new Space(ssize),
			
		tab2 = new TabGroup
		(
			"Sci",		
			new VGroup  // 3 x 7 rightmost key cluster
			(
				new HGroup
					(MakeButton(ID_SIGN, lsize), MakeButton(ID_ABS, lsize), MakeButton(ID_THETA, lsize), 0),
				new HGroup
					(MakeButton(ID_EXP, lsize), MakeButton(ID_LN, lsize), MakeButton(ID_RAND, lsize), 0),
				new HGroup
					(MakeButton(ID_10POWER, lsize), MakeButton(ID_LOG, lsize), MakeButton(ID_MIN, lsize), 0),
				new HGroup
					(MakeButton(ID_LOG2, lsize),  MakeButton(ID_TORECT, lsize), MakeButton(ID_MAX, lsize), 0),
				new HGroup
					(MakeButton(ID_INT, lsize),  MakeButton(ID_FRAC, lsize), MakeButton(ID_CONJ, lsize), 0),						
				new HGroup
					(MakeButton(ID_X, lsize),  MakeButton(ID_FIB, lsize), MakeButton(ID_GCD, lsize), 0),
			0),
			"Usr1",
			new Space(),
			"Usr2",
			new Space(),
			0
		), 0
	);
		
	// Initialize the disabled buttons
	tab2->SetEnabled(false);
	
	// create menu
	MMenuBar *menu = MakeMenu();
	
	// Build up the final view with the display, equation entry, and buttons	
	BFont font(be_plain_font);
	font.SetSize(be_plain_font->Size()*1.5);
	font_height h;
	font.GetHeight(&h);
	float ch = h.leading + h.ascent + h.descent + 5;
	MScrollView *scroller;
	topview= new VGroup
			(
				menu,
				new MBorder 
				(
					M_RAISED_BORDER, 3, "outer",
					new VGroup
					(
						displayArea,
						scroller = new MScrollView(display = new MyTextView(), true, false, B_FANCY_BORDER, minimax(-1, ch, 1E4, ch)),
						buttons,
					0)
				),
			0);
		
	// restore plot menu
	plotCount = Settings::GetInt(MAXPLOTS);
	ReadPlotMenu(plots);
					
	// change the display to use a different font
	display->flags|=M_NO_FONT_CHANGES;
	display->SetViewColor(Settings::GetColor(EDITSTR));
	display->SetFontAndColor(&font);
	display->SetWordWrap(false);
					
	// because topview is an MView, we need to cast it to a BView.
	flags|=M_NO_Y_LEFTOVERS;
	AddChild(dynamic_cast<BView*>(topview));
}

void BeCalcWin::WindowActivated (bool active)
{	
	// set up border for equation entry control	
	PWindow::WindowActivated(active);	
	MScrollView *v = (MScrollView *)display->Parent();
	if (active) {
		if (display->IsFocus()) v->SetBorderHighlighted(true);
	} else
		v->SetBorderHighlighted(false);
}

void BeCalcWin::InitWindow()
{
	bool ok;
	Variables::Get(LASTRESULTN, lastResult, ok);
	if (!ok) lastResult = Complex::Ex0;
	Real::SetDigits(Complex::GetDigits());     // need to initialize this		
	angle->SetActive(Complex::GetAngle());
	radix->SetActive(Complex::GetBase()-2, false);  // no message since that clears the display
	prefix->SetActive(Settings::GetInt(PREFIX), false);
	format->SetActive(Complex::GetFPFormat(), false);
	tab->ActivateTab(Settings::GetInt(KEYTAB));
	ltab->ActivateTab(Settings::GetInt(LISTTAB));
	display->SetDoesUndo(true);
	display->SetText(Settings::GetString(LASTRESULT));
	state = (DisplayState)Settings::GetInt(DISPSTATE);
	Statistics::InitState(nStat);
	
	// open all the windows
	openwin = Settings::GetInt(OPENWINDOWS);
	int32 lopenwin = openwin;
	for (int32 i = wMortgage; i < wReserved1; i=i*2) {
		BMessage doOpen((uint32)0);
		OpenWindows select = (OpenWindows)(i & lopenwin);
		switch (select) {
			case wMortgage: 	doOpen.what = ID_MORTGAGE; break;
			case wHelp:			doOpen.what = ID_COMMANDS; break;
			case wConvert:		doOpen.what = ID_CONVERSIONS; break;
			case wAbout:		doOpen.what = ID_ABOUT; break;
			case wPref:			doOpen.what = ID_PREFERENCES; break;
			case wReserved1:
			case wReserved2:
			case wNone: break;
			default: // this is a plot window
				doOpen.AddInt32("id", floor(log(select)/log(2))-5);
				doOpen.what = ID_REOPEN_PLOT;
				break;
		}
		if (doOpen.what != 0) DispatchMessage(&doOpen, this);
	}
}

void BeCalcWin::MenusBeginning(void)
{
	// fix the edit menu states
	SetEditEnabled(display->IsFocus(), display);
}

bool BeCalcWin::QuitRequested()
{
	PWindow::QuitRequested();
	
	// save the display state
	Settings::SetInt(DISPSTATE, state);
	
	// save the open window state
	Settings::SetInt(OPENWINDOWS, openwin);
	
	// save the currently displayed string & last result
	Settings::SetString(LASTRESULT, display->Text());
	Variables::Set(LASTRESULTN, lastResult);
	
	// get current active tab
	Settings::SetInt(KEYTAB, tab->ActiveTab());
	Settings::SetInt(LISTTAB, tab->ActiveTab());
	
	// save total number of plots
	Settings::SetInt(MAXPLOTS, plotCount);
	SavePlotMenu(plots);
	
	// save statistics state
	Statistics::SaveState();	
		
	// save equation entry color
	Settings::SetColor(EDITSTR, display->ViewColor());	
		
	// dispose of the save panel
	if (save != NULL) delete save;
	
	// save and dispose the equations
	Equations::CloseEquations();
	
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

static int32 PromptUser (const char * question, const char * text, 
									const char * button1, const char * button2 = NULL, const char * button3 = NULL)
{
	int32 len = strlen(question);
	if (text != NULL) len += strlen(text);
	char *str = new char[len+4];
	sprintf(str, question, text);
	BAlert *alert = new BAlert("title", str, button1, button2, button3, 
											B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
	if (strcmp(button1, "Cancel") == 0) alert->SetShortcut(0, B_ESCAPE);
	if ((button2 != NULL) && (strcmp(button2, "Cancel") == 0)) alert->SetShortcut(1, B_ESCAPE);
	delete [] str;
	return alert->Go();
}

void BeCalcWin::SavePanel (const char *file, const char *button, int32 id)
{
	BMessage msg(B_SAVE_REQUESTED);
	msg.AddInt32("list_id", id);
	if (save == NULL) {
		save = new BFilePanel(B_SAVE_PANEL);	
		save->Window()->SetTitle("Export");	
	}
	save->SetMessage(&msg);
	save->SetSaveText(file);
	save->SetButtonLabel(B_DEFAULT_BUTTON, button);
	save->Show();
}

void BeCalcWin::Recalculate ()
{
	if (Lock()) {
		if (state == dsAppend) {
			if (!Equations::Evaluate(display->Text(), lastResult)) Scanner::Mark(Real::err);
			state = dsReturn;
		}
		
		// format into a string
		BString text;
		Complex::ComplexToStr(lastResult, text);
		display->SetText(text.String());
		
		// update variable list
		if (Vars->CountItems() != Variables::Defined()) {
			Variables::CopyVars(Vars);
		}
		Vars->Invalidate();  // refresh variables
		Unlock();	
	}
}

// Always call the inherited MessageReceived(), unless you don't
// want your windows to react to font-changes and color-drops.
void BeCalcWin::MessageReceived(BMessage *message)
{
	int32 id;
	void * ptr;
	status_t stat;
	app_info app;
	type_code mtype;
	entry_ref ref;
	BPath path;
	
	switch(message->what)
	{
		case ID_PRINT_EQUATIONS:
			Printer::Print("BeCalc Print", Equations::Equation);
			break;
		case ID_PRINT_RESULTS:
			Printer::Print("BeCalc Print", Equations::Results);
			break;
		case ID_PRINT_VARIABLES:
			Printer::Print("BeCalc Print", Vars);
			break;
		case ID_PRINT_MORTGAGE:
			Printer::Print("BeCalc Print", win->resultView);
			break;
		case ID_PRINT_ALL:	
			Printer::Print("BeCalc Print1", Equations::Equation, Equations::Results, Vars);
			break;
		case ID_PAGE_SETUP:
			Printer::Setup("BeCalc Print Settings");
			break;
		case ID_EXPORT_EQUATIONS:
			SavePanel("equations.txt", "Export Equations", message->what);
			break;
		case ID_EXPORT_RESULTS:
			SavePanel("results.txt", "Export Results", message->what);
			break;
		case ID_EXPORT_VARIABLES:
			SavePanel("variables.txt", "Export Variables", message->what);
			break;
		case ID_EXPORT_MORTGAGE:
			SavePanel("mortgage.txt", "Export Mortgage List", message->what);
			break;
		case ID_EXPORT_ALL:
			SavePanel("equ_res_var.txt", "Export All", message->what);
			break;
		case ID_CLOSE_MORTGAGE:
			// mortage window closing, reenable menu
			openwin &= ~wMortgage;
			win = NULL;
			SetMortgageEnabled(false);
			break;	
		case ID_CLOSE_CONVERSIONS:
			// conversion window closing, reenable menu
			openwin &= ~wConvert;		
			Equations::convWin = NULL;
			cwin = NULL;
			break;	
		case ID_CLOSE_ABOUT:
			// about window closing, reenable menu
			awin = NULL;
			openwin &= ~wAbout;
			break;	
		case ID_CLOSE_COMMANDS:
			// about window closing, reenable menu
			openwin &= ~wHelp;
			hwin = NULL;
			break;
		case ID_CLOSE_PREFS:
			// preference window closing, reenable menu
			openwin &= ~wPref;
			prefwin = NULL;
			break;
		case ID_CLOSE_PLOT:
			// one of the plot windows was just closed
			id = message->FindInt32("id");
			if (id < plots.CountItems()) {
				openwin &= ~(OpenWindows)(1 << (id+5));
			}
			break;	
		case ID_ABOUT:
		case B_ABOUT_REQUESTED:			
			if (awin == NULL) {
				// open the About window
				awin = new AboutWin(ABOUTNAME, B_TITLED_WINDOW);
				openwin |= wAbout;
				awin->target = this;
			} else {
				// window to front
				awin->Activate();
			}
			break;
		case ID_SHOW_GUIDE:
			// open the guide for the user			
			AboutWin::GetAppDir(path);
			path.Append("documents/BeCalc.html");		// add the default name for manual
			stat = get_ref_for_path(path.Path(), &ref);
			if (stat == B_OK) {
			  	be_roster->Launch(&ref);		// start up NetPositive with BeCalc manual
			}
			break;
		case ID_HOME:
			// go to Computer Inspirations web site
			AboutWin::GetAppDir(path);
			path.Append("Computer Inspirations");		// the book mark
			stat = get_ref_for_path(path.Path(), &ref);
			if (stat == B_OK) {
			  	be_roster->Launch(&ref);		// start up NetPositive to CI site
			}
			break;	
		case ID_REGISTER:
			// go to registration web site
			AboutWin::GetAppDir(path);
			path.Append("Buy BeCalc");		// the book mark
			stat = get_ref_for_path(path.Path(), &ref);
			if (stat == B_OK) {
			  	be_roster->Launch(&ref);		// start up NetPositive to registration site
			}
			break;	
		case ID_WEB_PAGE:
			// go to registration web site
			AboutWin::GetAppDir(path);
			path.Append("BeCalc Online");		// the book mark
			stat = get_ref_for_path(path.Path(), &ref);
			if (stat == B_OK) {
			  	be_roster->Launch(&ref);		// start up NetPositive to registration site
			}
			break;						
		case ID_COMMANDS:
			// open the Help window
			if (hwin == NULL) {
				hwin = new HelpWin(HELPNAME, B_TITLED_WINDOW);
				openwin |= wHelp;	
				hwin->target = this;
			} else {
				hwin->Activate();
			}	
			break;	
		case ID_PREFERENCES:
			// open the Preference window
			if (prefwin == NULL) {
				prefwin = new PrefWin(PREFNAME, B_TITLED_WINDOW);
				openwin |= wPref;	
				prefwin->target = this;
			} else {
				prefwin->Activate();
			}	
			break;
		case ID_REMOVE_PLOTS:
			if (PromptUser("Are you sure you want to remove all predefined plots?", "", "Yes", "No") == 0) {
				BMenuItem *item;
				while ((item = (BMenuItem *)plots.RemoveItem((int32)0)) != NULL) RemovePlotItem(item);
				plotCount = 0; Settings::SetInt(MAXPLOTS, plotCount);
			}
			break;
		case ID_REOPEN_PLOT:
			// reopen a previous plot window
			id = message->FindInt32("id");
			if (id < plots.CountItems()) {
				BMenuItem *ptr = (BMenuItem *)plots.ItemAt(id);
				char title[64];
				if (ptr != NULL) {
					openwin |= (1 << (id+5));	
					BString s(ptr->Label());
					s.RemoveLast("'");
					s.Remove(0, s.FindFirst("'")+1);
					sprintf(title, "BeCalc Plot #%ld", id+1);
					
					// see if it's already open
					int32 wcount = be_app->CountWindows()-1;
					BWindow *win = NULL;
					while ((wcount > 0) && ((win = be_app->WindowAt(wcount)) != NULL) &&
							(strcmp(win->Title(), title) != 0)) wcount--;
					if ((win == NULL) || ((win != NULL) && (strcmp(win->Title(), title) != 0))) {
						// open a new window
						PlotWin::InitSettings(title);
						pwin = new PlotWin(title, s.String(), id, B_TITLED_WINDOW);
						pwin->target = this;
					} else  {
						win->Activate();
					}
				}
			}
			break;
		case ID_CHANGE_PLOT_MENU:
			id = message->FindInt32("id");
			if (id < plots.CountItems()) {
				BMenuItem *ptr = (BMenuItem *)plots.ItemAt(id);
				const char *str = message->FindString("eq");
				if ((ptr != NULL) && (str != NULL)) {
					BString s(ptr->Label());
					int32 start = s.FindFirst("'");
					int32 end = s.FindLast("'");
					s.Remove(start+1, end-start);
					s.Append(str); s.Append("'");
					ptr->SetLabel(s.String());
				}
			}			
			break;				
		case ID_PLOT: {
			// open plot window
			char title[64];
			sprintf(title, "BeCalc Plot #%d", plotCount+1);
			PlotWin::InitSettings(title);
			pwin = new PlotWin(title, display->Text(), plotCount, B_TITLED_WINDOW);
			if (pwin != NULL) {
				openwin |= (1 << (plotCount+5));	
				BString s(title);
				s.RemoveFirst("BeCalc ");
				s.Append(" of '"); s.Append(display->Text()); s.Append("'");
				plots.AddItem(AddPlotItem(s.String(), plotCount));
				plotCount++;
				Settings::SetInt("BeCalc-maxplots", plotCount);
				pwin->target = this;
			}
			} break;							
		case ID_MORTGAGE:
			// open mortgage window
			if (win == NULL) {
				win = new MortgageWin(MORTGAGENAME, B_TITLED_WINDOW);
				openwin |= wMortgage;
				SetMortgageEnabled(true);	
				win->target = this;
			} else {
				win->Activate();
			}
			break;
		case ID_CONVERSIONS:
			// open conversions window
			if (cwin == NULL) {
				cwin = new ConvertWin(CONVERTTITLE, B_TITLED_WINDOW);
				openwin |= wConvert;	
				cwin->target = this;
				Equations::convWin = cwin;  // important for conversion callback
			} else {
				cwin->Activate();
			}
			break;
		case ID_DELETE_SEL: {
				// delete the selected equation, result, variable
				int32 tab = ltab->ActiveTab();
				MListView *list;
				MyItem *item;
				BListItem *item2;
				int32 selected;
				char * str = NULL;
				
				if (tab == 0) {list = Equations::Equation; str = "result";}
				else if (tab == 1) {list = Equations::Results; str = "equation";}
				else list = Vars;
				selected = list->CurrentSelection();
				if (selected == -1) {PromptUser("Nothing was selected!", NULL, "Ok"); break;}
				item = dynamic_cast<MyItem *> (list->ItemAt(selected));
				if (PromptUser("Are you sure you want to delete %s?", item->DragStr(), "Yes", "No") == 0) {
					item2 = list->RemoveItem(selected); delete item2;
				 	if (tab < 2)  {   // also remove corresponding result/equation
				 		if ((Equations::IsDeleteBoth()) || (PromptUser("Also delete associated %s?", str, "Yes", "No") == 0)) {
							if (list == Equations::Equation) item2 = Equations::Results->RemoveItem(selected);
							else item2 = Equations::Equation->RemoveItem(selected);
							delete item2;
				 		}
				 	} else Variables::Delete(selected);
				}
			}
			break;
		case ID_DELETE_EQUATIONS: {
				BListItem *item;
				if  (PromptUser("Are you sure you want to delete ALL the equations?", NULL, "Yes", "No") == 0) {
					while ((item = Equations::Equation->RemoveItem((int32)0)) != NULL) delete item;
				  	if  (PromptUser("Also delete ALL the results?", NULL, "Yes", "No") == 0) {
				  		while ((item = Equations::Results->RemoveItem((int32)0)) != NULL) delete item;
				  	}
				}
			}
			break;
		case ID_DELETE_RESULTS: {
				BListItem *item;
				if  (PromptUser("Are you sure you want to delete ALL the results?", NULL, "Yes", "No") == 0) {
					while ((item = Equations::Results->RemoveItem((int32)0)) != NULL) delete item;
				  	if  (PromptUser("Also delete ALL the equations?", NULL, "Yes", "No") == 0) {
				  		while ((item = Equations::Equation->RemoveItem((int32)0)) != NULL) delete item;
				  	}
				}
			}
			break;
		case ID_DELETE_VARIABLES: {
				BListItem *item;
				if  (PromptUser("Are you sure you want to delete ALL the variables?", NULL, "Yes", "No") == 0) {
					Variables::DeleteAll();  // need to do this first so we can check for hidden items
					while ((item = Vars->RemoveItem((int32)0)) != NULL) {
						delete item;
					}
				}
			}
			break;
		case B_CUT:
		case B_COPY:	
		case B_UNDO:			
			state = dsAppend;
			DispatchMessage(message, display);
			break;
		case B_PASTE:
			state = dsAppend;
			if (message->GetInfo("RGBColor", &mtype) == B_OK) 
				MWindow::MessageReceived(message);
			else 
				display->Paste(be_clipboard); 
			break;
		case ID_UPDATE_VARS:
			// sent when sorting preferences changed
			Variables::CopyVars(Vars); 
			break;
		case ID_UPDATE_NUMS: 
			// sent when number format has changed
			Recalculate();
			Vars->Invalidate();
			break;
		case M_BUTTON_SELECTED:
			id = message->FindInt32("butID");
			switch (id)
			{
				case ID_CE:
					display->MakeFocus(true);
					display->SelectAll();
					display->Cut(be_clipboard);
					lastResult = Complex::Ex0;
					state = dsClear;
					break;
				case CID_CONVERSIONS: 
				case CID_INVERSE:
					if (Lock()) {
						BTextView *v = display;
						if (id == CID_INVERSE) v->Insert(0, "convi(", 6);						
						else v->Insert(0, "conv(", 5);
						v->Insert(v->TextLength(), ")", 1);
						Unlock();
						state = dsAppend;
					}
				case ID_ENTER:
					// compute the equation result
					Recalculate();
					break;
				case ID_AVGX:
				case ID_AVGY:
				case ID_SUMXY:
				case ID_SUMX:
				case ID_SUMY:
				case ID_SIGX:
				case ID_SIGY:
				case ID_SUMX2:
				case ID_SUMY2:
				case ID_DEVX:
				case ID_DEVY:
				case ID_N:
					Statistics::Display((button_ids)id, display);
					break;
				case ID_SETX:
					Statistics::StoreToX(display, nStat);
					break;
				case ID_SETXY:
					Statistics::StoreToXY(display, nStat);
					break;
				case ID_CS:
					Statistics::Clear(nStat);
					break;
				case ID_PM:
					display->Insert(0, "-(", 2); display->Insert(strlen(display->Text()), ")", 1);
					break;
				default:
					display->MakeFocus(true);
					if (state == dsClear) {
						if (IsNumber(id) || IsFunction(id)) {
							// clear display
							display->SelectAll();
							display->Cut(be_clipboard);							
						} else {
							// preceed operator with a zero
							display->Insert(0, "0", 1);
						}	
					} else if (state == dsReturn) {
						if (IsNumber(id)) {
							// clear the display
							display->SelectAll();
							display->Cut(be_clipboard);	
						} else if (IsFunction(id)) {
							// insert the function
							display->Select(0, 0);
						} else {
							// append operator
							int32 len = display->TextLength();
							display->Select(len, len);
						}
					}
					display->Insert(ButtonIs(id));
					if ((state == dsReturn) && (IsFunction(id))) 
						display->Select(display->TextLength(), display->TextLength());
					display->ScrollToOffset(strlen(display->Text()));
					state = dsAppend;			
					break;
			}
			break;
		case M_POPUP_SELECTED:
			id = message->FindInt32("index");
			message->FindPointer("popup", &ptr);
			if (ptr == radix) {
				BString text;
				
				// numerical base
				Recalculate();	
				Complex::SetBase(id+2);		
				Vars->Invalidate();  // refresh variables
				Complex::ComplexToStr(lastResult, text);
				display->SetText(text.String());	
			} else if (ptr == angle) {
				// angular measure
				Complex::SetAngle((msAngle)id);  		
			} else if (ptr == format) {
				// number format
				Complex::SetFPFormat((fpFormat)id);
				Recalculate(); 	
			} else if (ptr == prefix) {
				// insert prefix
				char *str;
				int32 base;
				Settings::SetInt(PREFIX, id);
				switch (id) {
					case 0: str = "0b"; base = 2; break;
					case 1: str = "0t"; base = 3; break;
					case 2: str = "0o"; base = 8; break;
					case 3: str = "0d"; base = 12; break;
					default: str = "0x"; base = 16;
				}
				display->Insert(str);
				display->ScrollToOffset(strlen(display->Text()));				
			}
			break;
		case B_KEY_DOWN:
			display->MakeFocus(true);
			DispatchMessage(message, display);
			state = dsAppend;
			break;
		default:
			//printf("received by window:\n");
			//message->PrintToStream();
			//printf("----------\n");
			MWindow::MessageReceived(message);
			break;
	}
}
