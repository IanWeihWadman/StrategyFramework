#pragma once

#include "Document.h"

#include <iostream>
#include <format>

namespace Strategy
{
	namespace Detail
	{
		struct JsonData
		{
			std::string_view json;
			size_t cursor;

			char current()
			{
				return json[cursor];
			}

			void next()
			{
				cursor++;
			}
		};

		inline bool IsBlank( char c )
		{
			return c == ' ' || c == '\t';
		}

		inline Primitive ParsePrimitive( JsonData& data )
		{
			size_t end = std::min( std::min( data.json.find( ',', data.cursor ), data.json.find( ']', data.cursor ) ), data.json.find( '}', data.cursor ) );
			std::string_view view = data.json.substr( data.cursor, end - data.cursor );
			data.cursor = end;

			if ( view[0] == '\"' )
			{
				return { std::string( view.substr( 1, view.size() - 2 ) ) };
			}
			else if ( view.find( '.' ) != std::string_view::npos )
			{
				double d;
				std::from_chars( view.data(), view.data() + view.size(), d );
				return { d };
			}
			else
			{
				int64_t i;
				std::from_chars( view.data(), view.data() + view.size(), i );
				return { i };
			}

			return {};
		}

		inline std::string PrimitiveToString( Primitive p )
		{
			auto visitor = []( auto& v )-> std::string
			{
				using Type = std::decay_t<decltype(v)>;

				if constexpr ( std::is_same_v<Type, double> )
					return std::to_string( v );
				else if constexpr ( std::is_same_v<Type, int64_t> )
					return std::to_string( v );
				else if constexpr ( std::is_same_v<Type, std::string> )
					return std::format( "\"{}\"", v );

				return "\"\"";
			};

			return std::visit( visitor, p );
		}
	}

	void JsonToObjectInternal( Document& doc, Detail::JsonData& data, NodeHandle handle );
	void JsonToArrayInternal( Document& doc, Detail::JsonData& data, NodeHandle handle );
	void DocumentToJsonArray( const Document& doc, std::ostream& jsonStream, Document::Iterator& it );
	void DocumentToJsonObject( const Document& doc, std::ostream& jsonStream, Document::Iterator& it );

	inline Document JsonToDocument( const std::string& json )
	{
		Document doc;
		Detail::JsonData data{ json, 1 };
		JsonToObjectInternal( doc, data, doc.GetRoot() );
		return doc;
	}

	inline void DocumentToJson( const Document& doc, std::ostream& jsonStream )
	{
		auto it = doc.GetRootIterator();
		DocumentToJsonObject( doc, jsonStream, it );
	}
}