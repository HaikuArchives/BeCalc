#ifndef PREFWINDOW_H
#define PREFWINDOW_H

#include <String.h>
#include <InterfaceKit.h>
#include "layout-all.h"

class PWindow : public MWindow
{
	public: 
		PWindow(const char *name, window_type type);	
		virtual bool QuitRequested();
		virtual void WindowActivated(bool active);
		virtual void InitWindow() {};
		
	private:
		BString wname;
		bool initialized;
};

class PListView : public MListView
{
	public:		
		PListView(const char *name, list_view_type type=B_SINGLE_SELECTION_LIST, 
						minimax size = minimax(50,50))
			: MListView(type, size), lname(name) {};
			
		virtual void AttachedToWindow();
		virtual void DetachedFromWindow();
		
	private:
		BString lname;
};

class PTextView : public MTextView
{
	public:		
		PTextView(const char *name, minimax size = 0) : MTextView(size), tname(name) {};
			
		virtual void AttachedToWindow();
		virtual void DetachedFromWindow();
		
	private:
		BString tname;
};

class PCheckBox : public MCheckBox
{
	public:		
		PCheckBox(const char *label, ulong id=0, bool state=false)
			: MCheckBox(label, id, state), cname(label) {};
				
		virtual void AttachedToWindow();
		virtual void DetachedFromWindow();
		
	private:
		BString cname;
};

class PSpinButton : public SpinButton
{
	public:		
		PSpinButton(const char *label, spinmode mode, BHandler *target=NULL)
			: SpinButton(label, mode, target), sname(label) {};
		PSpinButton(const char *name, const char *label, spinmode mode, BHandler *target=NULL)
			: SpinButton(label, mode, target), sname(name) {};
				
		virtual void AttachedToWindow();
		virtual void DetachedFromWindow();
		
	private:
		BString sname;
};

#endif
