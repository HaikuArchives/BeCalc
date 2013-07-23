
/*
     Mortgage window for BeCalc.
*/

#include <stdio.h>
#include <math.h>
#include <Application.h>
#include <ScrollView.h>

#include "layout-all.h"

#include "Mortgage.h"
#include "Buttons.h"
#include "Colors.h"
#include "Errors.h"

static const char * INTP 				= "Mortgage-int";
static const char * PAYP			= "Mortgage-period";
static const char * VARSUSED 	= "Mortgage-vars";
static const char * N					= "Mortgage-n";
static const char * LIST				= "Mortgage-list";

MListView * MyStringView::list;

BRect MyStringView::layout (BRect rect)
{
	BRect r = MStringView::layout(rect);
	MortgageItem::widths[index] = rect.Width();
	list->Invalidate();
	return r;
}

void MortgageWin::InitSettings()
{
	Settings::SetPosition(MORTGAGENAME, BRect(100,110,300,300));
	Settings::SetColor(LIST, White);
	Settings::SetInt(INTP, 0);
	Settings::SetInt(PAYP, 2);
	Settings::SetInt(VARSUSED, 0);
	Settings::SetInt(N, 36);
}

// The MWindow receives font-change messages from the application.
// It also takes care of color-drops. 
MortgageWin::MortgageWin(char *name, window_type type)
	: PWindow(name, type), One(1.0), Zero(0.0)
{
	MView *topview;

    // define the top display area
    resultView =  new MyListView(LIST, false); // no drag 'n drop
    MyStringView::list = resultView;   			// so we can invalidate list when columns are resized
    			              			           
    // define the control buttons
	HGroup *buttons=
		new HGroup
			(new MButton("Find Unknown Value", MID_FIND_UNKNOWN), 
			 new MButton("Update Mortgage Table", MID_UPDATE_TABLE),
			 0);
			 			 			 
	// define the data entry controls
	VGroup *strings=
		new VGroup(		 
			new HGroup (
		     	new Space(minimax(20,-1,-1,-1,0.25)),	
		     	Amount = new PSpinButton("Amount:", SPIN_FLOAT),
		     	new Space(minimax(20,-1,-1,-1,0.25)),
			  	Interest = new PSpinButton("Interest:", SPIN_FLOAT),
			  	intPeriod = new MPopup("Per", "Year", "Quarter", "Month", "Week", "Day", 0),
			0),		 
			new HGroup (
			 	new Space(minimax(20,-1,-1,-1,0.25)),	
			 	Payment = new PSpinButton("Payment:", SPIN_FLOAT),
			 	new Space(minimax(20,-1,-1,-1,0.25)),			 	
			 	Intervals = new PSpinButton("Intervals:", SPIN_INTEGER),
			 	payPeriod = new MPopup(NULL, "Years", "Quarters", "Months", "Weeks", "Days", 0),
			 0),
		0);
			 
	// set up some limits and parameters
	Amount->SetMinimum(0);	
	Amount->SetMaximum(1000000000.0);
	Amount->SetStepSize(1000.0);
	Amount->SetFormat("%-01.2f");
	Payment->SetMinimum(0);
	Payment->SetFormat("%-01.2f");
	Payment->SetMaximum(1000000000.0);
	Interest->SetMinimum(0);	
	Interest->SetMaximum(100.0);
	Interest->SetStepSize(0.1);
	Interest->SetFormat("%-01.1f%%");
	Intervals->SetMinimum(0);	
			 			 
	// create a set of draggable column width adjustments
	HGroup *splitters =
		new HGroup (
			new MyStringView(0, "n", minimax(10,10,65536,65536, 0.5)),  new MSplitter(),
			new MyStringView(1, "Interest"), new MSplitter(),
			new MyStringView(2, "Total Interest"), new MSplitter(),
			new MyStringView(3, "Principal"), new MSplitter(),	
			new MyStringView(4, "Balance"),
			new Space(minimax(10, 10, 10, 10, 1)),											
		0);	
			     
    // equalize these gadgets vertically
    DivideSame(Amount, Payment, 0);
    DivideSame(Interest, Intervals, 0);
    
    // initialize the IntArray
    IntArray[0] = 365.25; IntArray[1] = 91.3125; IntArray[2] = 30.4375;
    IntArray[3] = 7.024038462; IntArray[4] = 1.0;
                 		
	// Build up the final view with the display, equation entry, and buttons
	topview= new MBorder
				(
					M_RAISED_BORDER, 3, "outer",
					new VGroup
					(
						splitters,
						new MScrollView(resultView, false, true, B_FANCY_BORDER, minimax(200, 100, 10000, 10000, 1)),
						buttons,
						strings,
					0)
				);
	
	// because topview is an MView, we need to cast it to a BView.
	AddChild(dynamic_cast<BView*>(topview));
	Show();	
}

