/**
 * List of properties (strings->any).
 *
 */

#ifndef PROPERTY_MAP_H
#define PROPERTY_MAP_H

#include <map>
#include <string>
#include <utility>
#include <sstream>
#include <list>

#include <boost/any.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>

class PropertyList
{
public:
	typedef std::string key_type;
	typedef boost::any value_type;
	typedef std::pair<key_type,value_type> property_type;

	void clear();
	bool empty();

	// property retrieval
	/// single property retrieval
	boost::any property(key_type s);
	/// multiple-value property retrieval
	std::list<PropertyList::value_type> properties(PropertyList::key_type s);
	/// all property retrieval
	std::list<PropertyList::property_type> all();

	// add a property
	template <typename T> void add(const key_type& s, const T& value);
	void add(const key_type& s, const char* value);
	void add(const key_type& s);
	template <typename T> void add(const T& value);
	template <typename T> void addStringAndValue(const key_type& s, const T& value);

	// make a property
	template <typename T>
	static PropertyList::property_type makeProperty(const key_type& s, const T& value);
	static PropertyList::property_type makeProperty(const key_type& s, const char* value);
	static PropertyList::property_type makeProperty(const key_type& s);
	template <typename T> static PropertyList::property_type makeProperty(const T& value);
private:
	typedef std::multimap<key_type,value_type> map_type;
	typedef std::pair<const key_type,value_type> const_property_type;
	map_type mProperties;
};

template <typename T>
	void PropertyList::add(const PropertyList::key_type& s, const T& value)
{
	mProperties.insert(property_type(s,value));
}

template <typename T>
void PropertyList::add(const T& value)
{
	std::ostringstream oss;
	oss << value;
	add(oss.str());
}

template <typename T>
void PropertyList::addStringAndValue(const key_type& s, const T& value)
{
	std::ostringstream oss;
	oss << s << ": " << value;
	add(oss.str());
}


template <typename T>
PropertyList::property_type PropertyList::makeProperty(const PropertyList::key_type& s, const T& value)
{
	return PropertyList::property_type(s,value);
}

template <typename T>
PropertyList::property_type PropertyList::makeProperty(const T& value)
{
	std::ostringstream oss;
	oss << value;
	return makeProperty(oss.str(),value);
}

#endif
