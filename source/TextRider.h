#ifndef TEXTRIDER_H
#define TEXTRIDER_H

#include <File.h>
#include <Path.h>
#include "BinaryRider.h"

class TextWriter
{
	public:	
		TextWriter(BPath &fname);
		~TextWriter() {};
		
		// things we can write
		void WriteBool (bool b);
		void WriteChar(char c);
		void WriteString(const char *s);
		void WriteLn();
		brStatus Err() {return stat;};
		
	private:
		BFile file;
		brStatus stat;
};

#endif
