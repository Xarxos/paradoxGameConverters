/*Copyright (c) 2016 The Paradox Game Converters Project

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/


/*Copyright (c) 2010 Rolf Andreassen

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/



#include "Object.h"
#include "ParadoxParser.h"
#include <sstream> 
#include <fstream>
#include <algorithm>
#include <iostream>
#include <assert.h>



Object::Object(wstring k) :
objects(),
strVal(),
leaf(false),
isObjList(false)
{
	key = k;
}


Object::~Object() {
	for (objiter i = objects.begin(); i != objects.end(); ++i)
	{
		delete (*i);
	}
	if (br == this)
	{
		br = 0;
	}
}


Object::Object(Object* other) :
objects(),
strVal(other->strVal),
leaf(other->leaf),
isObjList(other->isObjList)
{
	key = other->key;
	for (vector<Object*>::iterator i = other->objects.begin(); i != other->objects.end(); ++i)
	{
		objects.push_back(new Object(*i));
	}
}


void Object::setValue(wstring val)
{
	strVal = val;
	leaf = true;
}


void Object::setValue(Object* val)
{
	objects.push_back(val);
	leaf = false;
}


void Object::unsetValue(wstring val)
{
	for (unsigned int i = 0; i < objects.size(); ++i)
	{
		if (objects[i]->getKey() != val)
		{
			continue;
		}
		objects[i] = objects.back();
		objects.pop_back();
	}
}


void Object::setLeaf(wstring key, wstring val)
{
	Object* leaf = new Object(key);	// an object to hold the leaf
	leaf->setValue(val);
	setValue(leaf);
}


void Object::setValue(vector<Object*> val)
{
	objects = val;
}


void Object::addToList(wstring val)
{
	isObjList = true;
	if (strVal.size() > 0)
	{
		strVal += L" \"";
	}
	else
	{
		strVal += L"\"";
	}
	strVal += val;
	strVal += L"\"";
	tokens.push_back(val);
}


void Object::addToList(vector<wstring>::iterator begin, vector<wstring>::iterator end)
{
	isObjList = true;
	for (vector<wstring>::iterator itr = begin; itr != end; ++itr)
	{
		if (strVal.size() > 0)
		{
			strVal += L"\" \"";
		}
		else
		{
			strVal += L"\"";
		}
		strVal += *itr;
	}
	strVal += L"\"";
	tokens.insert(tokens.end(), begin, end);
}


vector<Object*> Object::getValue(wstring key) const
{
	vector<Object*> ret;	// the objects to return
	for (vector<Object*>::const_iterator i = objects.begin(); i != objects.end(); ++i)
	{
		if ((*i)->getKey() != key)
		{
			continue;
		}
		ret.push_back(*i);
	}
	return ret;
}


wstring Object::getToken(const int index)
{
	if (!isObjList)
	{
		return L"";
	}
	if (index >= (int)tokens.size())
	{
		return L"";
	}
	if (index < 0)
	{
		return L"";
	}
	return tokens[index];
}


int Object::numTokens()
{
	if (!isObjList)
	{
		return 0;
	}
	return tokens.size();
}


vector<wstring> Object::getKeys()
{
	vector<wstring> ret;	// the keys to return
	for (vector<Object*>::iterator i = objects.begin(); i != objects.end(); ++i)
	{
		wstring curr = (*i)->getKey();	// the current key
		if (find(ret.begin(), ret.end(), curr) != ret.end())
		{
			continue;
		}
		ret.push_back(curr);
	}
	return ret;
}


wstring Object::getLeaf(wstring leaf) const
{
	vector<Object*> leaves = getValue(leaf); // the objects to return
	if (0 == leaves.size())
	{
		cout << "Error: Cannot find leaf " << leaf << " in object " << endl << *this;
		assert(leaves.size());
	}
	return leaves[0]->getLeaf();
}


wostream& operator<< (wostream& os, const Object& obj)
{
	static int indent = 0; // the level of indentation to output to
	for (int i = 0; i < indent; i++)
	{
		os << "\t";
	}
	if (obj.leaf) {
		os << obj.key << "=" << obj.strVal << "\n";
		return os;
	}
	if (obj.isObjList)
	{
		os << obj.key << "={" << obj.strVal << " }\n";
		return os;
	}

	if (&obj != getTopLevel())
	{
		os << obj.key << "=\n";
		for (int i = 0; i < indent; i++)
		{
			os << "\t";
		}
		os << "{\n";
		indent++;
	}
	for (vector<Object*>::const_iterator i = obj.objects.begin(); i != obj.objects.end(); ++i)
	{
		os << *(*i);
	}
	if (&obj != getTopLevel())
	{
		indent--;
		for (int i = 0; i < indent; i++)
		{
			os << "\t";
		}
		os << "}\n";
	}
	return os;
}


ostream& operator<< (ostream& os, const Object& obj)
{
	static int indent = 0; // the level of indentation to output to
	for (int i = 0; i < indent; i++)
	{
		os << "\t";
	}
	if (obj.leaf) {
		os << obj.key << "=" << obj.strVal << "\n";
		return os;
	}
	if (obj.isObjList)
	{
		os << obj.key << "={" << obj.strVal << " }\n";
		return os;
	}

	if (&obj != getTopLevel())
	{
		os << obj.key << "=\n";
		for (int i = 0; i < indent; i++)
		{
			os << "\t";
		}
		os << "{\n";
		indent++;
	}
	for (vector<Object*>::const_iterator i = obj.objects.begin(); i != obj.objects.end(); ++i)
	{
		os << *(*i);
	}
	if (&obj != getTopLevel())
	{
		indent--;
		for (int i = 0; i < indent; i++)
		{
			os << "\t";
		}
		os << "}\n";
	}
	return os;
}


void Object::keyCount()
{
	if (leaf)
	{
		cout << key << " : 1\n";
		return;
	}

	map<wstring, int> refCount;	// the count of the references
	keyCount(refCount);
	vector<pair<wstring, int> > sortedCount; // an organized container for the counts
	for (auto i = refCount.begin(); i != refCount.end(); ++i)
	{
		pair<wstring, int> curr((*i).first, (*i).second);
		if (2 > curr.second)
		{
			continue;
		}
		if ((0 == sortedCount.size()) || (curr.second <= sortedCount.back().second))
		{
			sortedCount.push_back(curr);
			continue;
		}

		for (vector<pair<wstring, int> >::iterator j = sortedCount.begin(); j != sortedCount.end(); ++j)
		{
			if (curr.second < (*j).second)
			{
				continue;
			}
			sortedCount.insert(j, 1, curr);
			break;
		}
	}

	for (vector<pair<wstring, int> >::iterator j = sortedCount.begin(); j != sortedCount.end(); ++j)
	{
		cout << (*j).first << " : " << (*j).second << "\n";
	}
}


void Object::keyCount(map<wstring, int>& counter)
{
	for (vector<Object*>::iterator i = objects.begin(); i != objects.end(); ++i)
	{
		counter[(*i)->key]++;
		if ((*i)->leaf)
		{
			continue;
		}
		(*i)->keyCount(counter);
	}
}


void Object::printTopLevel()
{
	for (vector<Object*>::iterator i = objects.begin(); i != objects.end(); ++i)
	{
		cout << (*i)->key << endl;
	}
}


void Object::removeObject(Object* target)
{
	vector<Object*>::iterator pos = find(objects.begin(), objects.end(), target);	// the position of the object to be removed
	if (pos == objects.end())
	{
		return;
	}
	objects.erase(pos);
}


void Object::addObject(Object* target)
{
	objects.push_back(target);
}


void Object::addObjectAfter(Object* target, wstring key)
{
	vector<Object*>::iterator i;

	for (i = objects.begin(); i != objects.end(); ++i)
	{
		if ((*i)->getKey() == key)
		{
			objects.insert(i, target);
			break;
		}
	}

	if (i == objects.end())
	{
		objects.push_back(target);
	}
}



Object* br = 0;	// the branch being set
void setVal(wstring name, const wstring val, Object* branch)
{
	if ((branch) && (br != branch))
	{
		br = branch;
	}
	Object* b = new Object(name);	// the new object to add to the branch
	b->setValue(val);
	br->setValue(b);
}


void setInt(wstring name, const int val, Object* branch)
{
	if ((branch) && (br != branch))
	{
		br = branch;
	}
	static wchar_t strbuffer[1000];	// the text to add to the branch
	swprintf_s(strbuffer, 1000, L"%i", val);
	Object* b = new Object(name);	// the new object to add to the branch
	b->setValue(strbuffer);
	br->setValue(b);
}


void setFlt(wstring name, const double val, Object* branch)
{
	if ((branch) && (br != branch))
	{
		br = branch;
	}
	static wchar_t strbuffer[1000];	// the text to add to the branch
	swprintf_s(strbuffer, 1000, L"%.3f", val);
	Object* b = new Object(name);	// the new object to add to the branch
	b->setValue(strbuffer);
	br->setValue(b);
}

double Object::safeGetFloat(wstring k, const double def)
{
	objvec vec = getValue(k);	// the objects with the keys to be returned
	if (0 == vec.size()) return def;
	return _wtof(vec[0]->getLeaf().c_str());
}

wstring Object::safeGetString(wstring k, wstring def)
{
	objvec vec = getValue(k);	// the objects with the strings to be returned
	if (0 == vec.size())
	{
		return def;
	}
	return vec[0]->getLeaf();
}

int Object::safeGetInt(wstring k, const int def)
{
	objvec vec = getValue(k);	// the objects with the ints to be returned
	if (0 == vec.size())
	{
		return def;
	}
	return _wtoi(vec[0]->getLeaf().c_str());
}

Object* Object::safeGetObject(wstring k, Object* def)
{
	objvec vec = getValue(k);	// the objects with the objects to be returned 
	if (0 == vec.size())
	{
		return def;
	}
	return vec[0];
}


wstring Object::toString() const
{
	wostringstream blah;	// the output string
	blah << *(this);
	return blah.str();
}

