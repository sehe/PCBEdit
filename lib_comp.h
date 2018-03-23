

/*
	Library components
*/

struct LComp : Component
{
};

//Instantiation opens the global component index for lookup
//if no lib name is given

class CompLibrary
{
public:
	CompLibrary( /*lib name*/ )
	{
	}

	bool LoadPackage( SP_Component pC, LPCTSTR psName );
};

