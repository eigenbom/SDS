#include "propertylist.h"
#include <boost/foreach.hpp>

void PropertyList::clear()
{
	mProperties.clear();
}

bool PropertyList::empty()
{
	return mProperties.empty();
}


std::list<PropertyList::property_type> PropertyList::all()
{
	return std::list<PropertyList::property_type>(mProperties.begin(),mProperties.end());
}

boost::any PropertyList::property(PropertyList::key_type s)
{
	map_type::iterator it = mProperties.find(s);
	if (it==mProperties.end())
		return 0;
	else return it->second;
}

std::list<PropertyList::value_type> PropertyList::properties(PropertyList::key_type s)
{
	std::list<PropertyList::value_type> values;
	std::pair<map_type::iterator,map_type::iterator> range = mProperties.equal_range(s);
	BOOST_FOREACH(const_property_type pt, range)
	{
		values.push_back(pt.second);
	}
	return values;
}

void PropertyList::add(const PropertyList::key_type& s, const char* value)
{
	add(s,std::string(value));
}

void PropertyList::add(const std::string& s)
{
	mProperties.insert(property_type(s,boost::any()));
}

PropertyList::property_type PropertyList::makeProperty(const PropertyList::key_type& s, const char* value)
{
	return makeProperty(s,std::string(value));
}

PropertyList::property_type PropertyList::makeProperty(const std::string& s)
{
	return makeProperty(s,std::string());
}
