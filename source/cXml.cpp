#include "cXml.h"

std::map<std::string/*path*/, std::shared_ptr<Xml>> Xml::_fileCache;

inline static bool xmlIsSpace(const std::string::const_iterator& iter) {
	return *iter == ' ' || *iter == '\n' || *iter == '\t';
}

inline static bool xmlNameCheck(const Xml::StringView& stringView) {
	// TODO: check the name whether vaild
	// can't begin with Number or Punctuation
	// can't begin with "xml" string(eg. "XML", "Xml")
	// can't contain spaces
	return true;
}

inline static std::string& removeEndSpaces(std::string& str) {
	for (auto iter = --str.end(); iter != str.begin(); iter--) {
		if (xmlIsSpace(iter))iter = str.erase(iter);
		else return str;
	}
	return str;
}

inline static std::string& removeEndSpaces(std::string&& str) {
	for (auto iter = --str.end(); iter != str.begin(); iter--) {
		if (xmlIsSpace(iter))iter = str.erase(iter);
		else return str;
	}
	return str;
}

// example0:
// <[name] attr="value" />
//        ^             ^
//        begin         end

// example1:
// <[name] attr="value" >
//        ^             ^
//        begin         end
static bool getAttribute(std::shared_ptr<Xml::Tag>& tag, const std::string::const_iterator& begin, const std::string::const_iterator& end) {
	for (auto iter = begin; iter != end; iter++) {
		while (xmlIsSpace(iter)) {
			if (++iter == end)return true;
		}
		if (*iter == '=')return ParseFail;// this attr without name
		// <[name] attr="value" >
		//         ^
		//         iter
		auto attrBegin = iter;
		while (!xmlIsSpace(iter) && *iter != '=') {
			if (++iter == end)return ParseFail;//error: attr format missing "="
		}
		// <[name] attr = "value" >
		//             ^
		//             iter
		auto attrEnd = iter;
		while (*iter != '=') {
			if (++iter == end)return ParseFail;//error: attr format missing "="
		}
		// <[name] attr = "value" >
		//              ^
		//              iter
		while (true) {
			if (++iter == end)return ParseFail;//error: attr format missing attr value
			if (*iter == '/') {
				return ParseFail;
			}
			else if (*iter == '>') {
				return ParseFail;
			}
			else if (*iter == '\"') {
				// <[name] attr = "value" >
				//                ^
				//                iter
				auto attrValueBegin = ++iter;
				// <[name] attr = "value" >
				//                 ^
				//                 iter
				while (*iter != '\"') {
					if (++iter == end)return ParseFail;//error: attr format missing close scope "
				}
				// <[name] attr = "value" >
				//                      ^
				//                      iter
				auto attrValueEnd = iter;
				tag->attributes[std::string(attrBegin, attrEnd)] = Xml::StringView(attrValueBegin, attrValueEnd);
				iter++;
				if (iter == end || *iter == '/' || *iter == '>')return true;
				if (!xmlIsSpace(iter))return ParseFail;//error: attr format missing space between two attr(<[name] attr0="value"attr1="value">)
				break;
			}
			else if (*iter == '\'') {
				auto attrValueBegin = ++iter;
				while (*iter != '\'') {
					if (++iter == end)return ParseFail;//error: attr format missing close scope "
				}
				auto attrValueEnd = iter;
				tag->attributes[std::string(attrBegin, attrEnd)] = Xml::StringView(attrValueBegin, attrValueEnd);
				iter++;
				if (iter == end || *iter == '/' || *iter == '>')return true;
				if (!xmlIsSpace(iter))return ParseFail;//error: attr format missing space between two attr(<[name] attr0="value"attr1="value">)
				break;
			}
		}
	}
	return true; // this tag has no attr
}

