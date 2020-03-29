// cXml.cpp : Defines the entry point for the application.
//

#include "cXml.h"

using namespace std;

int main()
{
	cout << "cXml Demo" << endl << endl;

	/*load xml file, call fromFile(string) need to call free()*/
	/*Xml will return a shared_ptr for xml*/
	auto xml = Xml::fromFile( "test.xml");
	if (xml == nullptr) {
		cout << "xml read file failed" << endl;
		return 0;
	}

	/*print the raw xml string*/
	cout << xml->toXmlString()<< endl;
	auto res = Xml::parse(xml);//or call "xml->update()"
	if (res == false) {
		cout << "xml parse failed" << endl;
		return 0;
	}

	/*print the debug string*/
	cout << xml->toString() << endl;

	/*If get xml pointer come from fromFile(string), need to call free()*/
	xml->free();

	return 0;
}
