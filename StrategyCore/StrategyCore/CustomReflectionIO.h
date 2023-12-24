#pragma once

#include "Document.h"
#include "Reflection.h"
#include <variant>
#include <optional>
#include <memory>

namespace Strategy
{
	template<class T>
	struct Serializer;

	template<class T>
	concept HasSerializer = std::is_void_v<typename Serializer<T>::SerializerTag>;

	template<class T, class V>
	bool TryVisitMember( T& obj, std::string_view name, V&& visitCallback )
	{
		bool found = false;
		auto visitor = [&found, &obj, name, visitCallback = std::forward<V>( visitCallback )]( auto& member )
		{
			if ( std::get<0>( member ) == name )
			{
				visitCallback( std::get<1>( member )(obj).get() );
				found = true;
			}
		};

		Reflection::VisitTuple( Accessors<T>::Get(), visitor );
		return found;
	}

	template<class T> requires( Concepts::NonStringRange<T> )
	struct Serializer<T>
	{
		using SerializerTag = void;

		void Write( Document& doc, std::string_view name, const T& range, NodeHandle parent, auto recursiveWriter ) const
		{
			auto arrayRoot = doc.EmplaceArrayNode( parent, name );
			size_t idx = 0;

			for ( const auto& m : range )
				recursiveWriter( doc, std::to_string( idx++ ), m, arrayRoot );
		}

		void Read( T& obj, const Document& doc, Document::Iterator& it, auto recursiveReader ) const
		{
			size_t depth = doc.GetDepth( GetPath( *it ) );
			size_t idx = 0;

			while ( it < doc.GetEnd() - 1 && doc.GetDepth( GetPath( *(it + 1) ) ) > depth )
			{
				++it;
				if constexpr ( Concepts::ResizableRange<T> )
				{
					recursiveReader( obj.emplace_back(), doc, it );
				}
				else if constexpr ( Concepts::FixedRange<T> )
				{
					recursiveReader( obj[idx++], doc, it );
				}
			}
		}
	};

	template<class T> requires( Reflection::HasAccessors<T> )
	struct Serializer<T>
	{
		using SerializerTag = void;

		void Write( Document& doc, std::string_view name, const T& obj, NodeHandle parent, auto recursiveWriter ) const
		{
			auto objRoot = doc.EmplaceObjectNode( parent, name );
			auto visitor = [&doc, &obj, objRoot, recursiveWriter]( auto& accessor )
			{
				recursiveWriter( doc, std::get<0>( accessor ), std::get<1>( accessor )(obj).get(), objRoot );
			};

			Reflection::VisitTuple( Accessors<T>::Get(), visitor );
		}

		void Read( T& obj, const Document& doc, Document::Iterator& it, auto recursiveReader ) const
		{
			size_t depth = doc.GetDepth( GetPath( *it ) );

			while ( it < doc.GetEnd() - 1 && doc.GetDepth( GetPath( *(it + 1) ) ) > depth )
			{
				++it;
				TryVisitMember( obj, doc.GetName( GetPath( *it ) ), [recursiveReader, &it, &doc]( auto& member )
				{
					recursiveReader( member, doc, it );
				} );
			}
		}
	};

	template<class... Alts>
	struct Serializer<std::variant<Alts...>>
	{
		using SerializerTag = void;

		void Write( Document& doc, std::string_view name, const std::variant<Alts...>& obj, NodeHandle parent, auto recursiveWriter ) const
		{
			auto objRoot = doc.EmplaceObjectNode( parent, name );
			auto visitor = [&doc, &obj, objRoot, recursiveWriter]( const auto& value )
			{
				recursiveWriter( doc, "value", value, objRoot );
			};

			doc.EmplaceProperty( objRoot, "index", static_cast<int64_t>( obj.index() ) );
			std::visit( visitor, obj );
		}

		void Read( std::variant<Alts...>& obj, const Document& doc, Document::Iterator& it, auto recursiveReader ) const
		{
			it++;
			size_t index = *FromPrimitive<size_t>( std::get<Property>( *it++ ).value );
			obj = Reflection::DefaultInitVariantFromIndex<std::decay_t<decltype(obj)>>( index );
			auto visitor = [&doc, &it, recursiveReader]( auto& value )
			{
				recursiveReader( value, doc, it );
			};

			std::visit( visitor, obj );
		}
	};

	template<class T>
	struct Serializer<std::optional<T>>
	{
		using SerializerTag = void;

		void Write( Document& doc, std::string_view name, const std::optional<T>& obj, NodeHandle parent, auto recursiveWriter ) const
		{
			auto objRoot = doc.EmplaceObjectNode( parent, name );
			
			doc.EmplaceProperty( objRoot, "valid", obj.has_value() );

			if ( obj.has_value() )
			{
				recursiveWriter( doc, "value", *obj, objRoot );
			}
		}

		void Read( std::optional<T>& obj, const Document& doc, Document::Iterator& it, auto recursiveReader ) const
		{
			it++;
			bool hasValue = *FromPrimitive<bool>( std::get<Property>( *it++ ).value );
			
			if ( hasValue )
			{
				obj.emplace();
				recursiveReader( *obj, doc, it );
			}
			else
			{
				obj = {};
			}
		}
	};

	template<class T>
	struct Serializer<std::unique_ptr<T>>
	{
		using SerializerTag = void;

		void Write( Document& doc, std::string_view name, const std::unique_ptr<T>& obj, NodeHandle parent, auto recursiveWriter ) const
		{
			recursiveWriter( doc, name, *obj, parent );
		}

		void Read( std::unique_ptr<T>& obj, const Document& doc, Document::Iterator& it, auto recursiveReader ) const
		{
			obj = std::make_unique<T>();
			recursiveReader( *obj, doc, it );
		}
	};
}