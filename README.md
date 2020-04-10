


# cXml
A open-source c++ library for parse markup language data. 
### Features
- **Max performance 🚀**
`only retrieve characters once without exception handling`

- **Less memory 🛒**
`no string copy`

- **Safe operation ☂️**
`stl smart pointer and string iterator, no raw pointer`

- **Cross platform 🐟**
`standard c++ 11, no dependencies`

- **No mixed content (this package doesn't support mix content) 👌🏻**
`ease to use in the most xml file (with single root node, every node can only be either text node or element node)`

### Doc
|Xml|Xml::Tag|Xml::StringView|
|--|--|--|
|contain full xml doc|xml tag|view xml raw string window|

 - **example**

    <?xml?>
    <🏛>
	    <🚗/>
	    <🚂/>
	    <✈️/>
		<🎁>gift</🎁>
    </🏛>

	 1. Xml object contain whole string
	 2. Xml's `std::list<std::shared_ptr<Tag>>`externTags contain one tag object  *<?xml?>*
	 3. Xml's root is the *<🏛> ... </🏛>* `Xml::Tag` object
	 4. Xml's root has 3 children(`Xml::Tag` Objects):  
			 - *<🚗/>* 
			 - *<🚂/>* 
			 - *<✈️/>*
			 - *<🎁>gift</🎁>*
	 5. *<🎁>gift</🎁>*  the 🎁 `Xml::Tag`'s content is “gift”
