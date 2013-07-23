#ifndef BINARYRIDER_H
#define BINARYRIDER_H

#include <BeBuild.h>

#ifndef IMPEXP_LIBREALS
#ifdef BUILDING_LIBREALS
#define IMPEXP_LIBREALS _EXPORT
#else
#define IMPEXP_LIBREALS _IMPORT
#endif
#endif

#include <File.h>
#include <Path.h>

#if __POWERPC__
#pragma simple_class_byval off
#endif

enum brStatus {brOK, brInvalidData, brReadAfterEnd, brNotOpen};
enum brByteOrder {bigEndian, littleEndian};

class IMPEXP_LIBREALS Writer
{
	public:	
		Writer(BPath &fname);
		~Writer() {};
		
		// things we can write
		void WriteBytes (const void *x, int32 start, int32 n);
		void WriteBool (bool b);
		void WriteChar(char c);
		void WriteNum(int32 i);
		void WriteReal(float f);
		void WriteLReal(double f);
		void WriteString(const char *s);
		brStatus Err() {return stat;};
		
	private:
		BFile file;
		brStatus stat;
		brByteOrder systemByteOrder;
		void SetDefaultByteOrder (const void *x);
		void WriteBytesSwapped (const void *x, int32 n);
};

class IMPEXP_LIBREALS Reader
{
	public:	
		Reader(BPath &fname);
		~Reader() {};
		
		// things we can read
		void ReadBytes (void *x, int32 start, int32 n);
		void ReadBool (bool &b);
		void ReadChar(char &c);
		void ReadNum(int32 &i);
		void ReadReal(float &f);
		void ReadLReal(double &f);
		void ReadString(char *s);
		brStatus Err() {return stat;};
		
	private:
		BFile file;
		brStatus stat;
		brByteOrder systemByteOrder;
		void SetDefaultByteOrder (const void *x);
		void ReadBytesSwapped (void *x, int32 n);
};

#endif
