#include "TextRider.h"
#include "strings.h"

// Writing routines

TextWriter::TextWriter(BPath &fname)
{
	stat = brNotOpen;
	if (file.SetTo(fname.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE) == B_OK) {
		stat = brOK;
	}
}
	
void TextWriter::WriteBool (bool b)
{
	if (b) file.Write("TRUE", 4);
	else file.Write("FALSE", 5);
}

void TextWriter::WriteChar(char c)
{
	file.Write(&c, sizeof(char));
}

void TextWriter::WriteString(const char *s)
{
	file.Write(s, strlen(s));
}

void TextWriter::WriteLn()
{
	file.Write("\n", 1);
}
