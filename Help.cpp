
/*
     Mortgage window for BeCalc.
*/

#include <stdio.h>
#include <Application.h>
#include <ScrollView.h>
#include "VGroup.h"
#include "HGroup.h"
#include "MButton.h"
#include "TabGroup.h"
#include "LayeredGroup.h"
#include "MMenuBar.h"
#include "MRadioGroup.h"
#include "MBViewWrapper.h"
#include "MTextView.h"
#include "MListView.h"
#include "MTextControl.h"
#include "MSplitter.h"
#include "MCheckBox.h"
#include "Space.h"
#include "MPopup.h"
#include "MBorder.h"
#include "MDragBar.h"
#include "MProgressBar.h"
#include "PropGadget.h"
#include "MScrollView.h"
#include "MenuItem.h"
#include "MStringView.h"
#include "Help.h"
#include "MyItem.h"
#include "Buttons.h"
#include "Colors.h"
#include "Settings.h"

class HelpItem : public MyItem
{
public:
	HelpItem (const char *label, const char *str) : MyItem(label), content(str) {};
	virtual const char* ContentStr() const {return content.String();};
	virtual const char* DragStr() const {return content.String();};
	virtual float MaxWidth() {return hwidth;};
	virtual void SetMaxWidth(float w) {hwidth = w;};
private:
	static float hwidth;
	BString content;
};

float HelpItem::hwidth = 32.0;
static const char *HELPLIST = "Help-list";

// The MWindow receives font-change messages from the application.
// It also takes care of color-drops. 
HelpWin::HelpWin(char *name, window_type type) : PWindow(name, type)
{
	MView *topview;
	
    // define the top display area
    resultView = new MyListView(HELPLIST, false);  // don't allow drag 'n drop
    			           
	// add help items
	AddHelp();
             		
	// Build up the final view 
	topview= new MBorder
				(
					M_RAISED_BORDER, 3,"outer",
					new MScrollView(resultView, false, true, B_FANCY_BORDER, minimax(300, 200))
				);

	// because topview is an MView, we need to cast it to a BView.	
	AddChild(dynamic_cast<BView*>(topview));
	Show();
}

void HelpWin::InitSettings()
{
	Settings::SetPosition(HELPNAME, BRect(100, 100, 300, 300));
	Settings::SetColor(HELPLIST, White);
}