/*
parse extern tag <? ... ?>
*/
static bool parseExtern(const std::shared_ptr<Xml>& xml, std::shared_ptr<Xml::Tag>& tag, const std::string::const_iterator& begin, std::string::const_iterator& end) {
	const auto stringEnd = xml->toXmlString().end();
	auto current = begin;
	if (++current == stringEnd)return ParseFail;
	if (++current == stringEnd)return ParseFail;

	//<? [name] attr="value"?>
	//  ^
	//  current
	while (xmlIsSpace(current)) {
		if (*current == '?' || current == xml->toXmlString().end())return ParseFail;//error: xml format[tag without name]
		if (++current == stringEnd)return ParseFail;
	}
	//<? [name] attr="value"?>
	//   ^
	//   current
	auto nbegin = current;
	while (!xmlIsSpace(current) && *current != '?') {
		if (*current == '>' || current == xml->toXmlString().end())return ParseFail;//error: xml format[extern tag not end up with "?>"]
		if (++current == stringEnd)return ParseFail;
	}
	//<? [name] attr="value"?>
	//         ^
	//         current

	//<? [name]?>
	//         ^
	//         current
	auto nend = current;
	tag->name = Xml::StringView(nbegin, nend);
	// extern tags don't need to check name
	// if (!xmlNameCheck(tag->name))return false;

	while (*current != '?') {
		if (*current == '>')return ParseFail;//error: xml format[extern tag not end up with "?>"]
		if (!xmlIsSpace(current)) {// get attribute
			//<?[name] attr="value"?>
			//         ^
			//         current
			auto attributeBegin = current;
			while (*current != '=') {
				if (*current == '?' || *current == '>')return ParseFail;//error: xml format[attribute without "="]
				if (++current == stringEnd)return ParseFail;
			}
			//<?[name] attr="value"?>
			//             ^
			//             current
			auto attributeEnd = current;
			while (*current != '\"' && *current != '\'') {
				if (*current == '?' || *current == '>')return ParseFail;//error: xml format[extern tag not end up with "?>"]
				if (++current == stringEnd)return ParseFail;
			}
			//<?[name] attr="value"?>
			//              ^
			//              current
			auto attributeValueBegin = current + 1;
			if (*current == '\"') {
				if (++current == stringEnd)return ParseFail;
				while (*current != '\"') {
					if (*current == '?' || *current == '>')return ParseFail;//error: xml format[extern tag attribute not close with "]
					if (++current == stringEnd)return ParseFail;
				}
			}
			else {
				if (++current == stringEnd)return ParseFail;
				while (*current != '\'') {
					if (*current == '?' || *current == '>')return ParseFail;//error: xml format[extern tag attribute not close with ']
					if (++current == stringEnd)return ParseFail;
				}
			}
			//<?[name] attr="value"?>
			//                    ^
			//                    current
			auto attributeValueEnd = current;
			tag->attributes[removeEndSpaces(std::string(attributeBegin, attributeEnd))] = Xml::StringView(attributeValueBegin, attributeValueEnd);
		}
		if (++current == stringEnd)return ParseFail;
	}
	//<? [name] attr="value"?>
	//                      ^
	//                      current
	if (++current == stringEnd)return ParseFail;
	//<? [name] attr="value"?>
	//                       ^
	//                       current
	if (*current == '>') {
		end = current + 1;
		//<? [name] attr="value"?>
		//                        ^
		//                        end
		return true;
	}

	return ParseFail;//error: xml format[extern tag not end up with "?>"]
}
/*
parse normal tag </>

example:
	< ... />
	^       ^
	begin   end
*/
static bool parseNode(const std::shared_ptr<Xml>& xml, std::shared_ptr<Xml::Tag>& tag, const std::string::const_iterator& begin, std::string::const_iterator& end) {
	// <	[name] attr="value" >
	// ^
	// begin
	const auto stringEnd = xml->toXmlString().end();
	auto current = begin + 1;
	if (current == stringEnd)return ParseFail;
	// <	[name] attr="value" >
	//  ^
	//  current
	while (xmlIsSpace(current)) {
		if (*current == '/' || *current == '>')return ParseFail;//error: xml format[tag without name]
		if (++current == stringEnd)return ParseFail;
	}

	// <	[name] attr="value" >
	//      ^
	//      current
	auto nbegin = current;
	while (!xmlIsSpace(current)) {
		if (current == stringEnd)return ParseFail;
		if (*current == '/') {
			if (++current == stringEnd)return ParseFail;
			else if (*current == '>') {
				tag->name = Xml::StringView(nbegin, current - 1);
				if (!xmlNameCheck(tag->name))return ParseFail;
				// the tag in format: <[name]/>
				end = ++current;
				return true;
			}
			else
				return ParseFail;//error: xml format[tag without end up with "/>"]
		}
		else if (*current == '>') {
			break;//format < ... >
		}
		if (++current == stringEnd)return ParseFail;
	}
	// There may be a few space between [name] and end of the tag(">" or "/>")
	auto nend = current;
	tag->name = Xml::StringView(nbegin, nend);
	if (!xmlNameCheck(tag->name))return ParseFail;
	// <[name] attr="value" >
	//        ^
	//        current

	// skip to end of tag (">" or "/>")
	while (true) {
		if (*current == '>') {
			break;
		}
		if (++current == stringEnd)return ParseFail;//error: xml format[tag without end up with "/>"]
		if (*current == '\"') {
			do {
				if (++current == stringEnd)return ParseFail;
			} while (*current != '\"');
		}
		else if (*current == '\'') {
			do {
				if (++current == stringEnd)return ParseFail;
			} while (*current != '\'');
		}
		else if (*current == '/') {
			auto res = getAttribute(tag, nend, current);
			if (!res)return ParseFail;
			if (++current == stringEnd)return ParseFail;
			if (*current == '>') {
				// the tag in format: <[name] attr="attrValue"/>
				end = ++current;
				return true;
			}
			else
				return ParseFail;//error: xml format[tag without end up with "/>"]
		}
	}
	// tag is the "begin" tag (< ... >), need to find the end tag (</ ... >)
	// <[name] attr="value" >
	//        ^             ^
	//        nend          current
	auto res = getAttribute(tag, nend, current);
	if (!res)return ParseFail;
	if (++current == stringEnd)return ParseFail;//error: xml format[tag without end tag]

	while (true) {
		while (xmlIsSpace(current)) {
			if (++current == stringEnd)return ParseFail;
		}
		if (*current == '<') {
			if ((current + 1) == stringEnd)return ParseFail;//error: xml format[tag without end tag]
			if (*(current + 1) == '/') {
				//example0:
				// </[name]>
				// ^
				// current

				// find end tag (</ ... >)
				// TODO: check the end tag name is matched the begin tag name
				while (*current != '>') {
					if (++current == stringEnd)return ParseFail;//error: xml format[tag without end up with ">"]
				}
				end = ++current;
				return true;
			}
			else {
				// tag with subTag
				//< ... >
				//		< ... />
				//		< ... > ... </ ... >
				//		...
				//		< ... />
				//</ ... >
				auto stag = std::make_shared<Xml::Tag>(xml, current, tag);
				auto end = current;
				auto res = parseNode(xml, stag, current, end);
				if (!res)return ParseFail;//error: sub tag parse failed
				tag->children.push_back(stag);
				current = end;
			}
		}
		else {
			// tag with content(< ... > ... </ ... >)
			auto contentBegin = current;
			while (*current != '<') {
				if (++current == stringEnd)return ParseFail;//error: xml format[tag without end up with ">"]
			}
			auto contentEnd = current;
			tag->content = Xml::StringView(contentBegin, contentEnd);
		}
	}

	return true;
}

