
#include "PWindow.h"
#include "MyListView.h"
#include "MPopup.h"
#include "MTextControl.h"
#include "TextRider.h"
#include "Handler.h"
#include "Settings.h"

#include "Reals.h"

enum Unknown_vars {
	AMOUNT  = 0x1,
	INTEREST = 0x2,
	PAYMENT = 0x4,
	INTERVALS = 0x8
};

enum Mortgage_ids {
	MID_FIND_UNKNOWN, MID_UPDATE_TABLE
};

#define MORTGAGENAME "BeCalc Mortgage"

class MyStringView : public MStringView
{
	public:
		MyStringView (int32 id, const char *label) : MStringView(label, B_ALIGN_CENTER), index(id) {};
		MyStringView (int32 id, const char *label, minimax m) : MStringView(label, B_ALIGN_CENTER, m), index(id) {};
		virtual ~MyStringView () {};
		
		friend class MortgageWin;
		virtual BRect layout (BRect rect);
		
	protected:
		static MListView *list;   // list to invalidate for width changes
		
	private:
		int32 index;
};

class MortgageWin : public PWindow
{
	public:	
		MortgageWin(char*, window_type);
		virtual bool QuitRequested();
		virtual void MessageReceived(BMessage*);
		virtual void InitWindow();
		static void InitSettings();
		void ComputeMortgage();	
		void SetAmount();
		void SetIntervals();
		void SetInterest();
		void SetPayment();
		void SetInterestPeriod();
		void SetIntervalPeriod();
		void FindUnknown();
		void UpdateTable();
		void Deselect();
		void ClearVars();
		void Export (TextWriter &w);
			
		PSpinButton *Amount;
		PSpinButton *Payment;
		PSpinButton *Interest;
		PSpinButton *Intervals;
		MPopup *intPeriod;
		MPopup *payPeriod;	
		MyListView *resultView; 	// Mortgage results lists
		MWindow *target;
			
	private: 
		typedef float PerArray[5];
		
		PerArray IntArray;
		int32 IntPeriod, PayPeriod;
		const Real One, Zero;
		int32 VarsUsed, n;
		Real amount, intg, pay;
		
		void UpdatePayment(void);
		void UpdateInterval(void);
		void UpdateAmount(void);
		void UpdateInterest(void);
		void AddListLine(long, Real&, Real&, Real&, Real&);
		void DetermineInterest(Real&);
		void FindAnnuity(Real&, Real&, long);
		void FindAmount(Real&, Real&, long);
		double ipower(double, long);
		double PAFunction(double, double, double, long);
		void EstInterestRate(double&, double&, double, double, long);
		void FindInterest(Real&, Real&, Real&, long);
		long FindIntervals(Real&, Real&);
		void Amortize(void);
		void ExtractVar(PSpinButton *var, int32 flag, Real &v);
};			 

class MortgageItem : public BListItem
{
	friend class MortgageWin;
	public:
		MortgageItem(uint32 level, bool superitem, bool expanded, int32 number, Real & interest, Real & total_int, Real & principal, Real & balance);
		~MortgageItem();
		
		// override this method to format list properly (ie. right-aligned)
		friend class MyStringView;
		virtual void DrawItem(BView* owner, BRect frame, bool complete);
		
	protected:
		static double widths[5];
		double content[5];

};