void MortgageWin::InitWindow()
{
	IntPeriod = Settings::GetInt(INTP);
	intPeriod->SetActive(IntPeriod);
	PayPeriod = Settings::GetInt(PAYP);
	payPeriod->SetActive(PayPeriod);
	VarsUsed = Settings::GetInt(VARSUSED);
	n = Settings::GetInt(N);
	ExtractVar(Amount, AMOUNT, amount);
	ExtractVar(Interest, INTEREST, intg);
	ExtractVar(Payment, PAYMENT, pay);
	if (VarsUsed == (AMOUNT|INTEREST|PAYMENT|INTERVALS)) UpdateTable();
}

void MortgageWin::Export (TextWriter &w)
{
	BListItem* ptr;
	MortgageItem* item;
	int32 id = 0;
	char str[64];
	
	if (w.Err() != brOK) return;
	w.WriteString("n\tInterest\tTotal Interest\tPrincipal\tBalance"); w.WriteLn();
	while ((ptr = resultView->ItemAt(id++)) != NULL) {
		item = (MortgageItem *)ptr;
		for (int32 i=0; i<5; i++) {
			if (i==0) sprintf(str, "%ld", (long)item->content[0]);
			else sprintf(str, "%.2f", item->content[i]);
			w.WriteString(str); 
			if (i < 4) w.WriteString("\t");
		}
		w.WriteLn();
	}
}

bool MortgageWin::QuitRequested()
{
	// save current pop-up positions and n
	Real tmp;
	
	PWindow::QuitRequested();
	
	// save settings
	Settings::SetInt(INTP, IntPeriod);
	Settings::SetInt(PAYP, PayPeriod);
	Settings::SetInt(VARSUSED, VarsUsed);
	Settings::SetInt(N, n);
		
	// destroy all the list items
	MortgageItem* TheItem;
	resultView->RemoveSelf();
	do
	{
		TheItem = (MortgageItem*)resultView->RemoveItem(int32(0));
		if (TheItem) delete TheItem;
	}while(TheItem);
	delete resultView;
	
	// unghost the mortgage button
	if (target->Lock()) {
    	DispatchMessage(new BMessage(ID_CLOSE_MORTGAGE), target);
    	target->Unlock();
	}
	
	// shut down this window
	Quit();  	 // call the parent
	return true;
}

// Always call the inherited MessageReceived(), unless you don't
// want your windows to react to font-changes and color-drops.
void MortgageWin::MessageReceived(BMessage *message)
{
	int32 id;
	void *ptr;
	
	switch(message->what)
	{
		case M_BUTTON_SELECTED:
			id = message->FindInt32("butID");
			if (id == MID_FIND_UNKNOWN) FindUnknown();
			else if (id == MID_UPDATE_TABLE) UpdateTable();
			MWindow::MessageReceived(message);			
			break;
		case M_POPUP_SELECTED:
			id = message->FindInt32("index");
			message->FindPointer("popup", &ptr);
			if (ptr == intPeriod) {
				// interest period
				IntPeriod = id;
			} else if (ptr == payPeriod) {
				// payment period
				PayPeriod = id; 
			}
			MWindow::MessageReceived(message);
			break;
		case M_SPIN_TICK:
			// refresh spin buttons -- bug in GUI lib
			Payment->SetValue(Payment->Value());
			Amount->SetValue(Amount->Value());
			Interest->SetValue(Interest->Value());
			Intervals->SetValue(Intervals->Value());
			break;
		default:
			//printf("received by window:\n");
			//message->PrintToStream();
			//printf("----------\n");
			MWindow::MessageReceived(message);
			break;
	}
}

