#include <AppKit.h>
#include <InterfaceKit.h>
#include <StorageKit.h>
#include "Register.h"
#include <Application.h>
#include <stdio.h>

//
// main program
//
int main()
{
	BApplication app("application/x-vnd.CI-CreateLinks");
			
	// initialize registration module
	//Register::InitRegister();

	// create a link in the deskbar applications
	BPath org, link;
	find_directory(B_USER_DESKBAR_DIRECTORY, &link);
	link.Append("Applications");
//	Register::GetAppDir(org); 
	org.GetParent(&org); org.Append("BeCalc");
	BDirectory dir(link.Path());
	BSymLink *nlink = NULL;
	BAlert *myAlert = new BAlert("title", "Create a link to BeCalc in deskbar?", "No", "Yes", NULL,
		B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
	int32 button_index = myAlert->Go();
    if (button_index > 0)
		dir.CreateSymLink("BeCalc", org.Path(), nlink);
	printf("Original file is %s\nand the link is in %s\n", org.Path(), link.Path());
	
	// create a link on the desktop
	find_directory(B_DESKTOP_DIRECTORY, &link);
	BDirectory ndir(link.Path());
	myAlert = new BAlert("title", "Create a link to BeCalc on the desktop?", "No", "Yes", NULL,
		B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
	button_index = myAlert->Go();
	if (button_index > 0)
		ndir.CreateSymLink("BeCalc", org.Path(), nlink);
	printf("Original file is %s\nand the link is in %s\n", org.Path(), link.Path());
	return(0);
}

