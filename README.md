


# cXml
A open-source c++ library for parse markup language data. 
### Features
- **Max performance 🚀**
`only retrieve characters once`

- **Less memory 🛒**
`no string copy`

- **Safe operation ☂️**
`stl smart pointer and string iterator, no raw pointer`

- **Cross platform 🐟**
`standard c++ 11, no dependencies`

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
    </🏛>

	 1. Xml object contain whole string
	 2. Xml's `std::list<std::shared_ptr<Tag>>`externTags contain one tag object  *<?xml?>*
	 3. Xml's root is the *<🏛> ... </🏛>* `Xml::Tag` object
	 4. Xml's root has 3 children(`Xml::Tag` Objects):  

			 - *<🚗/>* 
			 - *<🚂/>* 
			 - *<✈️/>*
