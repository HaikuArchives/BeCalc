
/*
     Plot window for BeCalc.
*/

#include <stdio.h>
#include <ctype.h>
#include <InterfaceKit.h>
#include <Application.h>
#include <TranslationKit.h>
#include <StorageKit.h>

#include "Errors.h"
#include "Plots.h"
#include "ExComplex.h"
#include "Equations.h"
#include "Variables.h"
#include "Scanner.h"
#include "Buttons.h"
#include "Colors.h"

// preferences names
static const char *PLOTIS 	= "-plotkind";
static const char *XDEC		= "-xdec";
static const char *YDEC		= "-ydec";
static const char *XMIN		= "-xmin";
static const char *XMAX	= "-xmax";
static const char *YMIN		= "-ymin";
static const char *YMAX	= "-ymax";
static const char *XDIVS	= "-xdivs";
static const char *XMINOR	= "-xminor";
static const char *YDIVS	= "-ydivs";
static const char *YMINOR	= "-yminor";
//static const char *VARS		= "-vars";

// internally used plot model

double PlotModel::Eval(double x) const
{
	Complex res(x, 0.0);
	Variables::Set(var.String(), res);
	if (Equations::Evaluate(Equation(), res, false)) return Real::Short(res.RealPart());
	else return 0;
}


static bool RealToString(double num, char *str, short minchars, short fracchars, char pad);

// The MWindow receives font-change messages from the application.
// It also takes care of color-drops. 
PlotWin::PlotWin(const char *name, const char *eqn, int32 id, window_type type)
	: PWindow(name, type), pname(name), plotID(id)
{
	MView *topview;
	char str[1024];
		
	// just because MTextControl is defined without const
	strcpy(str, eqn);
    			               			           
    // define the controls
	VGroup *plotArea =
		new VGroup(
			plotBorder = new MBorder(M_DEPRESSED_BORDER, 1, NULL, 
				plot = new mPlot (eqn, "x")
			),
			new MBorder(M_LABELED_BORDER, 5, "Plot Type",
				new HGroup (
					plotKind = new MPopup("Type:", "Linear", "Log X", "Log Y", "Full Log", 0), 
			 		new HGroup(minimax(-1, -1, 10000, 10000, 3), 
			 							equation = new MTextControl("Equation:", str),
			 							0),
			 	0)
			 ),
		0);
		
	// set equation modification message
	equation->SetMessage(new BMessage(PID_EQUATION_CHANGE));
			 
	// define x-options
	VGroup *xOptions =
		new VGroup
		(
			new MBorder(M_LABELED_BORDER, 8, "Bounds",
				new HGroup(
					xmin = new SpinButton("Min:", SPIN_FLOAT),
					xmax = new SpinButton("Max:", SPIN_FLOAT),
				0)
			),
			new MBorder(M_LABELED_BORDER, 8, "Divisions",
				new HGroup(			
					xdivs = new SpinButton("Major:", SPIN_INTEGER),
					xmdivs = new SpinButton("Minor:", SPIN_INTEGER),
				0)
			),
			new MBorder(M_NO_BORDER, 8, NULL,
				new HGroup(
					vars = new MPopup("Variables:", 0),
					xdec = new SpinButton("Dec:", SPIN_INTEGER),
				0)
			),
		0);
		
	// set up limits
	xmin->SetMaximum(1E20); xmin->SetMinimum(-1E20);
	xmax->SetMaximum(1E20); xmax->SetMinimum(-1E20);
	xdivs->SetMaximum(10); xdivs->SetMinimum(1);
	xmdivs->SetMaximum(10); xmdivs->SetMinimum(0);
	xdec->SetMaximum(4); xdec->SetMinimum(0);
		
	// define y-options
	VGroup *yOptions =
		new VGroup
		(	
			new MBorder(M_LABELED_BORDER, 8, "Bounds",
				new HGroup(
					ymin = new SpinButton("Min:", SPIN_FLOAT),
					ymax = new SpinButton("Max:", SPIN_FLOAT),
				0)
			),
			new MBorder(M_LABELED_BORDER, 8, "Divisions",
				new HGroup(			
					ydivs = new SpinButton("Major:", SPIN_INTEGER),
					ymdivs = new SpinButton("Minor:", SPIN_INTEGER),
				0)
			),
			new MBorder(M_NO_BORDER, 8, NULL,
				new HGroup(
					new MButton("New Min/Max", PID_NEW_YMIN_MAX), 
					ydec = new SpinButton("Dec:", SPIN_INTEGER),
				0)
			),
		0);
		
	// set up limits
	ymin->SetMaximum(1E20); ymin->SetMinimum(-1E20);
	ymax->SetMaximum(1E20); ymax->SetMinimum(-1E20);
	ydivs->SetMaximum(10); ydivs->SetMinimum(1);
	ymdivs->SetMaximum(10); ymdivs->SetMinimum(0);
	ydec->SetMaximum(4); ydec->SetMinimum(0);
			
	TabGroup *bottom = new TabGroup ("X Options", xOptions, "Y Options", yOptions, 0);
					 
	// equalize the spacing
	DivideSame(xmin, xmax, ymin, ymax, xdivs, ydivs, xmdivs, ymdivs, 0);
         		
	// Build up the final view with the display, equation entry, and buttons
	topview= new MBorder
				(
					M_RAISED_BORDER, 3, "outer",
					new VGroup
					(
						plotArea,
						bottom,
					0)
				);

	// because topview is an MView, we need to cast it to a BView.
	AddChild(dynamic_cast<BView*>(topview));
	Show();
}

