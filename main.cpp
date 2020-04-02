// cXml.cpp : Defines the entry point for the application.
//

#include "cXml.h"

using namespace std;

int main()
{
	cout << "cXml Demo" << endl << endl;

	/*load xml file, call fromFile(string) need to call free()*/
	/*Xml will return a shared_ptr for xml*/
	auto xml = Xml::fromFile("test.xml");
	if (xml == nullptr) {
		cout << "xml read file failed" << endl;
		return 0;
	}

	/*print the raw xml string*/
	cout << xml->toXmlString() << endl;
	auto res = Xml::parse(xml);//or call "xml->update()"
	if (res == false) {
		cout << "xml parse failed" << endl;
		return 0;
	}

	/*print the debug string*/
	cout << xml->toString() << endl;

	auto newXml = xml->root->toXml();// create xml fromString(), no need to call free()
	cout << newXml->toXmlString() << endl;

	/*If get xml pointer come from fromFile(string), need to call free()*/
	xml->free();

	{
		// create xml
		cout <<endl<< "* create xml *" << endl;
		auto tag = Xml::Tag::newTag("hello world");// vaild name: contain a space
		if (tag == nullptr) {
			cout << "- vaild name" << endl;
			tag = Xml::Tag::newTag("hellWorld");
		}
		tag->attributes["attr"] = "value";
		tag->addChild(Xml::Tag::newTag("tag0"));
		tag->addChild(Xml::Tag::newTag("tag1"));
		auto xmlFromTag = tag->toXml();
		cout << xmlFromTag->toXmlString() << endl;
	}

	return 0;
}
