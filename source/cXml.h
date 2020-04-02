#pragma once

#include <iostream>
#include <fstream>
#include <iterator> 
#include <memory>
#include <map>
#include <string>
#include <list>
#include <atomic>

#include <assert.h>

#ifdef _XML_DLL
#ifdef WIN32 // Windows platform
#pragma warning(push)
#pragma warning(disable:4251)
#pragma warning(pop)
#ifdef _DLL_IMPORT
#define _XML_API __declspec(dllimport)
#else
#define _XML_API __declspec(dllexport)
#endif // _DLL_EXPORT
#else	// UNIX platform
#define _XML_API __attribute__ ((__visibility__("default")))
#endif
#else
#define _XML_API // static link
#endif // _DLL

#ifdef  _XML_DEBUG
inline bool fail() {
	assert(false);
	return false;
}

#define ParseFail fail()
#else
#define ParseFail false
#endif

class _XML_API Xml {
	static std::map<std::string/*path*/, std::shared_ptr<Xml>> _fileCache;
	Xml(const std::string& filePath_) :path(filePath_), content() {}
	Xml(std::shared_ptr<std::string> str) :path(), content(*str) {}
	Xml(const std::string& filePath_, std::string&& content_) :path(filePath_), content(content_) {}
	std::string path;
	std::string content;
public:
	static bool parse(const std::shared_ptr<Xml> xml);

	class _XML_API StringView {
		std::string raw;
		std::string::const_iterator _begin;
		std::string::const_iterator _end;
		bool _vaild;
	public:
		static bool compare(const Xml::StringView& first, const Xml::StringView& second);
		static bool compare(const Xml::StringView& first, const std::string& second);
#ifdef  _XML_DEBUG
		std::string view;
		StringView() :_begin(), _end(), _vaild(false), view() {}
		StringView(std::string::const_iterator&& begin_, std::string::const_iterator&& end_) :_begin(begin_), _end(end_), _vaild(begin_ != end_), view(_begin, _end) {}
		StringView(const std::string::const_iterator& begin_, const std::string::const_iterator& end_) :_begin(begin_), _end(end_), _vaild(begin_ != end_), view(_begin, _end) {}

		StringView(const StringView& other) :_begin(other._begin), _end(other._end), _vaild(other._vaild), view() {}
		StringView(StringView&& other)noexcept :_begin(std::move(other._begin)), _end(std::move(other._end)), _vaild(std::move(other._vaild)), view(_begin, _end) {}

		StringView& operator=(const StringView& other) {
			this->_begin = other._begin;
			this->_end = other._end;
			this->_vaild = other._vaild;
			this->view = other.view;
			return *this;
		}
#else
		StringView() :raw(), _begin(), _end(), _vaild(false) {}
		StringView(std::string::const_iterator&& begin_, std::string::const_iterator&& end_) :raw(), _begin(begin_), _end(end_), _vaild(begin_ != end_) {}
		StringView(const std::string::const_iterator& begin_, const std::string::const_iterator& end_) :raw(), _begin(begin_), _end(end_), _vaild(begin_ != end_) {}

		StringView(const StringView& other) :raw(), _begin(other._begin), _end(other._end), _vaild(other._vaild) {}
		StringView(StringView&& other)noexcept :raw(), _begin(std::move(other._begin)), _end(std::move(other._end)), _vaild(std::move(other._vaild)) {}

		// from string
		StringView(const std::string& other)noexcept :raw(other), _begin(raw.begin()), _end(raw.end()), _vaild(std::move(!raw.empty())) {}
		StringView(std::string&& other)noexcept :raw(other), _begin(raw.begin()), _end(raw.end()), _vaild(std::move(!raw.empty())) {}

		StringView& operator=(const StringView& other) {
			this->_begin = other._begin;
			this->_end = other._end;
			this->_vaild = other._vaild;
			return *this;
		}

		StringView& operator=(const char* str) {
			this->raw = std::string(str);
			this->_begin = raw.begin();
			this->_end = raw.end();
			this->_vaild = (_begin != _end);
			return *this;
		}

#endif //  _DEBUG

