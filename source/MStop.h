#ifndef _MSTOP_H
#define _MSTOP_H

#include "MPictureButton.h"

#if __POWERPC__
#pragma simple_class_byval off
#endif

class IMPEXPLIBLAYOUT MStop : public MPictureButton
{
	public:		MStop(BHandler*);
				MStop(BHandler *id, BMessage*);
				MStop(BMessage*);
				virtual ~MStop();
				virtual status_t Archive(BMessage *archive, bool deep=true) const;
				static BArchivable *Instantiate(BMessage *archive);
				virtual void MakePictures();
};
extern const IMPEXPLIBLAYOUT char M_BUTTON_POINTER[];

#endif
