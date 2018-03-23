

bool ConnectWrt( Connect& con, XMLNODE& node );

bool ComponenttWrt( Component& con, XMLNODE& node );


class PCBWriter
{
public:
	PCBWriter( PCBoard& board, XMLNODE& node );
};

