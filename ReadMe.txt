


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The plan so far......
First goal
	Keep MFC separate from the project objects as much as possible. Note and use specialized methods for the interface.

PCBoard
object containing all the information for one PC board project.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
6/29/13
parse a PCB file (.pcb from said application)
	Parser lives separate from board object as there may be more than one. This first one is
	a hand made hack, it is not crash proof. Made to get some working data into the board.

	I'm using the first project I found at:
	http://www.delorie.com/pcb/
	bldc.pcb

	7000 lines of well formed data
~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
todo document AFX_WM_PROPERTY_CHANGED

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
3/27/14
That the drill file should output as Gerber per Ucamco.
But it should be optional to output as dfx.
~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
1/1/2017
the mfc file: afxwinappex.h is in the project directory as reference for the development of the store
documentation on afx_store.txt


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//other code.........
/*
//this function should remain totally unnecessary.
int my_atoi( CString * in_str )
{
	CString str = *in_str;
	BOOL mil_units = FALSE;
	BOOL mm_units = FALSE;
	int len = str.GetLength();
	if( len > 3 && ( str.Right(3) == "MIL" || str.Right(3) == "mil" ) )
	{
		mil_units = TRUE;
		str = str.Left(len-3);
		len = len-3;
	}
	else if( len > 2 && ( str.Right(2) == "MM" || str.Right(2) == "mm" ) )
	{
		mm_units = TRUE;
		str = str.Left(len-2);
		len = len-2;
	}
	int test = _tstoi( str );
	if( test == 0 )
	{
		for( int i=0; i<len; i++ )
		{
			TCHAR c = str[i];
			if( !( c == '+' || c == '-' || ( c >= '0' ) ) )
			{
				CString * err_str = new CString;
				err_str->Format( _T("my_atoi() unable to convert string \"%s\" to int"), *in_str );
				throw err_str;
			}
		}
	}
	if( mil_units )
		test = test * 25400;
	else if( mm_units )
		test = test * 1000000;
	return test;
}

*/