double PlotWin::SetFloatValue (SpinButton *spin, const char *postfix)
{
	double num;
	BString name(pname); name.Append(postfix);
	spin->SetValue(num = Settings::GetFloat(name.String()));
	return num;
}

int32 PlotWin::SetValue (SpinButton *spin, const char *postfix)
{
	int32 num;
	BString name(pname); name.Append(postfix);
	spin->SetValue(num = Settings::GetInt(name.String()));
	return num;
}

void PlotWin::InitSettings(const char *name)
{
	// initialize default settings for this plot
	if (!Settings::GetBool(name)) {
		Settings::SetPosition(name, BRect(100, 100, 300, 300));
		BString xname(name); xname.Append(XMAX);
		Settings::SetFloat(xname.String(), 10);
		BString yname(name); yname.Append(YMAX);
		Settings::SetFloat(yname.String(), 10);
		BString xdname(name); xdname.Append(XDIVS);
		Settings::SetInt(xdname.String(), 1);
		BString ydname(name); ydname.Append(YDIVS);
		Settings::SetInt(ydname.String(), 1);
		BString xmname(name); xmname.Append(XMINOR);
		Settings::SetInt(xmname.String(), 1);
		BString ymname(name); ymname.Append(YMINOR);
		Settings::SetInt(ymname.String(), 1);
		Settings::SetBool(name, true);
	}
}

void PlotWin::InitWindow()
{
	BString name(pname); name.Append(PLOTIS);
	plotKind->SetActive(Settings::GetInt(name.String()), false);
	plot->SetPlotKind((PlotKindType) Settings::GetInt(name.String()));
	plot->SetDecimals(SetValue(xdec, XDEC),  SetValue(ydec, YDEC));
	plot->SetDivisions(SetValue(xdivs, XDIVS), SetValue(ydivs, YDIVS), 
							   SetValue(xmdivs, XMINOR), SetValue(ymdivs, YMINOR));
	plot->SetPlotLimits(SetFloatValue(xmin, XMIN), SetFloatValue(xmax, XMAX),
								SetFloatValue(ymin, YMIN), SetFloatValue(ymax, YMAX));	
	plot->initialized = true;
}

void PlotWin::SaveValue (SpinButton *spin, const char *postfix)
{
	BString name(pname); name.Append(postfix);
	Settings::SetInt(name.String(), (int32)spin->Value());
}

void PlotWin::SaveFloatValue (SpinButton *spin, const char *postfix)
{
	BString name(pname); name.Append(postfix);
	Settings::SetFloat(name.String(), spin->Value());
}

bool PlotWin::QuitRequested()
{		
	// save the current defaults
	PWindow::QuitRequested();
	
	// save the current settings
	BString name(pname); name.Append(PLOTIS);
	Settings::SetInt(name.String(), plot->PlotIs);
	SaveValue(xdec, XDEC);
	SaveValue(ydec, YDEC);
	SaveValue(xdivs, XDIVS);
	SaveValue(xmdivs, XMINOR);
	SaveValue(ydivs, YDIVS);
	SaveValue(ymdivs, YMINOR);
	SaveFloatValue(xmin, XMIN);
	SaveFloatValue(xmax, XMAX);
	SaveFloatValue(ymin, YMIN);
	SaveFloatValue(ymax, YMAX);
				
	// unghost the appropriate plot window menu item
	if (target->Lock()) {
		BMessage* msg = new BMessage(ID_CLOSE_PLOT);
		msg->AddInt32("id", plotID);
    	DispatchMessage(msg, target);
    	target->Unlock();
	}
	
	// shut down this window
	Quit();  	 // call the parent
	return true;
}

