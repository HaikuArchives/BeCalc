/*
	PrefServer client code, previously tucked away in a library
	Copyright 1996-1999 Marco Nelissen <marcone@xs4all.nl>

	History
	
	1.2 - Code for reading/writing files built in, in case PrefServer
		  cannot be found.

	1.1 - Use and Save now wait for a reply, to force the client
	      to stick around long enough for the server to figure
	      out who the client is.
	
	1.0 - First release, forgot when.
	
	
*/

#include "Preference.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <Roster.h>
#include <Screen.h>
#include <FindDirectory.h>
#include <Directory.h>
#include <Path.h>

#undef debug
// handy debugging macro, use like:  debug(("error number %d\n",errornumber));
//#define debug(x) printf x
#define debug(x)

char Preference::servername[]="application/x-marcone-prefserver";

Preference::Preference(char *ident)
	: BMessage()
{
	if(ident)
		target=strdup(ident);
	else
		target=NULL;
}

Preference::~Preference()
{
	free(target);
}

// add some items to the message to tell the server who we are
void Preference::PrepareTransfer()
{
	char *user;
	status_t status;

	if(target)
	{
		debug(("adding target %s\n",target));
		AddString("_prefserver:prefid",target);
	}

	debug(("adding user\n",target));
	user=getenv("USER");
	if(user)
		AddString("_prefserver:user",user);
	else
		AddString("_prefserver:user","<Mr.X>");
		
	debug(("user added\n"));

	if(!be_roster->IsRunning(servername))
		be_roster->Launch(servername);
	postman=BMessenger(servername,-1,&status);
}

// clean up the message
void Preference::FinishTransfer()
{
	RemoveName("_prefserver:prefid");
	RemoveName("_prefserver:user");
}

// used in case PrefServer is not running/present
char *Preference::GetPrefsPathName(bool forreal)
{
	// create settings dir if needed
	BPath prefpath;
	if(forreal)
		find_directory(B_USER_SETTINGS_DIRECTORY,&prefpath);
	else 
		find_directory(B_COMMON_TEMP_DIRECTORY,&prefpath);
	BDirectory dir(prefpath.Path());
	dir.CreateDirectory("PrefServer",NULL);

	// get full path name
	const char *user=FindString("_prefserver:user");
	const char *sig=FindString("_prefserver:prefid");
	char fullsig[B_FILE_NAME_LENGTH];
	sprintf(fullsig,"pref0:%s:%s",user,sig);
	for(uint i=0;i<strlen(fullsig);i++)
		if(fullsig[i]=='/')
			fullsig[i]=':';
	sprintf(pathname,"%s/PrefServer/%s",prefpath.Path(),fullsig);
	return pathname;
}


/*
	To save a preference, create an instance of Preference, put all the
	information you wish to save into it (a preference is a BMessage),
	and call Preference::Save().
	Save() will contact the PrefServer and make it store the preference.
	If successful, Save() returns B_OK, otherwise it returns an error code.
*/

status_t Preference::Save()
{
	BMessage reply;
	PrepareTransfer();
	what=SAVEPREF;
	status_t ret;
	if(postman.IsValid())
	{
		ret=postman.SendMessage(this,&reply,2E6);
	}
	else
	{
		// PrefServer not installed, flatten message and save it
		
		// first erase tempfile (the one written by ::Use
		char *path=GetPrefsPathName(false);
		unlink(path);

		path=GetPrefsPathName(true);
		debug(("saving pref to %s\n",path));
		BFile file(path,B_WRITE_ONLY|B_CREATE_FILE|B_ERASE_FILE);
		ret=Flatten(&file);
		if(ret==B_OK)
			debug(("save succeeded\n"));
		else
			debug(("save failed\n"));
	}
	FinishTransfer();
	return ret;
}

status_t Preference::Use()
{	
	BMessage reply;
	PrepareTransfer();
	what=USEPREF;
	status_t ret;
	if(postman.IsValid())
	{
		ret=postman.SendMessage(this,&reply,2E6);
	}
	else
	{
		char *path=GetPrefsPathName(false);
		BFile file(path,B_WRITE_ONLY|B_CREATE_FILE|B_ERASE_FILE);
		ret=Flatten(&file);
	}
	FinishTransfer();
	return ret;
}

