﻿#pragma once

#include <iostream>
#include <fstream>
#include <iterator> 
#include <memory>
#include <map>
#include <string>
#include <list>
#include <atomic>

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
		std::string::const_iterator _begin;
		std::string::const_iterator _end;
		bool _vaild;
	public:

#ifdef  _DEBUG
		std::string view;
		StringView() :_begin(), _end(), _vaild(false), view() {}
		StringView(std::string::const_iterator&& begin_, std::string::const_iterator&& end_) :_begin(begin_), _end(end_), _vaild(true), view(_begin, _end) {}
		StringView(const std::string::const_iterator& begin_, const std::string::const_iterator& end_) :_begin(begin_), _end(end_), _vaild(true), view(_begin, _end) {}

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
		StringView() :_begin(), _end(), _vaild(false) {}
		StringView(std::string::const_iterator&& begin_, std::string::const_iterator&& end_) :_begin(begin_), _end(end_), _vaild(true) {}
		StringView(const std::string::const_iterator& begin_, const std::string::const_iterator& end_) :_begin(begin_), _end(end_), _vaild(true) {}

		StringView(const StringView& other) :_begin(other._begin), _end(other._end), _vaild(other._vaild) {}
		StringView(StringView&& other)noexcept :_begin(std::move(other._begin)), _end(std::move(other._end)), _vaild(std::move(other._vaild)) {}

		StringView& operator=(const StringView& other) {
			this->_begin = other._begin;
			this->_end = other._end;
			this->_vaild = other._vaild;
			return *this;
		}

#endif //  _DEBUG

		bool operator==(const StringView& other) const {
			return (other._begin == this->_begin) && (other._end == this->_end) ? true : false;
		}

		inline bool vaild() const {
			return !_vaild;
		}

		/*
		low performance convert to string for debug
		args:
			void
		return: string in format like yaml
		*/
		std::string toString() const;
	};

	class _XML_API Tag {
	public:
		Tag(const std::shared_ptr<Xml>& xml_, const std::string::const_iterator& begin_) :
			begin_(begin_),
			xml(xml_),
			parent(),
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

		inline std::string::const_iterator begin()const {
			return this->begin_;
		}

		std::string toString()const;
		std::string& toXmlString()const;
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
	const std::string toString() const;

	/*
	Delete cache, only remove Xml from [_fileCache]. If others catch the std::shared_ptr<Xml>, std::shared_ptr will not release resource.
	args:
		void
	return: void
	*/
	void free();

};