static bool parseClassify(const std::shared_ptr<Xml>& xml, const std::string::const_iterator& begin, std::string::const_iterator& end) {
	if (*(begin + 1) != '?') {
		// this tag is the root tag
		//< ... > ... < ... />
		auto root = std::make_shared<Xml::Tag>(xml, begin);
		auto res = parseNode(xml, root, begin, end);
		if (res)
			xml->root = root;
		return res;
	}
	else {
		// this tag is external tag
		//<? ... ?>
		auto node = std::make_shared<Xml::Tag>(xml, begin);
		auto res = parseExtern(xml, node, begin, end);
		if (res)
			xml->externTags.push_back(node);
		return res;
	}
}


bool Xml::parse(const std::shared_ptr<Xml> xml)
{
	const std::string& content = xml->toXmlString();
	std::string::const_iterator end;
	for (auto iter = content.begin(); iter != content.end(); iter++) {
		if (*iter == '<' && (iter + 1) != content.end()/*parseClassify() will access the next block memory, check the bound*/) {
			auto res = parseClassify(xml, iter, end);
			if (!res) {
				// if parse fail, clear all extern tag and root
				xml->root = nullptr;
				xml->externTags.clear();
				return false;
			}
			iter = end - 1;
		}
	}
	if (xml->root == nullptr)return false;
	return true;
}

std::shared_ptr<Xml> Xml::fromFile(const std::string& path)
{
	auto iter = _fileCache.find(path);
	if (iter == _fileCache.end()) {
		std::ifstream file(path);
		if (file.fail())return nullptr;
		auto rawPtr = new Xml(path);
		std::string buf;
		while (std::getline(file, buf))
			rawPtr->content += buf + "\n";

		_fileCache[path] = std::shared_ptr<Xml>(rawPtr);
		file.close();
		return _fileCache[path];
	}
	return iter->second;
}

