#ifndef CONVERSION_H
#define CONVERSION_H

#include "PWindow.h"
#include "MListView.h"
#include "MPopup.h"
#include "Handler.h"
#include "Reals.h"

// mime types
#define	B_CONVERT_MIMETYPE	"application/x-becalcConversion"
#define APP_SIG				"application/x-vnd.CI-CNVT"

// conversion field names
#define P_TYPE					"CI:type"
#define P_UNIT						"CI:units"
#define P_TOEQUATION		"CI:toEquation"
#define P_FROMEQUATION	"CI:fromEquation"
#define P_CONSTANT			"CI:konstant"

#define CONVERTTITLE "BeCalc Conversions"

class ConvertWin : public PWindow
{
	public:	
		ConvertWin(char*, window_type);
		virtual bool QuitRequested();
		virtual void MessageReceived(BMessage*);
		static void InitSettings();
		virtual void InitWindow();
		void DoConvert(Real &n, bool inverse = false);
			
		MPopup *ptypes;   		// select different conversions
		PListView* fromlist;	// from list items
		PListView* tolist;		// to list items
		MWindow *target;
			
	private:
		BList factors;				// conversion factors
		BMenu *types;			// conversion types
		
		void ReadTypes ();										// read conversion types
		void GetUnits (int32 tindex, MListView *v);	// read conversion units
		void GetFactorDef(int32 unit, char *from, char *to, char *con);
		static int Compare(const BStringItem **first, const BStringItem **second);
};

#endif
