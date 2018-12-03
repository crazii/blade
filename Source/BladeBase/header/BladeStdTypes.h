/********************************************************************
	created:	2010/09/08
	filename: 	BladeStdTypes.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeStdTypes_h__
#define __Blade_BladeStdTypes_h__
#include <BladeTypes.h>
#include <iostream>
#include <fstream>
#include <iomanip>

namespace std
{
#if BLADE_UNICODE

	static wostream& tcout	= wcout;
	static wistream& tcin	= wcin;
	static wostream& tcerr = wcerr;

	typedef wfstream	tfstream;
	typedef wofstream	tofstream;
	typedef wifstream	tifstream;
	typedef wstring		tstring;
	typedef wostream	tostream;
	typedef wistream	tistream;
	typedef wstringstream tsstream;

#else

	static ostream& tcout	= cout;
	static istream& tcin	= cin;
	static ostream& tcerr = cerr;

	typedef fstream		tfstream;
	typedef ofstream	tofstream;
	typedef ifstream	tifstream;
	typedef string		tstring;
	typedef ostream		tostream;
	typedef istream		tistream;
	typedef stringstream tsstream;

#endif

}//namespace std


#endif //__Blade_BladeStdTypes_h__