/*
	To retrieve a preference, create a model Preference, containing sensible
	default settings if	needed, and call Preference::Load(). This function
	will then retrieve the last saved preference through the PrefServer, and
	update the Preference instance accordingly by modifying fields or adding
	fields to it.

	Although it is not required, it is recommended that you completely
	initialize the Preference with suitable default values. This will ensure
	that you always have values in the Preferences, so you don't need to check
	for their presence, and can always use ReplaceXXX().
	
	This function will return B_OK for success, and an error code if something
	went wrong. In the case of an error, you can still use the Preference, it
	may just not have been filled in completely, or at all.
*/

status_t Preference::Load()
{
	BMessage returnmes;
	char  *newname;
	uint32  newtype;
	int32   newcount;
	void	*newdata;
	ssize_t	newsize;
	status_t status;
	status_t ret=B_OK;

	PrepareTransfer();
	what=LOADPREF;

	// retrieve preference from server
	if(postman.IsValid())
	{
		status=postman.SendMessage(this,&returnmes,3E6,3E6);
		if(status!=B_OK)
		{
			ret=status;
			goto end;
		}
	}
	else
	{
		// try temporary setting first
		char *path=GetPrefsPathName(false);
		debug(("trying to read %s\n",path));
		BFile file(path,B_READ_ONLY);
		status=returnmes.Unflatten(&file);
		if(status!=B_OK)
		{
			// try the real thing
			path=GetPrefsPathName(true);
			debug(("trying to read %s\n",path));
			BFile file(path,B_READ_ONLY);
			status=returnmes.Unflatten(&file);
			if(status!=B_OK)
			{
				ret=status;
				goto end;
			}
		}
	}

	// process the reply-message (copy the fields from the server's reply 
	// into this Preference
	for(int i=0;;i++)
	{
		status=returnmes.GetInfo(B_ANY_TYPE, i, &newname, &newtype, &newcount);
		if(status==B_BAD_INDEX)
		{
			debug(("end of preferences reached at %d\n",i));
			ret=B_OK;
			goto end;			// done
		}
		if(status!=B_OK)
		{
			debug(("getinfo failed\n"));
			ret=B_ERROR;			// GetInfo failed
			goto end;
		}
		if(newcount==0)
		{
			debug(("empty preference\n"));
			ret=B_ERROR;			// No data
			goto end;
		}
		for(int j=0;j<newcount;j++)
		{
			if(B_OK==returnmes.FindData(newname,newtype,j,(const void**)&newdata,&newsize))
			{
				if(HasData(newname,newtype,j))
				{
					debug(("replacing data '%s'\n",newname));
					ReplaceData(newname,newtype,j,newdata,newsize);
				}
				else
				{
					debug(("adding data '%s'\n",newname));
					AddData(newname,newtype,newdata,newsize,false);
				}
			}
			else
			{
				// FindData failed where GetInfo succeeded. This is serious...
				debug(("finddata failed where getinfo succeeded\n"));
				ret=B_ERROR;
				goto end;
			}
		}
	}

end:
	FinishTransfer();
	debug(("load done\n"));
	return ret;
}

// convenience function: adjust the rect so that a window with
// this rect and the given window_look will fit on the screen.
// The rect will only be moved, not resized. If the rect
// is too large for the screen, it will extend off the
// bottom and the right (and touch the left and/or top).

void FitRectOnScreen(BRect *rect, window_look wl)
{
	BScreen screen;
	BRect frame=screen.Frame();

	float BT, TH;
	if(wl==B_MODAL_WINDOW_LOOK)
	{
		BT=5;
		TH=0;
	}
	else if(wl==B_FLOATING_WINDOW_LOOK)
	{
		BT=3;
		TH=15;
	}
	else if(wl==B_BORDERED_WINDOW_LOOK)
	{
		BT=1;
		TH=0;
	}
	else if(wl==B_NO_BORDER_WINDOW_LOOK)
	{
		BT=0;
		TH=0;
	}
	else
	{
		BT=5;
		TH=19;
	}
	
	if(rect->bottom+BT >= frame.bottom)
	{
		// off-screen to the bottom
		rect->OffsetBy(0,frame.bottom-(rect->bottom+BT));
	}
	if(rect->top-(BT+TH) <= frame.top)
	{
		// off-screen to the top
		rect->OffsetBy(0,frame.top-(rect->top-(BT+TH)));
	}
	if(rect->right+BT >= frame.right)
	{
		// off-screen to the right
		rect->OffsetBy(frame.right-(rect->right+BT),0);
	}
	if(rect->left-BT <= frame.left)
	{
		// off-screen to the left
		rect->OffsetBy(frame.left-(rect->left-BT),0);
	}
	return;
}