// used to drag the plot 
void mPlot::MouseDown(BPoint where)
{
	BRect r(Bounds());
	if (r.Contains(where)) {
		//	wait for a drag to start
		while (true) {
			BPoint w2;
			uint32 mods;
			GetMouse(&w2, &mods);
			if (!mods) {
				//	releasing the buttons without moving means no drag
				goto punt;
			}
			if (w2 != where) {
				//	moving the mouse starts a drag
				break;
			}
			snooze(40000);
		}
		BMessage msg(B_SIMPLE_DATA);
		
		//	add the types
		msg.AddString("be:types", B_FILE_MIME_TYPE);
		msg.AddString("be:types", m_the_type);
		msg.AddString("be:filetypes", m_the_type);
		
		//	add the actions
		msg.AddInt32("be:actions", B_COPY_TARGET);
		msg.AddInt32("be:actions", B_TRASH_TARGET);
		msg.AddString("be:clip_name", "BeCalc Plot");
		
		//	start the drag (the rest will be handled by MessageReceived())
		DragMessage(&msg, r, Window());
		return;
	}
punt:
	Window()->Activate(true);
}

void PlotWin::ActionCopy(BMessage * request) {
	//	someone accepted our drag and requested one of the two
	//	types of data we can provide (in-message or in-file bitmap)
	const char * type = NULL;
	if (!request->FindString("be:types", &type)) {
		// detach the plot view from the plot window and attach to a bitmap
		BRect r = plotBorder->Frame();
		if (!plotBorder->RemoveChild(plot)) return;
		BBitmap *m_bitmap = new BBitmap(r, B_RGB32, true);
		m_bitmap->Lock();
		m_bitmap->AddChild(plot);
		plot->Draw(r);
		plot->Sync();
		m_bitmap->Unlock();
		
		// write to a file or return a bitmap in the message
		BBitmapStream strm(m_bitmap);
		if (!strcasecmp(type, B_FILE_MIME_TYPE)) {
			const char * name;
			entry_ref dir;
			if (!request->FindString("be:filetypes", &type) &&
					!request->FindString("name", &name) &&
					!request->FindRef("directory", &dir)) {
				//	write file
				BDirectory d(&dir);
				BFile f(&d, name, O_RDWR | O_TRUNC);
				BTranslatorRoster::Default()->Translate(plot->m_translator,
						&strm, NULL, &f, plot->m_type_code);
				BNodeInfo ni(&f);
				ni.SetType(plot->m_the_type);
			}
		}
		else {
			//	put in message
			BMessage msg(B_MIME_DATA);
			BMallocIO f;
			BTranslatorRoster::Default()->Translate(plot->m_translator,
					&strm, NULL, &f, plot->m_type_code);
			msg.AddData(plot->m_the_type, B_MIME_TYPE, f.Buffer(), f.BufferLength());
			request->SendReply(&msg);
		}
		strm.DetachBitmap(&m_bitmap);
		
		// put the view back in the window
		m_bitmap->RemoveChild(plot);
		plotBorder->AddChild(plot);
		delete m_bitmap;
	}
}