void HelpWin::AddHelp() {
	resultView->AddItem(new HelpItem("+","Addition"));
  	resultView->AddItem(new HelpItem("-","Subtraction"));
  	resultView->AddItem(new HelpItem("*, ×","Multiplication"));
  	resultView->AddItem(new HelpItem("/, ÷","Division"));
  	resultView->AddItem(new HelpItem("²","Squared"));
  	resultView->AddItem(new HelpItem("³","Cubed"));
  	resultView->AddItem(new HelpItem("¯¹","Reciprocal"));
   	resultView->AddItem(new HelpItem("!","Factorial"));
  	resultView->AddItem(new HelpItem("()","Brackets"));
  	resultView->AddItem(new HelpItem("^, **","Power"));
  	resultView->AddItem(new HelpItem("cnt","Count logical 1 bits"));
  	resultView->AddItem(new HelpItem("&, and","Logical And"));
  	resultView->AddItem(new HelpItem("nand","Logical Nand"));
  	resultView->AddItem(new HelpItem("|, or","Logical Inclusive Or"));
  	resultView->AddItem(new HelpItem("nor","Logical Nor"));
  	resultView->AddItem(new HelpItem("xor","Logical Exclusive Or"));
  	resultView->AddItem(new HelpItem("~, not","Logical Complement"));
  	resultView->AddItem(new HelpItem("mod","Modulo"));
  	resultView->AddItem(new HelpItem("div","Integer Division"));
  	resultView->AddItem(new HelpItem("√, sqrt","Square Root"));
  	resultView->AddItem(new HelpItem("³√, cbrt","Cube Root"));
  	resultView->AddItem(new HelpItem("*√, root","Any Root"));
  	resultView->AddItem(new HelpItem("abs","Absolute value and complex magnitude"));
  	resultView->AddItem(new HelpItem("rand","Random number between 0 and 1"));
  	resultView->AddItem(new HelpItem("e","Natural Log Base"));
  	resultView->AddItem(new HelpItem("i","Imaginary number"));
  	resultView->AddItem(new HelpItem("ø","Complex angle"));
  	resultView->AddItem(new HelpItem("im","Imaginary part of a number"));
  	resultView->AddItem(new HelpItem("re","Real part of a number"));
  	resultView->AddItem(new HelpItem("rect","Polar to rectangle conversion"));
  	resultView->AddItem(new HelpItem("conj","Complex conjugate"));
  	resultView->AddItem(new HelpItem("conv <n>","Convert n units to destination units"));
  	resultView->AddItem(new HelpItem("convi <n>","Convert n units from destination units"));
  	resultView->AddItem(new HelpItem("nCr","Factorial combinations"));
  	resultView->AddItem(new HelpItem("nPr","Factorial permutations"));
  	resultView->AddItem(new HelpItem("frac","Fractional part of a number"));
  	resultView->AddItem(new HelpItem("int","Integer part of a number"));
  	resultView->AddItem(new HelpItem("sign","Sign of a number (-1 or 1)"));
  	resultView->AddItem(new HelpItem("fib","Fibonacci function of a number"));
  	resultView->AddItem(new HelpItem("gcd(<x>;<y>)","Greatest common integer divisor of x and y"));
  	resultView->AddItem(new HelpItem("min(<x>;<y>)","Minimum of x and y"));
  	resultView->AddItem(new HelpItem("max(<x>;<y>)  ","Maximum of x and y"));
  	resultView->AddItem(new HelpItem("ln, log","Natural or base e Logarithm"));
   	resultView->AddItem(new HelpItem("log2","Base 2 Logarithm"));
  	resultView->AddItem(new HelpItem("log10","Base 10 Logarithm"));
  	resultView->AddItem(new HelpItem("sin, asin, sin¯¹","Sine, Arcsine"));
  	resultView->AddItem(new HelpItem("cos, acos, cos¯¹","Cosine, Arccosine"));
  	resultView->AddItem(new HelpItem("tan, atan, tan¯¹","Tangent, Arctangent"));
  	resultView->AddItem(new HelpItem("cot, acot, cot¯¹","Cotangent, Arccotangent"));
  	resultView->AddItem(new HelpItem("sinh, asinh, sinh¯¹","Hyperbolic Sine, Arcsine"));
  	resultView->AddItem(new HelpItem("cosh, acosh, cosh¯¹","Hyperbolic Cosine, Arccosine"));
  	resultView->AddItem(new HelpItem("tanh, atanh, tanh¯¹","Hyperbolic Tangent, Arctangent"));
  	resultView->AddItem(new HelpItem("coth, acoth, coth¯¹","Hyperbolix Cotangent, Arccotangent"));
  	resultView->AddItem(new HelpItem("sbit","Set Bit"));
  	resultView->AddItem(new HelpItem("cbit","Clear Bit"));
  	resultView->AddItem(new HelpItem("tbit","Toggle Bit"));
  	resultView->AddItem(new HelpItem("shr","Shift Right"));
  	resultView->AddItem(new HelpItem("shl","Shift Left"));
  	resultView->AddItem(new HelpItem("asr","Arithmetic Shift Right"));
  	resultView->AddItem(new HelpItem("ror","Rotate Right"));
  	resultView->AddItem(new HelpItem("rol","Rotate Left"));
  	resultView->AddItem(new HelpItem("<var>","Variable contents"));
  	resultView->AddItem(new HelpItem("<var> = <exp>","Store to variable"));
  	resultView->AddItem(new HelpItem("π, pi","Constant Pi"));
   	resultView->AddItem(new HelpItem("»x","Enter one statistic variable"));
  	resultView->AddItem(new HelpItem("»x;y","Enter two statistic variables"));
 	resultView->AddItem(new HelpItem("avg(x)","Average of variable 1"));
  	resultView->AddItem(new HelpItem("avg(y)","Average of variable 2"));
  	resultView->AddItem(new HelpItem("∑xy","Sum of the product of both variables"));
  	resultView->AddItem(new HelpItem("∑x","Sum of variable 1"));
  	resultView->AddItem(new HelpItem("∑y","Sum of variable 2"));
  	resultView->AddItem(new HelpItem("Sx","Sigma of variable 1"));
  	resultView->AddItem(new HelpItem("Sy","Sigma of variable 2"));
  	resultView->AddItem(new HelpItem("∑x²","Sum of the squares of variable 1"));
  	resultView->AddItem(new HelpItem("∑y²","Sum of the squares of variable 2"));
  	resultView->AddItem(new HelpItem("dev(x)","Standard deviation of variable 1"));
  	resultView->AddItem(new HelpItem("dev(y)","Standard deviation of variable 2"));
  	resultView->AddItem(new HelpItem("n","Number of statistical samples"));
  	resultView->AddItem(new HelpItem("CS","Clear statistics (n = 0)"));
}

bool HelpWin::QuitRequested()
{
	// destroy all the list items
	PWindow::QuitRequested();
	HelpItem* TheItem;
	resultView->RemoveSelf();  // to avoid flicker
	do {
		TheItem = (HelpItem*)resultView->RemoveItem(int32(0));
		if (TheItem) delete TheItem;
	}while(TheItem);
	delete resultView;
	
	// unghost the mortgage button
	if (target->Lock()) {
    	DispatchMessage(new BMessage(ID_CLOSE_COMMANDS), target);
    	target->Unlock();
	}
	
	// shut down this window
	Quit();  		// call the parent
	return true;
}
