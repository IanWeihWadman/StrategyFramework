#pragma once

#include "Document.h"
#include "DocumentJsonIO.h"
#include "DocumentReflectionIO.h"

#include <sstream>

namespace Strategy
{
	template<class T>
	std::string Serialize( const T& obj )
	{
		std::stringstream sstream;
		Document doc = ToDocument( obj );
		DocumentToJsonObject( doc, sstream, doc.GetRootIterator() );
		return sstream.str();
	}

	template<class T>
	T Deserialize( const std::string& str )
	{
		return FromDocument<T>( JsonToDocument( str ) );
	}
}