// Always call the inherited MessageReceived(), unless you don't
// want your windows to react to font-changes and color-drops.
void PlotWin::MessageReceived(BMessage *message)
{
	int32 id;
	void *ptr;
	const char *eqn;
	double xMin, xMax, yMin, yMax;
	Complex res;
	
	switch(message->what)
	{
		case M_BUTTON_SELECTED:
			id = message->FindInt32("butID");
			if (id == PID_NEW_YMIN_MAX) {
				plot->NewYLimits();  // calculate new y min/max limits
				ymin->SetValue(plot->Miny);
				ymax->SetValue(plot->Maxy);
			} else MWindow::MessageReceived(message);			
			break;
		case B_COPY_TARGET:
			ActionCopy(message);
			break;
		case PID_EQUATION_CHANGE:
			eqn = equation->Text();
			if (Equations::Evaluate(eqn, res, false)) {
				plot->SetModel(eqn, "x");
				if (target->Lock()) {
					BMessage* msg = new BMessage(ID_CHANGE_PLOT_MENU);
					msg->AddInt32("id", plotID);
					msg->AddString("eq", eqn);
			    	DispatchMessage(msg, target);
			    	target->Unlock();
				}
				plot->SetPlotKind(plot->PlotIs);  // refresh graph
			}
			break;
		case M_SPIN_TICK:
			plot->SetDecimals(xdec->Value(), ydec->Value());
			plot->SetDivisions(xdivs->Value(), ydivs->Value(), xmdivs->Value(), ymdivs->Value());
			plot->SetPlotLimits(xmin->Value(), xmax->Value(), ymin->Value(), ymax->Value());
			break;
		case M_POPUP_SELECTED:
			id = message->FindInt32("index");
			message->FindPointer("popup", &ptr);
			if (ptr == plotKind) {
				// extract limits in case user didn't press return
				xMin = xmin->Value(); xMax = xmax->Value(); 
				yMin = ymin->Value(); yMax = ymax->Value();
				plot->SetPlotLimits(xMin, xMax, yMin, yMax);
				PlotKindType kind = (PlotKindType)id;
				if (kind & LogX) {
					if ((plot->Minx <= 0) || (plot->Maxx <= 0)) {
						Error("X range must be >= 0!");
						kind = Linear;
						plotKind->SetActive(0);
					}
				}
				if (kind & LogY) {
					if ((plot->Miny <= 0) || (plot->Maxy <= 0)) {
						Error("Y range must be >= 0!");						
						kind = Linear;
						plotKind->SetActive(0);
					}			
				}				
				plot->SetPlotKind(kind);
			} else if (ptr == vars) Error("Not implemented yet!");
			else MWindow::MessageReceived(message);
			break;
		default:
			//printf("received by window:\n");
			//message->PrintToStream();
			//printf("----------\n");
			MWindow::MessageReceived(message);
			break;
	}
}



const float ZERO = 0;
const float TEN = 10; 

#define ABS(x) (x < 0 ? -x : x)
#define PutCh(c) {buffer[pos++] = c;}
#define PutNumDig(c) {buffer[pos++] = c + '0';}
#define PutExp(exp) { \
	PutCh('E'); \
	if(exp < 0) { PutCh('-'); exp = -exp; } \
	if(exp > 10) { PutNumDig((int)((exp % 100) / 10)); } \
	PutNumDig((int)(exp % 10)); }

static bool RealToString(double num, char *str, short minchars, short fracchars, char pad)
{
	const int RAcc = 15;
	short D, s, exp;
	long digit;
	char buffer[80];
	short pos, i, j;
	bool useExp;
	
	pos = 0;
	
	D = ABS(fracchars);
	useExp = fracchars < 0;
	
	if (ABS(num) < 1.0e-5) {
		exp = 0;
		num = ZERO;
	} else {	
		if (num < ZERO) {
			PutCh('-')
			num = -num;
		}
		if (D > RAcc) D = RAcc;
		num = num + ::pow(TEN, -D) * 0.5;
		
		exp = (short)::log10(num);
		num = num / ::pow(TEN, exp);
		
		if (num >= TEN) {
			num /= TEN;
			exp++;
		}
		
		if (num < 1.0) {
			num *= TEN;
			exp--;
		}
		
		if(ABS(exp) > RAcc)
			useExp = true;
	}
	
	if (useExp) {
		PutNumDig((long)(floor(num)));
		PutCh('.')
		
		j = D;
		while (j > 0) {
			num = (num - (long)(num)) * TEN;
			PutNumDig((long)(floor(num)));
			j--;
		}
		
		PutExp(exp);
	} else {
		s = exp;
		if (exp < 0) s = 0;
		
		i = 0;
		for (j = s ; j >= -D ; j--) {
			if ((i > RAcc) || (exp < 0))
				PutCh('0')
			else {
				digit = (long)(num);
				PutNumDig(digit);
				num = (num - digit) * TEN;
			}
			i++;
			exp++;
			if (j == 0) PutCh('.')
		}
	}
	
	PutCh('\0')
			
	i = 0;
	while(i + pos <= minchars)
		str[i++] = pad;
		
	j = -1;
	
	do {
		str[i++] = buffer[++j];
	} while (buffer[j] != '\0');
	
	if ((j > 0) && (str[j - 1] == '.'))
		str[j - 1] = '\0';
		
	return true;
}

static double LogOf(double x)
{
	if(x <= 0.0)
		return 0.0;
	return ::log10(x);
}

void mPlot::xyToCoords(double x, double y, BPoint &p)
{
	p.x = (float)((x - Minx) * (GMxx - mPlot::x) / (Maxx - Minx) + mPlot::x);
	p.y = (float)(GMxy - (y - Miny) * (GMxy - mPlot::y) / (Maxy - Miny));
}

void mPlot::CoordsToxy(BPoint p, double& x, double& y)
{
	x = (p.x - mPlot::x) * (Maxx - Minx) / (GMxx - mPlot::x) + Minx;
	y = (GMxy - p.y) * (Maxy - Miny) / (GMxy - mPlot::y) + Miny;
}

