#include "pch.h"
#include "Document.h"

using namespace Strategy;

NodeHandle Document::EmplaceObjectNode( NodeHandle parent, std::string_view name )
{
	auto parentIter = Find( parent );
	auto combinedPath = pool.Get( parent.identifier, name );

	auto nextSibling = std::find_if( parentIter + 1, entries.cend(),
		[this, parentDepth = pool.GetDepth( parent.identifier )]( const auto& entry )
		{
			return parentDepth == pool.GetDepth( GetPath( entry ) );
		} );

	size_t insertIdx = std::distance( entries.cbegin(), nextSibling );

	entries.emplace( nextSibling, ObjectNode{ combinedPath } );
	return NodeHandle{ insertIdx, combinedPath };
}

NodeHandle Document::EmplaceArrayNode( NodeHandle parent, std::string_view name )
{
	auto parentIter = Find( parent );
	auto combinedPath = pool.Get( parent.identifier, name );

	auto nextSibling = std::find_if( parentIter + 1, entries.cend(),
		[this, parentDepth = pool.GetDepth( parent.identifier )]( const auto& entry )
		{
			return parentDepth == pool.GetDepth( GetPath( entry ) );
		} );

	size_t insertIdx = std::distance( entries.cbegin(), nextSibling );

	entries.emplace( nextSibling, ArrayNode{ combinedPath } );
	return NodeHandle{ insertIdx, combinedPath };
}

void Document::EmplaceProperty( NodeHandle node, std::string_view name, Primitive value )
{
	auto parentIter = Find( node );
	auto combinedPath = pool.Get( node.identifier, name );

	auto nextSibling = std::find_if( parentIter + 1, entries.cend(),
		[this, parentDepth = pool.GetDepth( node.identifier )]( const auto& entry )
		{
			return parentDepth == pool.GetDepth( GetPath( entry ) );
		} );

	entries.emplace( nextSibling, Property{ combinedPath, value } );
}

Document::Iterator Document::Find( NodeHandle node ) const
{
	if ( GetPath( entries[node.predictedIndex] ).idx == node.identifier.idx )
		return entries.cbegin() + node.predictedIndex;

	return std::find_if( entries.cbegin(), entries.cend(), [&node]( const auto& entry ) { return GetPath( entry ).idx == node.identifier.idx; } );
}