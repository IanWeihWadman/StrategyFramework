#include "pch.h"
#include "DocumentJsonIO.h"

namespace Strategy
{

	void JsonToObjectInternal( Document& doc, Detail::JsonData& data, NodeHandle handle )
	{
		if ( data.json[data.cursor + 1] == '}' )
		{
			data.next();
			data.next();
			return;
		}

		while ( true )
		{
			while ( Detail::IsBlank( data.current() ) )
				data.next();

			if ( data.current() == '}' )
				break;

			size_t nameStart = data.json.find( '\"', data.cursor ) + 1;
			size_t nameEnd = data.json.find( '\"', nameStart );
			data.cursor = nameEnd;

			std::string_view name = data.json.substr( nameStart, nameEnd - nameStart );
			data.cursor = data.json.find( ':', data.cursor ) + 1;

			while ( Detail::IsBlank( data.current() ) )
				data.next();

			if ( data.current() == '{' )
			{
				auto emplace = doc.EmplaceObjectNode( handle, name );
				JsonToObjectInternal( doc, data, emplace );
			}
			else if ( data.current() == '[' )
			{
				auto emplace = doc.EmplaceArrayNode( handle, name );
				JsonToArrayInternal( doc, data, emplace );
			}
			else
			{
				doc.EmplaceProperty( handle, name, Detail::ParsePrimitive( data ) );
			}
		}

		data.next();
	}

	void JsonToArrayInternal( Document& doc, Detail::JsonData& data, NodeHandle handle )
	{
		size_t idx = 0;

		while ( true )
		{
			data.next();

			while ( Detail::IsBlank( data.current() ) )
				data.next();

			// Could be of length zero, check before and after
			if ( data.current() == ']' )
				break;

			if ( data.current() == '{' )
			{
				auto emplace = doc.EmplaceObjectNode( handle, std::to_string( idx ) );
				JsonToObjectInternal( doc, data, emplace );
			}
			else if ( data.current() == '[' )
			{
				auto emplace = doc.EmplaceArrayNode( handle, std::to_string( idx ) );
				JsonToArrayInternal( doc, data, emplace );
			}
			else
			{
				doc.EmplaceProperty( handle, std::to_string( idx ), Detail::ParsePrimitive( data ) );
			}

			if ( data.current() == ']' )
				break;

			idx++;
		}

		data.next();
	}

	void DocumentToJsonArray( const Document& doc, std::ostream& jsonStream, Document::Iterator& it )
	{
		size_t depth = doc.GetDepth( *it );

		jsonStream << "[";

		bool first = false;
		while ( it < doc.GetEnd() - 1 && doc.GetDepth( GetPath( *(it + 1) ) ) > depth )
		{
			++it;
			if ( !first )
				first = true;
			else
				jsonStream << ", ";

			if ( std::holds_alternative<ObjectNode>( *it ) )
			{
				DocumentToJsonObject( doc, jsonStream, it );
			}
			else if ( std::holds_alternative<ArrayNode>( *it ) )
			{
				DocumentToJsonArray( doc, jsonStream, it );
			}
			else if ( std::holds_alternative<Property>( *it ) )
			{
				jsonStream << Detail::PrimitiveToString( std::get<Property>( *it ).value );
			}
		}

		jsonStream << "]";
	}

	void DocumentToJsonObject( const Document& doc, std::ostream& jsonStream, Document::Iterator& it )
	{
		size_t depth = doc.GetDepth( *it );

		jsonStream << "{";

		bool first = false;
		while ( it < doc.GetEnd() - 1 && doc.GetDepth( GetPath( *(it + 1) ) ) > depth )
		{
			++it;
			if ( !first )
				first = true;
			else
				jsonStream << ", ";

			jsonStream << std::format( "\"{}\": ", doc.GetName( *it ) );

			if ( std::holds_alternative<ObjectNode>( *it ) )
			{
				DocumentToJsonObject( doc, jsonStream, it );
			}
			else if ( std::holds_alternative<ArrayNode>( *it ) )
			{
				DocumentToJsonArray( doc, jsonStream, it );
			}
			else if ( std::holds_alternative<Property>( *it ) )
			{
				jsonStream << Detail::PrimitiveToString( std::get<Property>( *it ).value );
			}
		}

		jsonStream << "}";
	}
}