void mPlot::OffsetPoint(BPoint &p)
{
	p.x += xoff;
	p.y += yoff;
}

void mPlot::IntPlotLine(rgb_color Colour, double x1, double y1, double x2, double y2)
{
	BPoint p1, p2;
	
	xyToCoords(x1, y1, p1); xyToCoords(x2, y2, p2);
	OffsetPoint(p1); OffsetPoint(p2);
	SetHighColor(Colour);
	StrokeLine(p1, p2);
}

void mPlot::PlotBar(rgb_color Colour, double width, double x, double y)
{
	double Half = width / 2.0;
	BPoint p1, p2;
	
	if (y >= 0.0)
	{
		xyToCoords(x - Half, 0.0, p1);
		xyToCoords(x + Half, y, p2);
	}
	else
	{
		xyToCoords(x - Half, y, p1);
		xyToCoords(x + Half, 0.0, p2);
	}
	
	if (p1.y != p2.y)
	{
		OffsetPoint(p1); OffsetPoint(p2);
		SetHighColor(Colour);
		FillRect(BRect(p1, p2));
	}
}

void mPlot::LabelX(char *Label, BPoint p)
{
	SetHighColor(TextColour);
	MovePenTo(p);
	DrawString(Label);
}

void mPlot::LabelY(char *Label, BPoint p)
{
	long ChCnt;
	char Str[2];
	font_height h;
	float spaceHeight;
	
	GetFontHeight(&h);
	spaceHeight = h.ascent + h.leading + h.descent;
	Str[1] = '\0';
	for(ChCnt = 1 ; ChCnt <= (long)strlen(Label) ; ChCnt++)
	{
		Str[0] = Label[ChCnt - 1];
		LabelX(Str, p);
		p.y += spaceHeight;
	}
}

void mPlot::PlotXLabel(short intg, double x, double y)
{
	BPoint p;
	double xlog;
	char s[40];
	font_height h;
	float spaceHeight;
	
	GetFontHeight(&h);
	spaceHeight = h.ascent + h.leading + h.descent;	
	
	if (PlotIs & LogX) xlog = ::pow(TEN, x);
	else xlog = x;
	
	if (GetLabel != NULL)
		GetLabel(intg, s);
	else if (!RealToString(xlog, s, 2, Decx, ' '))
		strcpy(s, "***");
	
	xyToCoords(x, y, p);
	p.x -= StringWidth(s) / 2;
	p.y += spaceHeight;
	LabelX(s, p);
}

void mPlot::PlotYLabel(double x, double y)
{
	BPoint p;
	double ylog;
	char s[40];
	font_height h;
	float spaceHeight;
	
	GetFontHeight(&h);
	spaceHeight = h.ascent + h.leading;	
	
	if(PlotIs & LogY) ylog = ::pow(10, y);
	else ylog = y;
	
	if(!RealToString(ylog, s, 2, Decy, ' '))
		strcpy(s, "***");
		
	xyToCoords(x, y, p);
	p.x -= StringWidth(s) + 2;
	p.y += spaceHeight*3/8; 
	LabelX(s, p);
}

void mPlot::ClearPlot()
{
	BRect area = Bounds();
	rgb_color BackColour = White;
		
	SetHighColor(BackColour);
	FillRect(area);
	GridDrawn = false;
}

void mPlot::SetPlotOffset(long xoff, long yoff)
{
	mPlot::xoff = xoff;
	mPlot::yoff = yoff;
}

void mPlot::PlotMinorX(double x, double xint)
{
	short Cnt;
	double xm, xi;
	
	if (SubDivx > 1) {
		if (PlotIs & LogX) {
			xi = ::pow(TEN, xint) / (SubDivx + 1);
			for (Cnt = 2; Cnt <= SubDivx; Cnt++) {
				xm = x + LogOf(Cnt * xi);
				IntPlotLine(MinorColour, xm, Miny, xm, Maxy);
			}
		} else {
			xi = xint / SubDivx;
			for (Cnt = 1 ; Cnt <= SubDivx - 1 ; Cnt++) {
				xm = x + Cnt * xi;
				IntPlotLine(MinorColour, xm, Miny, xm, Maxy);
			}
		}
	}
}

