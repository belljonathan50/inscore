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

#include <QDebug>
#include "MouseEventAble.h"
#include "VGraphicsItemView.h"
#include "MapTools.h"

#include "rational.h"
 
namespace inscore
{

// todo : cleanup (static function, point2date method ...)

//----------------------------------------------------------------------
static GraphicSegment find (const std::pair<float,float>& p, const Graphic2RelativeTimeRelation& rel)
{
	typedef TRelation<GraphicSegment,RelativeTimeSegment>::const_iterator const_iterator;
	for (const_iterator i = rel.begin(); i != rel.end(); i++)
		if ( i->first.include(p) ) return i->first;
	return GraphicSegment();
}

//----------------------------------------------------------------------
rational _MouseEventAble::point2date (const IObject * obj, float x, float y, const std::string& mapname, int n)
{
	const SRelativeTime2GraphicMapping&	mapping = obj->getMapping (mapname);
	if (!mapping) return rational (-1);
	const Graphic2RelativeTimeRelation& g2rt = mapping->reverse();
	std::pair<float,float> location (x*2 - 1, y*2 - 1);
	GraphicSegment seg = find (location, g2rt);
	if (seg.empty()) return rational (-1);
	
	rational rpos = MapTools::relativepos (location.first, seg.xinterval());

	std::set<RelativeTimeSegment> rels = g2rt.get (seg);
	if (!rels.size()) return rational (-1);
	
	std::set<RelativeTimeSegment>::const_iterator i = rels.begin();
	
	RelativeTimeSegment ts;
	while (i != rels.end()) {
		if (!n) {
			ts = *i;
			break;
		}
		i++;
	}
	if (ts.empty()) return rational (-1);
	return ts.start() + ts.size() * rpos;
}

//----------------------------------------------------------------------
static void originshift (const IObject * obj, float& relx, float& rely)
{
	float xoriginscaled = (obj->getXOrigin() + 1) / 2.;
	float yoriginscaled = (obj->getYOrigin() + 1) / 2.;
	relx = xoriginscaled - relx;
	rely = yoriginscaled - rely;
	if (relx < 0) relx = -relx;
	if (rely < 0) rely = -rely;
}

//----------------------------------------------------------------------
void _MouseEventAble::handleEvent (const IObject * obj, QPointF pos,  EventsAble::eventype type)
{
	VGraphicsItemView*	view = obj->graphicView();
	float x = pos.x();
	float y = pos.y();
	float w = view->relative2SceneWidth (obj->getWidth());
	float h = view->relative2SceneHeight(obj->getHeight());

	float xpos = obj->getXPos();
	float ypos = obj->getYPos();

	float clippedx = (x < 0) ? 0 : (x > w) ? w : x;
	float clippedy = (y < 0) ? 0 : (y > h) ? h : y;

	float relx = view->scene2RelativeX(clippedx) / obj->getWidth();
	float rely = view->scene2RelativeY(clippedy) / obj->getHeight();

	float sx = xpos + (obj->getWidth()  * obj->getScale()/2 * (relx * 2 - 1));
	float sy = ypos + (obj->getHeight() * obj->getScale()/2 * (rely * 2 - 1));

	MouseLocation mouse (relx, rely, x, y, sx, sy);
	originshift (obj, mouse.fx, mouse.fy);

//qDebug() << "handle event date" << date.getNumerator() << date.getDenominator() ;
	
	const std::vector<SEventMessage>& msgs = obj->getMouseMsgs (type);
	for (unsigned int i=0; i<msgs.size(); i++) {
		std::string mapname;
		int num=0, denum=0;
		rational date (-1,1);
		if (msgs[i]->hasDateVar (mapname, num, denum)) {
			date = point2date (obj, relx, rely, mapname, 0);
			if (num) {
				float fd = float(date);
				date.set (int(fd * denum / num) * num, denum);
			}
		}
		EventContext env(mouse, date, obj);
		msgs[i]->send(env);
	}
}

} // end namespoace
