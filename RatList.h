#pragma once

/*
	Ratlist holds the associations of pin connections per net item.
	This is only used for drawing/editing perposses
*/
struct ConnectNetComp
{
	bool operator( ) ( const SP_Connect& s1, const SP_Connect& s2) const
	{
		return s1->net < s2->net;
	}
};

typedef boost::container::flat_multiset< SP_Connect, ConnectNetComp > ratlist_t;

inline void LoadRatList( PCBoard& board, ratlist_t& list )
{
	for( auto it : board.components )
	{
		for( auto cit : it->vsp_pins )
			list.insert( cit );
	}
}