std::shared_ptr<Xml> Xml::fromString(const std::shared_ptr<std::string>& str)
{
	auto rawPtr = new Xml(str);
	return std::shared_ptr<Xml>(rawPtr);
}

std::shared_ptr<Xml> Xml::fromString(const std::string& str)
{
	auto ptr = std::make_shared<std::string>(str);
	auto rawPtr = new Xml(ptr);
	return std::shared_ptr<Xml>(rawPtr);
}

std::shared_ptr<Xml> Xml::fromString(std::string&& str)
{
	auto ptr = std::make_shared<std::string>(str);
	auto rawPtr = new Xml(ptr);
	return std::shared_ptr<Xml>(rawPtr);
}

std::shared_ptr<Xml> Xml::updateFromFile(const std::string& path)
{
	std::ifstream file(path);
	if (file.fail())return nullptr;
	std::string res;
	std::string buf;
	while (std::getline(file, buf))
		res += buf + "\n";

	auto iter = _fileCache.find(path);
	if (iter == _fileCache.end()) {
		_fileCache[path] = std::shared_ptr<Xml>(new Xml(path, std::move(res)));
		file.close();
		return _fileCache[path];
	}

	iter->second->content = res;
	file.close();
	return iter->second;
}

bool Xml::update()
{
	auto iter = _fileCache.find(this->path);
	if (iter == _fileCache.end())return false;//this xml object has all ready free
	auto smartPtr = iter->second;

	if (!path.empty()) {
		// Retrieve file again
		std::ifstream file(path);
		if (file.fail())return false;// read fail but don't clear [root]] and [extern tag]
		std::string res;
		std::string buf;
		while (std::getline(file, buf))
			res += buf + "\n";
		this->content = res;
	}

	return Xml::parse(smartPtr);
}

inline void Xml::print() const
{
	std::cout << content << std::endl;
}

inline const std::string& Xml::toXmlString() const
{
	return content;
}

const std::string Xml::toString() const
{
	if (root == nullptr)return "Warning: this is not a xml string";
	// TODO: add externTag to string
	return root->toString();
}

void Xml::free()
{
	auto iter = _fileCache.find(this->path);
	if (iter != _fileCache.end())
		_fileCache.erase(_fileCache.find(this->path));
}

static std::string _indentation(const unsigned int depth) {
	std::string res;
	for (auto i = 0; i < depth; i++)
		res += '\t';
	return res;
}

static std::string _attrToString(const std::pair<std::string, Xml::StringView>& attr) {
	return attr.first + " : " + attr.second.toString();
}

static std::string _toString(const unsigned int depth, const Xml::Tag* ptr) {
	std::string res = _indentation(depth) + "- " + ptr->name.toString() + "\n";
	for (auto attr : ptr->attributes) {
		res += _indentation(depth) + " * " + _attrToString(attr) + "\n";
	}

	for (auto sub : ptr->children)
		res += _toString(depth + 1, sub.get());
	return res;
}

_XML_API std::shared_ptr<Xml::Tag> Xml::Tag::newTag(const std::string& name)
{
	for (auto iter = name.begin(); iter != name.end(); iter++) {
		if (xmlIsSpace(iter))return std::shared_ptr<Xml::Tag>(nullptr);
	}
	return std::shared_ptr<Xml::Tag>(new Xml::Tag(name));
}

_XML_API std::shared_ptr<Xml::Tag> Xml::Tag::newTag(std::string&& name)
{
	for (auto iter = name.begin(); iter != name.end(); iter++) {
		if (xmlIsSpace(iter))return std::shared_ptr<Xml::Tag>(nullptr);
	}
	return std::shared_ptr<Xml::Tag>(new Xml::Tag(name));
}

std::shared_ptr<Xml::Tag> Xml::Tag::get() 
{
	return shared_from_this();
}

bool Xml::Tag::addChild(std::shared_ptr<Xml::Tag>& child)
{
	if (child == nullptr)return false;
	child->xml = this->xml;
	child->parent = shared_from_this();
	children.push_back(child);
	return true;
}

std::string Xml::Tag::toString()const
{
	auto res = _toString(0, this);
	return res;
}

