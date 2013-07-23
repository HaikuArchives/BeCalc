/*
	Common source for liblayout-demos
	This file defines the basic window and application classes
	that any application with a GUI requires.
	
	This source is public domain. Customize at will.
*/

#include <stdio.h>
#include <InterfaceKit.h>
#include <Path.h>
#include <Roster.h>
#include "Buttons.h"
#include "Equations.h"
#include "BeCalcApp.h"
#include "TextRider.h"

// The MApplication receives font-change notifications, and sends them on
// to its windows.
BeCalcApp::BeCalcApp(char *id)
	: MApplication(id)
{
}

// Always call the inherited MessageReceived(), unless you don't
// want your windows to react to font-changes.
void BeCalcApp::MessageReceived(BMessage *message)
{	
 	const char *name;
 	
	switch(message->what)
	{
		case B_SAVE_REQUESTED:
			if ((name = message->FindString("name")) != NULL) {
				entry_ref ref;
				if (message->FindRef("directory", &ref) == B_OK) {
					BEntry entry(&ref, true);
					BPath path(&entry);
					path.Append(name);
					TextWriter w(path);
					if (w.Err() != brOK) break;
					w.WriteString("BeCalc Export of ");
					
					int32 id = message->FindInt32("list_id");
					switch (id)
					{
						case ID_EXPORT_EQUATIONS:
							w.WriteString("Equations"); w.WriteLn();
							Equations::Equation->Export(w);
							break;
						case ID_EXPORT_RESULTS:
							w.WriteString("Results"); w.WriteLn();
							Equations::Results->Export(w);
							break;
						case ID_EXPORT_VARIABLES:
							w.WriteString("Variables"); w.WriteLn();
							Variables::Export(w);
							break;
						case ID_EXPORT_MORTGAGE:
							w.WriteString("Mortgage List"); w.WriteLn();
							window->win->Export(w);
							break;
						case ID_EXPORT_ALL:
							w.WriteString("Equations, Results, and Variables"); w.WriteLn();
							Equations::Equation->Export(w);						
							Equations::Results->Export(w);
							Variables::Export(w);
							break;
					}
				}
			}
			break;
		default:
			//printf("received by application:\n");
			//message->PrintToStream();
			//printf("----------\n");
			MApplication::MessageReceived(message);
			break;
	}
}


