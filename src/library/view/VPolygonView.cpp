/*

  INScore Project

  Copyright (C) 2009,2010  Grame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France
  research@grame.fr

*/

#include "VPolygonView.h"
#include "IPolygon.h"

namespace inscore
{

//----------------------------------------------------------------------
VPolygonView::VPolygonView(QGraphicsScene * scene, const IPolygon* h) 
	: VMappedShapeView( scene , new IQGraphicsPolygonItem(h) )
    {}

bool equals( const QPolygonF& p1 , const QPolygonF& p2 )
{
	if ( p1.size() != p2.size() )
		return false;

	for ( int i = 0 ; i < p1.size() ; i++ )
		if ( p1[i]!=p2[i] )
			return false;
	return true;
}

//----------------------------------------------------------------------
void VPolygonView::updateView( IPolygon * polygon)
{
    polygon->cleanupSync();
    QPolygon p;
	for ( unsigned int i = 0 ; i < polygon->getPoints().size() ; i++ )
		p << QPoint( relative2SceneWidth(polygon->getPoints()[i].first) , relative2SceneHeight(polygon->getPoints()[i].second) );
	if ( !equals(p, item()->polygon()) )
	{
		item()->setPolygon( p );
		itemChanged();
	}
	VShapeView::updateView( polygon );
}

//----------------------------------------------------------------------
void VPolygonView::updateObjectSize(IObject* o)
{
    IPolygon * p = dynamic_cast<IPolygon*>(o);
    if(p)
        updateView(p);
    VGraphicsItemView::updateObjectSize(o);
}

} // end namespoace