static std::string attrToString(const Xml::Tag* tag) {
	if (tag->attributes.empty())return std::string();
	std::string res;// add a space
	for (auto iter = tag->attributes.begin(); iter != tag->attributes.end(); iter++) {
		res += " " + iter->first + "=" + "\"" + iter->second.toXmlString() + "\"";
	}
	return res;
}

inline static std::string toBeginTag(const Xml::Tag* tag) {
	return "<" + tag->name.toXmlString() + attrToString(tag) + ">";
}
inline static std::string toEndTag(const Xml::Tag* tag) {
	return "</" + tag->name.toXmlString() + ">";
}
inline static std::string toEndPointTag(const Xml::Tag* tag) {
	return "<" + tag->name.toXmlString() + attrToString(tag) + "/>";
}

std::string Xml::Tag::toXmlString()const
{
	if (content.vaild()) {
		std::string res;
		res += toBeginTag(this);
		res += content.toXmlString();
		res += toEndTag(this);
		return res;
	}
	else if (!children.empty()) {
		std::string res;
		res += toBeginTag(this);
		for (auto iter = children.begin(); iter != children.end(); iter++) {
			res += (*iter)->toXmlString();
		}
		res += toEndTag(this);
		return res;
	}
	else {
		return toEndPointTag(this);
	}

}

std::shared_ptr<Xml> Xml::Tag::toXml() const
{
	return Xml::fromString(toXmlString());
}

bool Xml::StringView::compare(const Xml::StringView& first, const Xml::StringView& second)
{
	auto fIter = first._begin, sIter = second._begin;
	for (; fIter != first._end; fIter++, sIter++) {
		if (sIter == second._end)return false;
		else if (*fIter != *sIter)return false;
	}
	if (sIter != second._end)return false;
	return true;
}

bool Xml::StringView::compare(const Xml::StringView& first, const std::string& second)
{
	auto fIter = first._begin;
	auto sIter = second.begin();
	for (; fIter != first._end; fIter++, sIter++) {
		if (sIter == second.end()) return false;
		else if (*fIter != *sIter)return false;
	}
	if (sIter != second.end())return false;
	return false;
}

/*
Convert the raw string to string
*/
std::string Xml::StringView::toString()const {
	if (!_vaild)return std::string();

	auto str = std::string();
	for (auto iter = _begin; iter != _end; iter++) {
		if (*iter == '&') {
			//Handle convert
			auto newiter = iter;
			newiter++;
			if (newiter == _end)goto normal;
			if (*newiter++ == 'a') {
				// "&" or "'"
				if (newiter == _end)goto normal;
				if (*newiter++ == 'm') {
					if (newiter == _end)goto normal;
					if (*newiter++ == 'p') {
						if (newiter == _end)goto normal;
						if (*newiter == ';') {
							str += '&';
							iter = newiter;
						}
						else goto normal;
					}
					else goto normal;
				}
				else if (*newiter++ == 'p') {
					if (newiter == _end)goto normal;
					if (*newiter++ == 'o') {
						if (newiter == _end)goto normal;
						if (*newiter++ == 's') {
							if (newiter == _end)goto normal;
							if (*newiter == ';') {
								str += '\'';
								iter = newiter;
							}
							else goto normal;
						}
						else goto normal;
					}
					else goto normal;
				}
				else goto normal;
			}
			else if (*newiter++ == 'l') {
				// "<"
				if (newiter == _end)goto normal;
				if (*newiter++ == 't') {
					if (newiter == _end)goto normal;
					if (*newiter == ';') {
						str += '<';
						iter = newiter;
					}
					else goto normal;
				}
				else goto normal;
			}
			else if (*newiter++ == 'g') {
				// ">"
				if (newiter == _end)goto normal;
				if (*newiter++ == 't') {
					if (newiter == _end)goto normal;
					if (*newiter == ';') {
						str += '>';
						iter = newiter;
					}
					else goto normal;
				}
				else goto normal;
			}
			else if (*newiter++ == 'q') {
				// """
				if (newiter == _end)goto normal;
				if (*newiter++ == 'u') {
					if (newiter == _end)goto normal;
					if (*newiter++ == 'o') {
						if (newiter == _end)goto normal;
						if (*newiter == ';') {
							str += '\"';
							iter = newiter;

						}
						else goto normal;
					}
					else goto normal;
				}
				else goto normal;
			}
			else {
			normal:
				str += *iter;
			}
		}
		else
			str += *iter;
	}

	return str;
}

std::string Xml::StringView::toXmlString() const
{
	return std::string(_begin, _end);
}