void mPlot::PlotMinorY(double y, double yint)
{
	short Cnt;
	double ym, yi;
	
	if (SubDivy > 1) {
		if (PlotIs & LogY) {
			yi = ::pow(TEN, yint) / (SubDivy + 1);
			for (Cnt = 2; Cnt <= SubDivy; Cnt++) {
				ym = y + LogOf(Cnt * yi);
				IntPlotLine(MinorColour, Minx, ym, Maxx, ym);
			}
		} else {
			yi = yint / SubDivy;
			for (Cnt = 1; Cnt <= SubDivy - 1 ; Cnt++) {
				ym = y + Cnt * yi;
				IntPlotLine(MinorColour, Minx, ym, Maxx, ym);
			}
		}
	}
}

void mPlot::DrawGrid()
{
	short vCnt, hCnt;
	float oldx, oldy, oldxoff, oldyoff, oldGMxx, oldGMxy;
	double x, y, xint, yint;
	bool ScatterState;

	ScatterState = ScatterPlot;
	ScatterPlot = false;
	ClearPlot();
	oldxoff = xoff; oldyoff = yoff;
	oldx = mPlot::x; oldy = mPlot::y;
	oldGMxx = GMxx; oldGMxy = GMxy;
	mPlot::x -= 1; mPlot::y -= 1; GMxx += 1; GMxy += 1;
	SetPlotOffset(0, 0);
	
	// set up intervals
	xint = (Maxx - Minx) / Divx;
	yint = (Maxy - Miny) / Divy;
	
	// draw all the minor grid lines first		
	for (vCnt = 0; vCnt < Divx; vCnt++) {
		PlotMinorX(Minx + vCnt * xint, xint);
	}		
	for (hCnt = 0 ; hCnt < Divy ; hCnt++) {
		PlotMinorY(Miny + hCnt * yint, yint);
	}
	
	// draw all the major grid lines & labels
	for (vCnt = 0; vCnt <= Divx; vCnt++) {
		x = Minx + vCnt * xint;
		IntPlotLine(GridColour, x, Miny, x, Maxy);
		PlotXLabel(vCnt, x, Miny);
	}
	for (hCnt = 0; hCnt <= Divy; hCnt++) {
		y = Miny + hCnt * yint;
		IntPlotLine(GridColour, Minx, y, Maxx, y);  
		PlotYLabel(Minx, y);
	}		
	
	SetPlotOffset(oldxoff, oldyoff);
	GMxx = oldGMxx; GMxy = oldGMxy;
	mPlot::x = oldx; mPlot::y = oldy;
	GridDrawn = true;
	ScatterPlot = ScatterState;
}

mPlot* mPlot::Empty()
{
	return NULL;
}

void mPlot::SetPlotLimits(double xMin, double xMax, double yMin, double yMax)
{
	Miny = yMin;
	Maxy = yMax;
	Minx = xMin;
	Maxx = xMax;
	if (Minx >= Maxx) Minx= Maxx - 0.5;
	if (Miny >= Maxy) Miny = Maxy - 0.5;
	if (initialized) layout(Bounds());
}

void mPlot::SetPlotScale(double xscale, double yscale)
{
	mPlot::xscale = ABS(xscale);
	mPlot::yscale = ABS(yscale);
	
	if (mPlot::xscale < 0.1) mPlot::xscale = 0.1;
	if (mPlot::yscale < 0.1) mPlot::yscale = 0.1;
	ClearPlotHistory();  // force regeneration of all points
	Invalidate();  // force redraw
}

void mPlot::SetScatterPlot()
{
	ScatterPlot = true;
	ScatterChar[0] = '0';
}

void mPlot::SetScatterChar(char Ch)
{
	ScatterChar[0] = Ch;
}

void mPlot::ClearPlotHistory()
{
	if (Points != NULL) delete [] Points;
	Points = NULL;
}