//******************************************************************************************************
//**** MortgageItem
//******************************************************************************************************

MortgageItem::MortgageItem(uint32 level, bool superitem, bool expanded, int32 number, Real & interest, Real & total_int, Real & principal, Real & balance)
: BListItem()
{
	content[0] = number;
	content[1] = Real::Short(interest);
	content[2] = Real::Short(total_int);
	content[3] = Real::Short(principal);
	content[4] = Real::Short(balance);	
}

MortgageItem::~MortgageItem()
{ }

void MortgageItem::DrawItem(BView* owner, BRect frame, bool complete)
{
	// do item selection
	if (IsSelected() || complete) {	
		rgb_color color; 
		if (IsSelected()) { 
			color = BeListSelectGrey;		
		} else {
			color = owner->ViewColor();
		}			
		owner->SetHighColor(color);
		owner->FillRect(frame); 
	}
	
	// draw the strings right-justified		
	float leftOff = 0; 
	float swidth;
	char str[64];
	owner->SetDrawingMode(B_OP_OVER);  // IMPORTANT!		
	if (IsEnabled()) owner->SetHighColor(Black); 
	else owner->SetHighColor(BeDarkShadow); 
	for (int32 i = 0; i < 5; i++) {
		if (i==0) sprintf(str, "%ld", (long)content[0]);
		else sprintf(str, "%.2f", content[i]);
		swidth = owner->StringWidth(str);
		leftOff += widths[i] - swidth;
		owner->MovePenTo(leftOff, frame.bottom-2);
		owner->DrawString(str);
		leftOff += swidth;
	}
}

double MortgageItem::widths[5] = {15, 50, 50, 50, 50};  // gets updated later

void MortgageWin::UpdatePayment()
{
	Payment->SetValue(Real::Short(pay));
}

void MortgageWin::UpdateInterval()
{
	Intervals->SetValue(n);
}

void MortgageWin::UpdateAmount()
{
	Amount->SetValue(Real::Short(amount));
}

void MortgageWin::UpdateInterest()
{
	Interest->SetValue(Real::Short(intg));
}

void MortgageWin::AddListLine(long i, Real& paidInt, Real& cumInt, Real& Prin, Real& Rem)
{
	resultView->AddItem(new MortgageItem(0, false, false, i, paidInt, cumInt, Prin, Rem));	
}

void MortgageWin::DetermineInterest(Real& i)
{
	double p;
	
	p = 100 * IntArray[IntPeriod] / IntArray[PayPeriod];
	if(p < 1)
		p = 1;
	i = Real::div(intg, Real::Long(p));
}

void MortgageWin::FindAnnuity(Real& pay, Real& principal, long intervals)
{
	Real interest;
	
	DetermineInterest(interest);
	pay = Real::power(Real::add(One, interest), Real::Long(intervals));
	interest = Real::mul(pay, interest);
	pay = Real::sub(pay, One);
	pay = Real::div(Real::mul(principal, interest), pay);
}

void MortgageWin::FindAmount(Real& amount, Real& annuity, long n)
{
	Real interest;
	
	DetermineInterest(interest);
	amount = Real::power(Real::add(One, interest), Real::Long(n));
	interest = Real::mul(interest, amount);
	amount = Real::sub(amount, One);
	amount = Real::div(Real::mul(annuity, amount), interest);
}

double MortgageWin::ipower(double x, long i)
{
	double y;
	
	y = 1;
	while(1)
	{
		if (i &1) y *= x;
		i /= 2;
		if (i == 0) break;
		x *= x;
	}
	return y;
}

double MortgageWin::PAFunction(double i, double P, double A, long n)
{
	double ip;
	
	ip = ipower(1 + i, n);
	return ((ip - 1) / (i * ip)) - (P/A);
}

