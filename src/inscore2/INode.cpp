
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <regex>

#include "INode.h"
#include "oscprinter.h"
#include "TreeTools.h"

using namespace std;

namespace inscore2
{

//------------------------------------------------------------
ostream& operator<< (ostream& os, const SINode n)	{ n->print(os); return os; }
ostream& operator<< (ostream& os, const INode* n)	{ n->print(os); return os; }
ostream& operator<< (ostream& os, const INode& n)	{ n.print(os); return os; }

//------------------------------------------------------------
void NList::add (const SINode& n)
{
	push_back (n);
//	if (n->getType() == INode::kList)
//		add (n->childs());
//	else push_back (n);
}
//------------------------------------------------------------
void NList::add (const NList& l)
{
//	for (auto n: l) add(n);
	for (auto n: l) push_back(n);
}

//------------------------------------------------------------
const SINode NList::find (const SINode& node) const
{
	for (auto n: *this) {
		if (*n == node) return n;
	}
	return 0;
}


//------------------------------------------------------------
const NList NList::but (const SINode& node) const
{
	NList out;
	for (auto n: *this)
		if (*n != node) out.push_back (n);
	return out;
}

//------------------------------------------------------------
void INode::print (ostream& os, int depth) const
{
	oscprinter::print (os, this);
}

//------------------------------------------------------------
INode* INode::createSeq (SINode n1, SINode n2)	{ return new SeqNode(n1, n2); }
INode* INode::createPar (SINode n1, SINode n2)	{ return new ParNode(n1, n2); }

//------------------------------------------------------------
INode::INode(const INode* node)
	: fName(node->getName()), fType(node->getType()), fAddressPart(node->address()), fLine(node->getLine()), fColumn(node->getColumn())
{
	setEnv (node->getEnv());
	for (auto n: node->childs()) fSubNodes.push_back(n->clone());
}

//------------------------------------------------------------
//INode::INode(const INode* node, TNodeType t)
//	: fName(node->getName()), fType(t), fAddressPart(node->address()), fLine(node->getLine()), fColumn(node->getColumn())
//{
//	setEnv (node->getEnv());
//	for (auto n: node->childs()) fSubNodes.push_back(n->clone());
//}

//------------------------------------------------------------
SINode INode::clone (const NList& sub) const
{
	SINode out = SINode(new INode(getName(), sub, getType()));
	out->setAddress( address() );
	out->setEnv (getEnv());
	out->setLC (fLine, fColumn);
	return out;
}

//------------------------------------------------------------
SINode INode::clone () const
{
	NList l;
	for (auto n: childs()) l.push_back (n->clone());
	SINode out = SINode(new INode (fName, l, fType));
	out->setAddress( address() );
	out->setEnv (getEnv());
	out->setLC (fLine, fColumn);
	return out;
}

//------------------------------------------------------------
SINode  INode::merge (const SINode& node) const
{
	NList l;
	for (auto n: childs()) {
		SINode e = node->find(n);
		if (e) l.push_back ( n->merge (e));
		else l.push_back (n->clone());
	}
	for (auto n: node->childs()) {
		SINode e = find(n);
		if (!e) l.push_back (n->clone());
	}
	return clone (l);
}

//------------------------------------------------------------
// creates an environment for the form $var(arg1, ... argn)
//------------------------------------------------------------
//void INode::setEnv (const SINode& node)
//{
//	if (node->getType() == kForest) {
//		int i = 1;
//		for (auto n: node->childs())
//			fEnv.put (to_string(i++), n);
//	}
//	else
//		fEnv.put ("1", node);
//}

void INode::addEnv (const SINode& node)
{
	fEnv += node->getEnv();
//	for (auto e: node->getEnv()) {
//		setEnv (e.first, e.second);
//	}
}

//------------------------------------------------------------
void INode::setEnv (const TEnv& env) 					{ fEnv = env; }
void INode::setEnv (const std::string& name, int val)	{ fEnv.put (name, create(to_string(val), kInt)); }

//------------------------------------------------------------
bool INode::petiole () const
{
	if (empty()) return false;
	for (auto n: childs())
		if (!n->empty()) return false;
	return true;
}

//------------------------------------------------------------
const char * INode::type2string (TNodeType t)
{
	switch (t) {
		case kACos: 	return "ACos";
		case kACosh: 	return "ACosh";
		case kAdd: 		return "Add";
		case kASin: 	return "ASin";
		case kASinh: 	return "ASinh";
		case kATan: 	return "ATan";
		case kATanh: 	return "ATanh";
		case kCbrt: 	return "Cbrt";
		case kCeil: 	return "Ceil";
		case kCos: 		return "Cos";
		case kCosh: 	return "Cosh";
		case kDiv: 		return "Div";
		case kEq: 		return "Eq";
		case kExp: 		return "Exp";
		case kExpand: 	return "Expand";
		case kFloat: 	return "Float";
		case kFloor: 	return "Floor";
		case kForest: 	return "Forest";
		case kGreater: 	return "Greater";
		case kGreatereq: return "Greatereq";
		case kHas: 		return "Has";
		case kInt: 		return "Int";
		case kJScript: 	return "JS";
		case kLess: 	return "Less";
		case kLesseq: 	return "Lesseq";
		case kLog: 		return "Log";
		case kLog2: 	return "Log2";
		case kLog10: 	return "Log10";
		case kMax: 		return "Max";
		case kMin: 		return "Min";
		case kModulo: 	return "Modulo";
		case kMult: 	return "Mult";
		case kNeg: 		return "Neg";
		case kPar: 		return "Par";
		case kPow: 		return "Pow";
		case kQuest: 	return "Quest";
		case kRand: 	return "Rand";
		case kRegexp: 	return "Regexp";
		case kRound: 	return "Round";
		case kSeq: 		return "Seq";
		case kSin: 		return "Sin";
		case kSinh: 	return "Sinh";
		case kSlash: 	return "Slash";
		case kSqrt: 	return "Sqrt";
		case kSub: 		return "Sub";
		case kTan: 		return "Tan";
		case kTanh: 	return "Tanh";
		case kText: 	return "Text";
		case kVariable: return "Variable";
	}
	return "unknown";
}

} // end namespace