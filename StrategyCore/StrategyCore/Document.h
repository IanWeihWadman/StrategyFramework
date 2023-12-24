#pragma once

#include "Concepts.h"

#include <variant>
#include <optional>
#include <string>
#include <vector>
#include <memory>

namespace Strategy
{
	class Document;

	struct DocumentString
	{
		const static char Separator = '/';
		uint32_t idx;
	};

	inline bool operator==( const DocumentString& a, const DocumentString& b )
	{
		return a.idx == b.idx;
	}

	inline bool operator!=( const DocumentString& a, const DocumentString& b )
	{
		return a.idx != b.idx;
	}

	using Primitive = std::variant<std::monostate, int64_t, double, std::string>;

	struct ObjectNode { DocumentString path; };
	struct ArrayNode { DocumentString path; };
	struct Property { DocumentString path; Primitive value; };

	using Entry = std::variant<ObjectNode, ArrayNode, Property>;

	inline DocumentString GetPath( const Entry& entry )
	{
		static auto getPath = []( const auto& e )
		{
			return e.path;
		};

		return std::visit( getPath, entry );
	}

	template<class T, class = void>
	struct HasToString : std::false_type {};

	template<class T>
	struct HasToString<T, std::enable_if_t<std::is_same_v<std::string, decltype( T::ToString( std::declval<T&>() ) )>>> : std::true_type{};

	template<class T, class = void>
	struct HasParse : std::false_type {};

	template<class T>
	struct HasParse<T, std::enable_if_t<std::is_same_v<std::optional<T>, decltype( T::Parse( std::declval<std::string&>() ) )>>> : std::true_type {};

	template<class T>
	constexpr bool Parseable = std::conjunction<HasToString<T>, HasParse<T>>::value;

	template<class T>
	constexpr Primitive ToPrimitive( T value )
	{
		if constexpr ( std::is_same_v<T, bool> )
			return { value ? 1 : 0 };
		else if constexpr ( std::is_same_v<T, std::monostate> )
			return { value };
		else if constexpr ( std::is_integral_v<T> )
			return { static_cast<int64_t>( value ) };
		else if constexpr ( std::is_convertible_v<T, double> )
			return { static_cast<double>( value ) };
		else if constexpr ( std::is_convertible_v<T, std::string> )
			return { static_cast<std::string>( value ) };
		else
			return { T::ToString( value ) };
	}

	template<class T>
	constexpr std::optional<T> FromPrimitive( Primitive primitive )
	{
		auto visitor = []( auto& value ) -> std::optional<T>
		{
			if constexpr ( std::is_convertible_v<T, std::decay_t<decltype( value )>> )
				return { static_cast<T>( value ) };
			else if constexpr ( Parseable<T> )
				return T::Parse( value );
			else
				return std::nullopt;
		};

		return std::visit( visitor, primitive );
	}

	template <class T, class Variant>
	struct VariantConvertible;

	template <class T, class... Types>
	struct VariantConvertible<T, std::variant<Types...>> : std::disjunction<Concepts::BiConvertible<T, Types>...> {};

	template<class T>
	concept PrimitiveConvertible = VariantConvertible<T, Primitive>::value;

	struct NodeHandle
	{
		size_t predictedIndex;
		DocumentString identifier;
	};

	class DocumentStringPool
	{
	public:
		DocumentString Get( DocumentString path, std::string_view str )
		{
			std::vector<size_t> cachedPath = cachedPaths[path.idx];
			cachedPath.push_back( FindName( str ) );
			return FindCachedPath( cachedPath );
		}

		DocumentString Get( std::string_view str )
		{
			std::vector<size_t> values;
			size_t separatorIdx = 0;

			while ( true )
			{
				size_t nextSeparatorIdx = str.find( DocumentString::Separator, ++separatorIdx );

				values.push_back( FindName( str.substr( separatorIdx, nextSeparatorIdx - separatorIdx ) ) );
				separatorIdx = nextSeparatorIdx;

				if ( nextSeparatorIdx == std::string_view::npos )
					break;
			}

			return FindCachedPath( values );
		}

		std::optional<DocumentString> TryGet( std::string_view str ) const
		{
			std::vector<size_t> values;
			size_t separatorIdx = 0;

			while ( true )
			{
				size_t nextSeparatorIdx = str.find( DocumentString::Separator, ++separatorIdx );

				if ( auto found = std::find( cachedNames.cbegin(), cachedNames.cend(), str.substr( separatorIdx, nextSeparatorIdx - separatorIdx ) ); found != cachedNames.cend() )
					values.emplace_back( std::distance( cachedNames.cbegin(), found ) );
				else
					return {};

				separatorIdx = nextSeparatorIdx;

				if ( nextSeparatorIdx == std::string_view::npos )
					break;
			}

			if ( auto found = std::find( cachedPaths.cbegin(), cachedPaths.cend(), values ); found != cachedPaths.cend() )
				return DocumentString{ static_cast<uint32_t>( std::distance( cachedPaths.cbegin(), found ) ) };
			
			return {};
		}