void MortgageWin::EstInterestRate(double& low, double& high, double P, double A, long n)
{
	double delta, r, plow;
	bool nplus, plus;
	
	if(low == 0)
		low = 1.0e-8;
	delta = (high - low) / 10;
	r = PAFunction(low, P, A, n);
	plus = r >= 0;
	plow = low;
	low += delta * 0.25;
	while(low <= high)
	{
		r = PAFunction(low, P, A, n);
		nplus = r >= 0;
		if(nplus != plus)
		{
			high = low;
			low = plow;
			return;
		}
		plus = nplus;
		plow = low;
		low += delta;
	}
}

void MortgageWin::FindInterest(Real& interest, Real& amount, Real& payment, long n)
{
	double low, high, P, A, p;
	
	low = 0;
	high = 1;
	P = Real::Short(amount);
	A = Real::Short(payment);
	while (high - low > 1.0e-12) EstInterestRate(low, high, P, A, n);
	p = 100 * IntArray[IntPeriod] / IntArray[PayPeriod];
	if (p < 1) p = 1;
	interest = Real::Long(p * low);
}

long MortgageWin::FindIntervals(Real& amount, Real& payment)
{
	Real interest;
	double n, f, p, i;
	
	DetermineInterest(interest);
	i = Real::Short(interest);
	f = Real::Short(amount);
	p = Real::Short(payment);
	
	n = log(p / (p - i * f)) / log(1 + i);
	return (long)(n + 0.5);
}

void MortgageWin::Amortize()
{
	Real AmtInterest, intPaid, i, prin, rem;
	long j;
	
	AmtInterest = Zero;
	rem = amount;
	DetermineInterest(i);
	
	// delete the old list lines -- if any
	BListItem *item;
	while ((item = resultView->RemoveItem((int32) 0)) != NULL) {
		delete item;
	}
	
	// create new set of list lines
	for(j = 1 ; j <= n ; j++) {
		intPaid = Real::mul(i, rem);
		AmtInterest = Real::add(AmtInterest, intPaid);
		prin = Real::sub(pay, intPaid);
		rem = Real::sub(rem, prin);
		AddListLine(j, intPaid, AmtInterest, prin, rem);
	}
}

void MortgageWin::ExtractVar(PSpinButton *var, int32 flag, Real &v)
{	
	Real t(var->Value());
	if (Real::cmp(t, Zero) != 0) {
 		VarsUsed |= flag;
      		v=t;
	}
}

void MortgageWin::FindUnknown()
{
	Real tmp;
	
	// extract the active control state
	VarsUsed = 0;
	ExtractVar(Amount, AMOUNT, amount);
	ExtractVar(Intervals, INTERVALS, tmp); n = (long)Real::Short(tmp);
	ExtractVar(Interest, INTEREST, intg);
	ExtractVar(Payment, PAYMENT, pay);
	
	// compute unknown quantity
	if(VarsUsed == (AMOUNT|INTEREST|PAYMENT))
	{
		n = FindIntervals(amount, pay);
		UpdateInterval();
		VarsUsed |= INTERVALS;
	}
	else if(VarsUsed == (AMOUNT|INTEREST|INTERVALS))
	{
		FindAnnuity(pay, amount, n);
		UpdatePayment();
		VarsUsed |= PAYMENT;
	}
	else if(VarsUsed == (AMOUNT|PAYMENT|INTERVALS))
	{
		FindInterest(intg, amount, pay, n);
		UpdateInterest();
		VarsUsed |= INTEREST;
	}
	else if(VarsUsed == (INTEREST|PAYMENT|INTERVALS))
	{
		FindAmount(amount, pay, n);
		UpdateAmount();
		VarsUsed |= AMOUNT;
	}
	else if(VarsUsed != (AMOUNT|INTEREST|PAYMENT|INTERVALS))
	{
		Error("Exactly one unknown quantity is required");
	}
}

void MortgageWin::UpdateTable()
{
	FindUnknown();
	if (VarsUsed == (AMOUNT|INTEREST|PAYMENT|INTERVALS))
		Amortize();
}

void MortgageWin::ClearVars()
{
	VarsUsed = 0;
	amount = Zero; UpdateAmount();
	intg = Zero; UpdateInterest();
	pay = Zero; UpdatePayment();
	n = 0; UpdateInterval();
}
