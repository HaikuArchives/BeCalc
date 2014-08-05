
#include "layout-all.h"
#include "PWindow.h"

enum {PID_SORTED_VARS, PID_SENSITIVE_VARS, PID_UNIQUE, 
	  PID_DELETE_BOTH, PID_PREF_SELECTED, 
	  PID_REVERT, PID_DEFAULT, PID_UNLIMITED};

#define PREFNAME "BeCalc Preferences"

class PrefWin : public PWindow
{
	public:	
		PrefWin(char*, window_type);
		virtual bool QuitRequested();
		virtual void InitWindow();
		static void InitSettings();
		virtual void MessageReceived(BMessage*);
			
		MWindow *target;
		MPopup *lDP, *rDP;
		char leftDP, rightDP;	
		PSpinButton *digs, *dec, *eqLimit;
		PListView *list;
		LayeredGroup *settings;
		PCheckBox *uniquebox, *deletebox;
		PCheckBox *sortbox, *casebox, *unlimited;
			
	private:
		void SetState(int32 digs = 16, int32 dp = 0, char leftSep = '\0', 
					  char rightSep = '\0', int32 eqLimit = 0, 
					  bool sorted = false, bool sensitive = true,
					  bool unique = false, bool delboth = false,
					  bool nolimit = true); 
		
		// cached values
		int32 olddigs, olddp, oldeqLimit;
		char oldleftSep, oldrightSep;
		bool oldsorted, oldsensitive, oldunique, olddelboth, oldunlimited;
};

