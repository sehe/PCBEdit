

// ....................................................
//
// these class maintain menus, accelerators, and toolbars
// along with their associated tooltips.
//

#pragma once

namespace MenuType
{
	AFX_STATIC_DATA LPCTSTR hotkeys[ ]={
		_T("Ctrl"),
		_T("Shft"),
		_T("Alt"),
	};

};

class Accelerator
{
};

class MenuItem
{
	Accelerator acc;

};
