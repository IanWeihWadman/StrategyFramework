#pragma once

#include "Document.h"
#include "Reflection.h"
#include "CustomReflectionIO.h"

#include <ranges>

namespace Strategy
{
	namespace Detail
	{
		struct DocumentWriter
		{
			template<class T>
			void operator()( Document& doc, std::string_view name, const T& obj, NodeHandle parent ) const
			{
				if constexpr ( HasSerializer<T> )
				{
					Serializer<T>{}.Write( doc, name, obj, parent, DocumentWriter{} );
				}
				else if constexpr ( PrimitiveConvertible<T> )
				{
					doc.EmplaceProperty( parent, name, ToPrimitive( obj ) );
				}
				else
				{
					static_assert( std::_Always_false<T>, "Serialization is not supported on this object" );
				}
			}
		};

		struct DocumentReader
		{
			template<class T>
			void operator()( T& obj, const Document& doc, Document::Iterator& it ) const
			{
				if constexpr ( HasSerializer<T> )
				{
					Serializer<T>{}.Read( obj, doc, it, DocumentReader{} );
				}
				else if constexpr ( PrimitiveConvertible<T> )
				{
					auto result = FromPrimitive<std::decay_t<decltype(obj)>>( std::get<Property>( *it ).value );
					if ( result )
						obj = *result;
				}
				else
				{
					static_assert( std::_Always_false<T>, "Serialization is not supported on this object" );
				}
			}
		};

	}

	template<class T>
	Document ToDocument( const T& obj )
	{
		Document doc;
		Detail::DocumentWriter{}( doc, "obj", obj, doc.GetRoot() );
		return doc;
	}

	template<class T>
	T FromDocument( const Document& doc )
	{
		T result{};
		auto iter = doc.GetRootIterator();
		iter++;
		Detail::DocumentReader{}( result, doc, iter );
		return std::move( result );
	}

	template<class T>
	void FromDocument( T& obj, const Document& doc )
	{
		auto iter = doc.GetRootIterator();
		iter++;
		Detail::DocumentReader{}( obj, doc, iter );
	}
}