mPlot::mPlot(const char *str, const char *v) 
	: BView(BRect(100,100,200,200), "", B_WILL_DRAW, 0), model(str, v)
{	
	ct_mpm=minimax(100, 100);
	width = 100; height = 100;
	x = 0; y = 0; illegal = false;
	
	xscale = 1.0;
	yscale = 1.0;
	
	//	Find the translator to use for us
	translator_id * all_translators = NULL;
	int32 count = 0;
	status_t err = BTranslatorRoster::Default()->GetAllTranslators(&all_translators, &count);
	if (err >= B_OK) {
		err = B_ERROR;
		//	look through all available translators
		for (int ix=0; ix<count; ix++) {
			const translation_format * in_formats;
			int32 fmt_count;
			if (B_OK <= BTranslatorRoster::Default()->GetInputFormats(all_translators[ix],
					&in_formats, &fmt_count)) {
				//	look for translators that accept BBitmaps as input
				for (int iy=0; iy<fmt_count; iy++) {
					if (in_formats[iy].type == B_TRANSLATOR_BITMAP) {
						goto do_this_format;
					}
				}
			}
			continue;
do_this_format:
			const translation_format * out_formats;
			if (B_OK <= BTranslatorRoster::Default()->GetOutputFormats(all_translators[ix],
					&out_formats, &fmt_count)) {
				//	look through the output formats
				for (int iy=0; iy<fmt_count; iy++) {
					//	and take the first output format that isn't BBitmap
					if (out_formats[iy].type != B_TRANSLATOR_BITMAP) {
						m_translator = all_translators[ix];
						m_type_code = out_formats[iy].type;
						strcpy(m_the_type, out_formats[iy].MIME);
						err = B_OK;
						break;
					}
				}
			}
		}
	}
	delete[] all_translators;
	if (err < B_OK) Error("There are no graphics-export translators installed!");
	
	PlotColour = Black;
	TextColour = Black;
	GridColour = BeShadow;
	MinorColour = BeLightShadow;
	PlotIs = Linear;
	GridDrawn = false;
	GetLabel = NULL;
	ScatterPlot = false;
	ScatterChar[0] = '0';
	ZeroLineOn = true;
	CopyOfAPlot = false;
	Divx = 1;
	Divy = 1;
	SubDivx = 5;
	SubDivy = 5;
	Decx = 1;
	Decy = 1;
	GMxx = 0;
	GMxy = 0;
	xoff = 0;
	yoff = 0;
	TopBorder = 2;
	Points = NULL;
	initialized = false;
}

minimax mPlot::layoutprefs()
{
	return mpm=ct_mpm;
}

BRect mPlot::layout(BRect rect)
{
  	float Border1 = 0;
  	float Border2 = 0;
  	char DumStr[40];
	font_height h;
	float spaceHeight;
	
	// update width & height
	width = rect.Width(); height = rect.Height();

	// update space height		
	GetFontHeight(&h);
	spaceHeight = h.ascent + h.leading;	

  	// calculate the border dimensions
  	if (RealToString(Maxy, DumStr, 2, Decy, ' ')) {
    	Border1 = StringWidth(DumStr);
  	}
  	if (RealToString(Miny, DumStr, 2, Decy, ' ')) {
    	Border2 = StringWidth(DumStr);
  	}
  	if (Border1 > Border2) x = Border1;
  	else x = Border2;
  	y = TopBorder + yoff + spaceHeight;
  	
  	// determine the required right border
  	if (RealToString(Maxx, DumStr, 2, Decx, ' ')) {
    	Border1 = StringWidth(DumStr);
  	}
  	if (RealToString(Minx, DumStr, 2, Decx, ' ')) {
    	Border2 = StringWidth(DumStr);
  	}
  	if (Border1 < Border2) Border1= Border2; 	
  	GMxx = width - Border1*0.625 + xoff;
  	GMxy = height - spaceHeight*1.5+ yoff;
  	x += xoff+8;
  		
	ResizeTo(rect.Width(), rect.Height());
	MoveTo(rect.LeftTop());	
	ClearPlotHistory();
	Draw(rect);
	return rect;
}

mPlot::~mPlot()
{
	// dispose of points	
	ClearPlotHistory();
}

void mPlot::SetModel(const char *str, const char *v)
{
	model.SetTo(str, v);
	illegal = false;
	ClearPlotHistory();  // force regeneration of all points
	Invalidate();   // force redraw
}

void mPlot::SetPlotKind(PlotKindType kind)
{
	PlotIs = kind;
	ClearPlotHistory();  // force regeneration of all points	
	Invalidate();	// force redraw
}

void mPlot::SetDivisions(int32 xDivs, int32 yDivs, int32 xMinor, int32 yMinor)
{
	Divx = xDivs;
	Divy = yDivs;
	SubDivx = xMinor;
	SubDivy = yMinor;
	Invalidate();  // force redraw
}

void mPlot::SetDecimals(int32 xDec, int32 yDec)
{
	Decx = xDec;
	Decy = yDec;
}

void mPlot::PlotZero()
{
	if (ZeroLineOn) {
		if (ScatterPlot) {
			ScatterPlot = false;
			if (!(PlotIs & LogY))
				IntPlotLine(Red, Minx, 0.0, Maxx, 0.0);
			ScatterPlot = true;
		} else {
			if (!(PlotIs & LogY))
				IntPlotLine(Red, Minx, 0.0, Maxx, 0.0);
		}
	}
}