		size_t GetHash( DocumentString str ) const
		{
			size_t seed = 17;
			for ( auto& p : cachedPaths[str.idx] )
			{
				seed ^= std::hash<std::string>{}( cachedNames[p] );
			}
			return seed;
		}

		size_t GetDepth( DocumentString str ) const
		{
			return cachedPaths[str.idx].size();
		}

		std::string_view GetName( DocumentString str ) const
		{
			return cachedNames[cachedPaths[str.idx].back()];
		}

	private:
		size_t FindName( std::string_view str )
		{
			auto found = std::find( cachedNames.cbegin(), cachedNames.cend(), str );

			if ( found == cachedNames.cend() )
			{
				cachedNames.emplace_back( str );
				return cachedNames.size() - 1;
			}

			return std::distance( cachedNames.cbegin(), found );
		}

		DocumentString FindCachedPath( const std::vector<size_t>& path )
		{
			auto found = std::find( cachedPaths.cbegin(), cachedPaths.cend(), path );

			if ( found == cachedPaths.cend() )
			{
				cachedPaths.emplace_back( path );
				return { static_cast<uint32_t>( cachedPaths.size() - 1 ) };
			}

			return { static_cast<uint32_t>( std::distance( cachedPaths.cbegin(), found ) ) };
		}

		std::vector<std::string> cachedNames;
		std::vector<std::vector<size_t>> cachedPaths;
	};

	class Document
	{
	public:
		using Iterator = decltype( std::declval<const std::vector<Entry>>().cbegin() );

		Document()
		{
			entries.emplace_back( ObjectNode{ pool.Get( "root" ) } );
		}

		NodeHandle EmplaceObjectNode( NodeHandle parent, std::string_view name );
		NodeHandle EmplaceArrayNode( NodeHandle parent, std::string_view name );
		void EmplaceProperty( NodeHandle node, std::string_view name, Primitive value );

		Primitive GetProperty( std::string_view propertyPath ) const
		{
			if ( auto documentString = pool.TryGet( propertyPath ); documentString )
			{
				auto entry = std::ranges::find_if( entries, [documentString]( auto& entry ) { return GetPath( entry ) == *documentString; } );

				if ( entry != entries.cend() && std::holds_alternative<Property>( *entry ) )
					return std::get<Property>( *entry ).value;
			}

			return {};
		}

		bool UpdateProperty( std::string_view propertyPath, Primitive value )
		{
			if ( auto documentString = pool.TryGet( propertyPath ); documentString )
			{
				auto entry = std::ranges::find_if( entries, [documentString]( auto& entry ) { return GetPath( entry ) == *documentString; } );

				if ( entry != entries.cend() && std::holds_alternative<Property>( *entry ) )
					std::get<Property>( *entry ).value = value;

				return true;
			}

			return false;
		}
		
		size_t Hash() const
		{
			auto hashEntry = [this]( const Entry& e )
			{
				if ( std::holds_alternative<ObjectNode>( e ) )
					return 0xc25f1eab8e19db6c ^ pool.GetHash( std::get<ObjectNode>( e ).path );
				if ( std::holds_alternative<ArrayNode>( e ) )
					return 0x8d92e95f190f7f74 ^ pool.GetHash( std::get<ArrayNode>( e ).path );
				if ( std::holds_alternative<Property>( e ) )
					return 0x3801e7aba7da6806 ^ pool.GetHash( std::get<Property>( e ).path ) ^ std::hash<Primitive>{}( std::get<Property>( e ).value );
				return static_cast<size_t>( 0 );
			};

			std::size_t seed = entries.size();
			for ( const auto& e : entries ) 
			{
				size_t x = hashEntry( e );
				seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}

		auto GetRootIterator() const
		{
			return entries.cbegin();
		}

		auto GetEnd() const
		{
			return entries.cend();
		}

		NodeHandle GetRoot() const
		{
			return NodeHandle{ 0, 0 };
		}

		size_t GetDepth( const Entry& node ) const
		{
			return pool.GetDepth( GetPath( node ) );
		}

		size_t GetDepth( NodeHandle node ) const
		{
			return pool.GetDepth( GetPath( *Find( node ) ) );
		}

		size_t GetDepth( DocumentString str ) const
		{
			return pool.GetDepth( str );
		}

		std::string_view GetName( NodeHandle node ) const
		{
			return pool.GetName( GetPath( *Find( node ) ) );
		}

		std::string_view GetName( const Entry& node ) const
		{
			return pool.GetName( GetPath( node ) );
		}

		std::string_view GetName( DocumentString str ) const
		{
			return pool.GetName( str );
		}

	private:
		Iterator Find( NodeHandle node ) const;

		std::vector<Entry> entries;
		DocumentStringPool pool;
	};
}