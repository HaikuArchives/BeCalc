#ifndef BECALCWIN_H
#define BECALCWIN_H

#include "layout-all.h"

#include "About.h"
#include "Conversion.h"
#include "Help.h"
#include "Mortgage.h"
#include "Plots.h"
#include "PWindow.h"
#include "PrefWin.h"
#include "Settings.h"
#include "Variables.h"


#include <FilePanel.h>


// local redefinition of MTextView
class MyTextView : public MTextView
{
	public:
		MyTextView(minimax size=0) : MTextView(size) {};
		virtual ~MyTextView() {};
		virtual void KeyDown (const char *bytes, int32 numBytes);
		virtual void MakeFocus (bool flag = true);
		virtual bool AcceptsDrop (const BMessage *message);
};

#define BECALCNAME "BeCalc"

class BeCalcWin : public PWindow
{
	friend class MyTextView;
	public:
		enum DisplayState {dsReturn, dsAppend, dsClear};
		enum OpenWindows {
			wNone = 0,
			wMortgage = 1,
			wHelp = 2,
			wConvert = 4,
			wAbout = 8,
			wPref = 16,
			wPlot1 = 32,
			wPlot2 = 64,
			wPlot3 = 128,
			wPlot4 = 256,
			wPlot5 = 512,
			wPlot6 = wPlot5*2,
			wPlot7 = wPlot6*2,
			wPlot8 = wPlot7*2,
			wPlot9 = wPlot8*2,
			wPlot10 = wPlot9*2,
			wPlot11 = wPlot10*2,
			wPlot12 = wPlot11*2,
			wPlot13 = wPlot12*2,
			wPlot14 = wPlot13*2,
			wPlot15 = wPlot14*2,
			wPlot16 = wPlot15*2,
			wPlot17 = wPlot16*2,
			wPlot18 = wPlot17*2,
			wPlot19 = wPlot18*2,
			wPlot20 = wPlot19*2,
			wPlot21 = wPlot20*2,
			wPlot22 = wPlot21*2,
			wPlot23 = wPlot22*2,
			wPlot24 = wPlot23*2,
			wReserved1 = wPlot24*2,
			wReserved2 = wReserved1*2
		};
	
		BeCalcWin(char*, window_type);
		virtual bool QuitRequested();
		virtual void MessageReceived(BMessage*);
		virtual void WindowActivated(bool active);
		virtual void InitWindow();
		virtual void MenusBeginning(void);
		static void InitSettings();
		
		MyTextView *display; 			// main display
		MPopup *angle;        				// angle popup
		MPopup *radix;       	   			// radix popup
		MPopup *format;					// number format popup
		MPopup *prefix;						// number prefix popup
		TabGroup *tab;						// key tabs
		TabGroup *ltab;						// equation list tabs
		MScrollView *s1;
		MScrollView *s2;
		MScrollView *s3;			
		MStringView *nStat;         		// statistic n display
		int plotCount;							// number of open plots
		MyListView *Vars;					// local variable list
		MortgageWin *win;	
		PlotWin *pwin;
		ConvertWin *cwin;
		AboutWin *awin;
		HelpWin *hwin;
		PrefWin *prefwin;
		BFilePanel *save;
			
	private: 
		DisplayState state;
		Complex lastResult;
		BList plots;								// list of all plots
		int32 openwin;						// all the current open windows
		
		void Recalculate();
		void SavePanel (const char *file, const char *button, int32 id);
};

#endif
