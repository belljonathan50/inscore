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

#include <iostream>
#include <sstream>

#include "IMessage.h"
#include "IMessageStream.h"
#include "rational.h"

#ifndef NO_OSCSTREAM
#include "ITLError.h"
#endif

using namespace std;
using namespace libmapping;

namespace inscore
{

//--------------------------------------------------------------------------
// messages list management
//--------------------------------------------------------------------------
void IMessageList::clear()
{
	for (unsigned int i=0; i < size(); i++)
		delete (*this)[i];
	vector<IMessage*>::clear();
}
 
void IMessageList::operator += (const IMessageList& msgs) 
{
	for (unsigned int i=0; i < msgs.size(); i++)
		push_back (msgs[i]);
}

//--------------------------------------------------------------------------
ostream& operator << (ostream& out, const IMessageList& msg)
{
	for (unsigned int i=0; i < msg.size(); i++) {
		msg[i]->print(out);
		out << ';' << endl;
	}
	return out;
}

//--------------------------------------------------------------------------
// IMessage implementation
//--------------------------------------------------------------------------
IMessage::IMessage(const std::string& address, const argslist& args, const TUrl& url)
				: fAddress(address), fHasMessage(true), fUrl(url.fHostname.c_str(), url.fPort)
{
	fArguments = args;
	fHasMessage = fArguments.size() && fArguments[0]->isType<string>();
}

//IMessage::IMessage(const IMessage& msg)
//{
//	setAddress (msg.address());
////	setMessage (msg.message());
//	fArguments = msg.params();
//}

//--------------------------------------------------------------------------
static string escape (const string& str)
{
	string out;
	const char *ptr = str.c_str();
	while (*ptr) {
		char c = *ptr++;
		if (c == '"')
			out += "\\\"";
		else out += c;
	}
	return out;
}

//--------------------------------------------------------------------------
static bool needQuotes (const string& str)
{
	const char *ptr = str.c_str();
	while (*ptr) {
		char c = *ptr++;
		if ((c == ' ') || (c == '\t') || (c == 0x0a) || (c == 0x0d))
			return true;
	}
	return false;
}

//--------------------------------------------------------------------------
void IMessage::setMessage(const string& msg)
{
	argPtr p = new IMsgParam<string>(msg);
	if (fHasMessage)
		fArguments[0] = p;
	else
		fArguments.insert (fArguments.begin(), p);
	fHasMessage = true;
}

//--------------------------------------------------------------------------
string IMessage::message() const
{
	if (!fHasMessage) return "";
	return fArguments[0]->value<string>("");
}

//--------------------------------------------------------------------------
void IMessage::add( const argslist& params )
{ 
	for (unsigned int i=0; i<params.size(); i++) {
		fArguments.push_back( params[i] );
	}
}

//--------------------------------------------------------------------------
bool IMessage::param(int i, rational& val) const
{ 
	if (size() < (i+2)) return false;
	int num, denum;
	if (!param(i, num))		return false;
	if (!param(i+1, denum)) return false;
	val = rational(num, denum);
	return true;
}

//--------------------------------------------------------------------------
void IMessage::print(std::ostream& out) const
{
	ios::fmtflags f = out.flags ( ios::showpoint );
	const char * msg = message().c_str();

	if (fUrl.fPort) out << fUrl.fHostname << ':' << fUrl.fPort;
	out << address() << " ";
	if (*msg) out << message() << " ";
	for (int i=0; i< size(); i++) {
		string str; int val; float fval;
		if (param(i, str)) {
			const char * q = needQuotes (str) ? "\"" : "";
			out << q << escape(str) << q << " ";
		}
		else if (param(i, val))
			out << val << " ";
		else if (param(i, fval))
			out << fval << " ";
	}
	out.flags ( f );

//	argslist::const_iterator i = params().begin();
//
//	ios::fmtflags f = out.flags ( ios::showpoint );
//	while (i != params().end()) {
//		IMsgParam<string>* s = dynamic_cast<IMsgParam<string>*>((baseparam*)(*i));
//		if (s) {
//			const char * q = needQuotes (s->getValue()) ? "\"" : "";
//			out << q << escape(s->getValue()) << q << " ";
//		}
//		IMsgParam<int>* ip = dynamic_cast<IMsgParam<int>*>((baseparam*)(*i));
//		if (ip) out << ip->getValue() << " ";
//		IMsgParam<float>* f = dynamic_cast<IMsgParam<float>*>((baseparam*)(*i));
//		if (f) out << f->getValue() << " ";
//		i++;
//	}
//	out.flags ( f );
}

//--------------------------------------------------------------------------
bool IMessage::operator == (const IMessage& other) const
{
	if ( address() != other.address() )
		return false;
	if ( message() != other.message() )
		return false;
	if ( size() != other.size() )
		return false;
	for ( unsigned int i = 0 ; i < size() ; i++ )
	{
		if (param(i) != other.param(i))
			return false;
	}
	return true;
}

//--------------------------------------------------------------------------
IMessage& operator << (IMessage& out, const rational& val)
{
	out.add(int(val.getNumerator()));
	out.add(int(val.getDenominator()));
	return out;
}

//--------------------------------------------------------------------------
IMessage& operator <<(IMessage& msg, const TFloatPoint& val)
{
	msg.add(val.x());
	msg.add(val.y());
	return msg; 
}

//--------------------------------------------------------------------------
IMessage& operator <<(IMessage& msg, const TIntSize& val)
{
	msg.add(val.width());
	msg.add(val.height()); 
	return msg; 
}

//--------------------------------------------------------------------------
IMessage& operator <<(IMessage& msg, const std::string& val)
{ 
	const char *ptr = val.c_str();
	bool quote = false;
	while (*ptr) {
		if (*ptr == ' ') {
			quote = true;
			break;
		}
		ptr++;
	}
	msg.add(val);
	return msg; 
}

//--------------------------------------------------------------------------
IMessage& operator << (IMessage& out, const IMessage* m)
{
	out << m->address() << m->message();
	for (int i = 0; i < m->size(); i++) {
		string strv; int iv; float fv;
		if (m->param(i, iv))	out << iv;
		else if (m->param(i, fv))	out << fv;
		else if (m->param(i, strv))	out << strv;
	}
	return out;
}

//--------------------------------------------------------------------------
std::ostream& operator << (std::ostream& out, const IMessage* m)
{
	m->print(out);
	return out;
}

} // end namespoace