		bool operator==(const StringView& other) const {
			return (other._begin == this->_begin) && (other._end == this->_end) ? true : false;
		}

		bool operator==(const std::string& other) const {
			return compare(*this, other);
		}

		inline bool vaild() const {
			return _vaild;
		}

		/*
		Escape character to normal character
		args:
			void
		return: normal character
		*/
		std::string toString() const;
		std::string toXmlString() const;
	};

	class _XML_API Tag : public std::enable_shared_from_this<Xml::Tag>{
		/*
		warning: use newTag(), do not direct use Xml::Tag construction
		*/
		Tag(const std::string& name_): name(name_) {}
		Tag(std::string&& name_) : name(name_) {
			
		}

	public:
		static std::shared_ptr<Xml::Tag> newTag(const std::string& name);
		static std::shared_ptr<Xml::Tag> newTag(std::string&& name);
		friend Xml;
		/*
		warning: use newTag(), do not direct use Xml::Tag construction
		*/
		Tag(const std::shared_ptr<Xml>& xml_, const std::string::const_iterator& begin_) :
			begin_(begin_),
			xml(xml_),
			parent(),
			name(),
			content(),
			attributes(),
			children() {}
		Tag(const std::shared_ptr<Xml>& xml_, const std::string::const_iterator& begin_, const std::shared_ptr<Xml::Tag>& parent) :
			begin_(begin_),
			xml(xml_),
			parent(parent),
			name(),
			content(),
			attributes(),
			children() {}


		std::string::const_iterator begin_;
		std::weak_ptr<Xml> xml;
		std::weak_ptr<Tag> parent;
		Xml::StringView name;
		Xml::StringView content;
		std::map<std::string, Xml::StringView> attributes;
		std::list<std::shared_ptr<Tag>> children;

		/*
		Get the begin string::iterator for this tag
		return: string::iterator for this tag
		example0:
			<[name] attr:"value">
			^
			begin
		example1:
			<[name]/>
			^
			begin()
		*/
		inline std::string::const_iterator begin()const {
			return this->begin_;
		}

		std::shared_ptr<Xml::Tag> get();
		bool addChild(std::shared_ptr<Xml::Tag>& child);

		std::string toString()const;
		std::string toXmlString()const;
		std::shared_ptr<Xml> toXml()const;
	};

	std::list<std::shared_ptr<Tag>> externTags;
	std::shared_ptr<Tag> root;

	/*
	Load cached xml file. if file doesn't exist in cache, cache the file.
	Warning: This method will automaticly cache file, need to call free() to release memory.
	args:
		[const std::string& path]: xml file's path
	return: std::shared_ptr<Xml> that point to Xml
	except: if {file doesn't exist in cache} and {file can't open}, return nullptr
	*/
	static std::shared_ptr<Xml> fromFile(const std::string& path);

	/*
	Load xml in string.
	args:
		[const std::shared_ptr<std::string>& str]: xml file's path
	return: std::shared_ptr<Xml> that point to Xml
	*/
	static std::shared_ptr<Xml> fromString(const std::shared_ptr<std::string>& str);
	static std::shared_ptr<Xml> fromString(const std::string& str);
	static std::shared_ptr<Xml> fromString(std::string&& str);
	/*
	Update xml content. if file doesn't exist in cache, cache the file
	args:
		[const std::string& path]: xml file's path
	return: std::shared_ptr<Xml> that point to Xml
	except: if {file doesn't exist in cache}, return nullptr even if file is cached(but don't delete cache)
	*/
	static std::shared_ptr<Xml> updateFromFile(const std::string& path);

	bool update();
	void print()const;

	/*
	Get raw xml string
	*/
	const std::string& toXmlString()const;
	/*
	Convert xml to yaml-like format string(for debug)
	*/
	const std::string toString()const;

	/*
	Delete cache, only remove Xml from [_fileCache]. If others catch the std::shared_ptr<Xml>, std::shared_ptr will not release resource.
	args:
		void
	return: void
	*/
	void free();

};