void mPlot::NewYLimits()
{	
	int32 intervals = (int32)(GMxx - mPlot::x);
	double intg = (Maxx - Minx) / intervals;
	double y;

	Miny = 1.0e10;
	Maxy = -1.0e10;
	for (int32 i = 0; i <= intervals; i++) {
		y = model.Eval(Minx + i*intg);
		if (y < Miny) Miny = y;
		if (y > Maxy) Maxy = y;
	};
	
	if (Miny == Maxy) {
		Maxy += 0.05;
		Miny -= 0.05;
	}
	if (initialized) layout(Bounds());
}

double mPlot::ScalePoint (double xl)
{
	switch (PlotIs) { 
  		case LogX: return (model.Eval(pow(TEN, xl)));
        case LogY: return (LogOf(model.Eval(xl)));
        case LogLog: return (LogOf(model.Eval(pow(TEN, xl))));
        default: return (model.Eval(xl));
 	}
}

void mPlot::Draw(BRect area)
{
	// Plot the current function
  	double lastx, lasty, intg, xl, yl, dx, dy;
  	double OldMinx, OldMaxx, OldMiny, OldMaxy;
  	int32 i;

  	// save the old plot window dimensions
  	if (Minx == Maxx) return;
  	OldMinx = Minx;  OldMaxx = Maxx;
  	OldMiny = Miny;  OldMaxy = Maxy;
  	SetPlotOffset(0, 0);
  	
  	// adjust for log scales
  	if (PlotIs & LogX) {
  		Minx = LogOf(Minx); 
  		Maxx = LogOf(Maxx);
  	}
  	if (PlotIs & LogY) {
  		Miny = LogOf(Miny); 
  		Maxy = LogOf(Maxy);
  	}

  	// calculate scaled plot window dimensions
  	dx = (Maxx - Minx) / xscale;
  	dy = (Maxy - Miny) / yscale;
  	Minx = Minx + dx * (xscale - 1.0) * 0.5;
  	Maxx = Minx + dx;
  	Miny = Miny + dy * (yscale - 1.0) * 0.5;
  	Maxy = Miny + dy;

  	// draw the plot grid
  	DrawGrid();

  	// initialize the plot variables  	
  	if (illegal) return;
  	BRegion plotClip;
	BRect cliparea(x, y, GMxx, GMxy);
  	plotClip.Set(cliparea);
  	ConstrainClippingRegion(&plotClip);
  	lastx = Minx;
  	lasty = ScalePoint(Minx);
  	if (Scanner::error || (Real::err != errNone)) {
  		illegal = true;  // prevent further errors
 		ConstrainClippingRegion(NULL); 
  		return;
  	}
  	xl = lastx;
  	yl = lasty;
  	if (PlotIs & (Bar | Line))
    		intg = (Maxx - Minx) / Divx;
  	else
    		intg = 4*(Maxx - Minx) / (GMxx - x);

  	if (Points != NULL) { // do a quick redraw
    	for (i=1; i< PointsMax; i++) {
      		IntPlotLine(PlotColour, Points[i-1].x, Points[i-1].y, Points[i].x, Points[i].y);
    	}
  	} else {
  		PointsMax = (long)((Maxx-Minx)/intg)+2;
    	Points = new BPoint[PointsMax];
   
    	// go through the range of x values and plot y values
    	i=0;
    	do {
      		if (PlotIs == Bar)
        		PlotBar(PlotColour, intg * 0.25, xl, yl);
      		else
        		IntPlotLine(PlotColour, lastx, lasty, xl, yl);
      		lastx = xl;
      		lasty = yl;
      		Points[i].x=xl; Points[i].y=yl; 
      		i++;
      		xl = xl + intg;
      		yl = ScalePoint(xl);
    	} while ((xl <= Maxx+intg) && (i < 1000));
  	}

  	PlotZero();

  	// restore the old plot window dimensions *)
  	Minx = OldMinx;  Maxx = OldMaxx;
  	Miny = OldMiny;  Maxy = OldMaxy;
  	ConstrainClippingRegion(NULL);        // remove clipping area
}

void mPlot::SetPlotColour (rgb_color Colour)
{
	PlotColour = Colour;
}

void mPlot::SetTextColour (rgb_color Colour)
{
	TextColour = Colour;
}

void mPlot::SetGridColour (rgb_color Minor, rgb_color Major)
{
	GridColour = Major;
	MinorColour = Minor;
}

void mPlot::SetZeroLine (bool On)
{
	ZeroLineOn = On;
}

void mPlot::SetLabelRoutine (LabelProc LabelRoutine)
{
	GetLabel = LabelRoutine;
}
