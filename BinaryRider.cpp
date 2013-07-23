#include "BinaryRider.h"
#include "strings.h"

// Writing routines

void Writer::SetDefaultByteOrder (const void *x)
{
	char * w = (char *) x;
	if (w[0]==1) systemByteOrder=littleEndian;
	else systemByteOrder=bigEndian;
}

Writer::Writer(BPath &fname)
{
	int16 i = 1;
	stat = brNotOpen;
	if (file.SetTo(fname.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE) == B_OK) {
		stat = brOK;
		SetDefaultByteOrder(&i);
	}
}
	
void Writer::WriteBytesSwapped (const void *x, int32 n)
{
	char * w = (char *) x;		
	if (systemByteOrder==littleEndian) {
		file.Write(w, n);
	} else {
		for (int32 i = n-1; i>=0; i--) {
			file.Write(&w[i], 1);
		}
	}
}

void Writer::WriteBytes (const void *x, int32 start, int32 n)
{
	file.WriteAt(start, x, n);
}

void Writer::WriteBool (bool b)
{
	file.Write(&b, sizeof(bool));
}

void Writer::WriteChar(char c)
{
	file.Write(&c, sizeof(char));
}

void Writer::WriteNum(int32 i)
{
	while ((i<-64) || (i>63)) {
  		WriteChar((i & 0x7F) | 0x80);
      	i = i >> 7;
	}
    WriteChar(i & 0x7F);
}

void Writer::WriteReal(float f)
{
	WriteBytesSwapped(&f, sizeof(float));
}

void Writer::WriteLReal(double f)
{
	WriteBytesSwapped(&f, sizeof(double));
}

void Writer::WriteString(const char *s)
{
	file.Write(s, strlen(s)+1);
}


// Read routines

void Reader::SetDefaultByteOrder (const void *x)
{
	char * w = (char *) x;
	if (w[0]==1) systemByteOrder=littleEndian;
	else systemByteOrder=bigEndian;
}

Reader::Reader(BPath &fname)
{
	int16 i = 1;
	stat = brNotOpen;
	if (file.SetTo(fname.Path(), B_READ_ONLY) == B_OK) {
		stat = brOK;
		SetDefaultByteOrder(&i);
	}
}
	
void Reader::ReadBytes (void *x, int32 start, int32 n)
{
	if (file.ReadAt(start, x, n) != n) stat = brReadAfterEnd;
}

void Reader::ReadBytesSwapped (void *x, int32 n)
{
	char * w = (char *) x;	
	if (systemByteOrder==littleEndian) {
		if (file.Read(w, n) != n) stat = brReadAfterEnd;
	} else {
		for (int32 i = n-1; i>=0; i--) {
			if (file.Read(&w[i], 1) == 0) stat = brReadAfterEnd;
		}
	}
}

void Reader::ReadBool (bool &b)
{
	if (file.Read(&b, sizeof(bool)) == 0) {		
		stat = brReadAfterEnd;
	}
}

void Reader::ReadChar(char &c)
{
	if (file.Read(&c, sizeof(char)) == 0) {
		c = 0; stat = brReadAfterEnd;
	}
}

void Reader::ReadNum(int32 &i)
{
	int16 s;
	char x;
	uchar x1; 
	int32 n;
 
    s=0; n=0; ReadChar(x); x1 = x;
    while (x1 >= 128) {
      n+= ((long)x1 -128) << s; s+=7; 
      ReadChar(x); x1 = x;
    }
 	i=n + ((((long)x1 & 0x3F) - ((long)x1>>6) * 64) << s);
}

void Reader::ReadReal(float &f)
{
	ReadBytesSwapped(&f, sizeof(float));
}

void Reader::ReadLReal(double &f)
{
	ReadBytesSwapped(&f, sizeof(double));
}

void Reader::ReadString(char *s)
{
	int32 cnt = -1;

    do {
      cnt++; ReadChar(s[cnt]);
    } while ((s[cnt] != 0) && (stat == brOK));
    if ((s[cnt] != 0) & (stat == brOK)) {
      stat = brInvalidData;
      s[cnt]=0;
    }
}
