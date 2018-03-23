
//universal netlist import conrainer, just TinyCAD for now...

struct NetListImport
{
	Netlist netlist;
	SSP_Component components;
};

bool TinyCADNetlistImport( CXML& xml, NetListImport& netlist );

