#pragma once

#include <type_traits>
#include <array>
#include <tuple>

namespace Strategy
{
	class EditorState;

	namespace Concepts
	{

		template<class T, class U>
		struct BiConvertible : std::conjunction<std::is_convertible<T, U>, std::is_convertible<U, T>> {};

		template<class T>
		concept HasUpdate = std::is_same_v<void, std::void_t<decltype(std::declval<T>().Update( 0.0f ))>>;

		template<class T>
		concept HasClose = std::is_same_v<bool, decltype(std::declval<T>().Closed())>;

		template<class T>
		concept HasEmplace = std::is_same_v<void, std::void_t<decltype(std::declval<T>().emplace_back())>>;

		template<class T>
		concept HasErase = std::is_same_v<void, std::void_t<decltype(std::declval<T>().erase( std::declval<std::ranges::iterator_t<T>>() ))>>;

		template<class T>
		concept StringLike = std::is_same_v<char, std::decay_t<decltype(std::declval<T&>()[0])>>;

		template<class T>
		constexpr bool NonStringRange = std::ranges::range<T> && !StringLike<T>;

		template<class T>
		constexpr bool ResizableRange = std::ranges::range<T> && HasEmplace<T>;

		template<class T>
		constexpr bool FixedRange = std::ranges::range<T> && !StringLike<T> && !HasEmplace<T>;

		template<class T>
		concept CustomSerialized = std::is_same_v<std::void_t<typename T::CustomSerializeTag>, void>;

		template<class T>
		struct TypeTag { using Type = T; };

		template<class... Args>
		using CompatibleTypes = decltype(std::make_tuple( std::declval<TypeTag<Args>>()... ));
	}
}