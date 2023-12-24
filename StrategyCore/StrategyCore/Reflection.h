#pragma once

#include <tuple>
#include <string>
#include <optional>
#include <functional>
#include <variant>

namespace Strategy::Reflection
{
	template<class T, class V, size_t... Idx>
	void VisitTupleImpl( T&& tuple, V&& visitor, std::index_sequence<Idx...> )
	{
		( visitor( std::get<Idx>( tuple ) ), ... );
	}

	template<class T, class V>
	void VisitTuple( T&& tuple, V&& visitor )
	{
		VisitTupleImpl( std::forward<T>( tuple ), std::forward<V>( visitor ), std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>() );
	}

	template<class Fn, class... M>
	struct AccessorDescription
	{
		constexpr AccessorDescription( Fn f, const M... m )
			: fn{ f }
			, metadata{ std::make_tuple( m... ) }
		{}

		auto operator()( auto& obj ) const
		{
			return fn( obj );
		}

		template<class V>
		struct CheckVisitor
		{
			void operator()( auto m )
			{
				if constexpr ( std::is_invocable_r_v<bool, V&, const decltype(m)&> )
					result |= v( m );
			}

			V& v;
			bool result = false;
		};

		template<class V>
		bool VisitMetadata( V& v ) const
		{
			CheckVisitor<V> visitor{ v };
			VisitTuple( metadata, visitor );
			return visitor.result;
		}

		std::tuple<M...> metadata;
		Fn fn;
	};
}

template<class T, class = void>
struct AccessorsWrapper;

template<class T>
struct AccessorsWrapper<T, std::void_t<typename T::AccessorsImpl>> { using Type = T::AccessorsImpl; };

template<class T, class = void>
struct FuncsWrapper;

template<class T>
struct FuncsWrapper<T, std::void_t<typename T::FuncsImpl>> { using Type = T::FuncsImpl; };

#define BEGIN_METADATA \
struct MetadataImpl \
{

#define NAME_METADATA( Name ) \
static constexpr const char* ClassName() { return #Name; }

#define END_METADATA \
};

#define BEGIN_ADAPT_ACCESSORS( Class )\
template<> struct AccessorsWrapper<Class, void> { struct Type { static constexpr auto Get(){ return std::make_tuple( 

#define END_ADAPT_ACCESSORS \
); } }; };

#define BEGIN_ACCESSORS \
struct AccessorsImpl { static constexpr auto Get() { return std::make_tuple(

#define END_ACCESSORS \
); } };

#define ACCESSOR( Name, ... ) \
std::make_tuple( #Name, Strategy::Reflection::AccessorDescription( []( auto& obj ){ return std::ref( obj.Name ); }, __VA_ARGS__ ) )

#define ACCESSOR_NAME( DisplayName, Name, ... ) \
std::make_tuple( #DisplayName, Strategy::Reflection::AccessorDescription( []( auto& obj ){ return std::ref( obj.Name ); }, __VA_ARGS__ ) )

#define BEGIN_FUNCS \
struct FuncsImpl { static constexpr auto Get() { return std::make_tuple(

#define END_FUNCS \
); } };

#define FUNC( Name ) \
std::make_tuple( #Name, []( auto& obj ) -> auto { return &std::decay_t<decltype( obj )>::Name; } )

template<class T>
using Accessors = typename AccessorsWrapper<T, void>::Type;

template<class T>
using Funcs = typename FuncsWrapper<T, void>::Type;

namespace Strategy::Reflection
{
	template<class T>
	concept HasAccessors = std::is_same_v<void, std::void_t<Accessors<std::decay_t<T>>>>;

	template<class T>
	concept HasFuncs = std::is_same_v<void, std::void_t<Funcs<std::decay_t<T>>>>;

	template <typename T>
	concept HasIdMember = std::is_same_v<void, std::void_t<decltype( std::decay_t<T>::MetadataImpl::Id() )>>;

	template<class T, class M>
	using MemberType = std::decay_t<decltype( std::get<1>( std::declval<M>() )(std::declval<T&>() ) )>;

	template<class V, class T>
	std::optional<V> GetValueType( const T& object, std::string_view memberName )
	{
		struct GetterVisitor
		{
			GetterVisitor( const T& o, std::string m )
				: memberName{ m }
				, obj{ o }
			{}

			void operator()( auto value )
			{
				if constexpr ( std::is_assignable_v<V, decltype( std::get<1>( value )( obj ) )> )
				{
					if ( memberName == std::get<0>( value ) && !result )
						result = static_cast<V>( std::get<1>( value )( obj ) );
				}
			}

			const T& obj;
			std::string_view memberName;
			std::optional<V> result;
		};

		GetterVisitor visitor{ memberName };
		VisitTuple( Accessors<T>::Get(), visitor );
		return visitor.result;
	}

	template<class T, class V>
	bool SetValueType( T& object, std::string_view memberName, V value )
	{
		struct SetterVisitor
		{
			SetterVisitor( T& o, std::string m, V v )
				: memberName{ m }
				, value{ v }
				, obj{ o }
			{}

			void operator()( auto& acc )
			{
				if constexpr ( std::is_assignable_v<MemberType<T, decltype( acc )>, V> )
				{
					if ( memberName == std::get<0>( acc ) && !result )
					{
						std::get<1>( acc )( obj ) = value;
						result = true;
					}
				}
			}

			T& obj;
			std::string_view memberName;
			V value;
			bool result;
		};

		SetterVisitor visitor{ memberName, value };
		VisitTuple( Accessors<T>::Get(), visitor );
		return visitor.result;
	}

	template<class V, class T>
	V* GetPointerType( T& object, std::string_view memberName )
	{
		struct GetterVisitor
		{
			GetterVisitor( const T& o, std::string m )
				: memberName{ m }
				, obj{ o }
				, result{ nullptr }
			{}

			void operator()( auto& acc )
			{
				if constexpr ( std::is_base_of_v<V, MemberType<T, decltype( acc )>> )
				{
					if ( memberName == std::get<0>( acc ) && !result )
						result = &std::get<1>( acc )( obj );
				}
			}

			const T& obj;
			std::string_view memberName;
			V* result;
		};

		GetterVisitor visitor{ memberName };
		VisitTuple( Accessors<T>::Get(), visitor );
		return visitor.result;
	}

	template<class T, class V>
	void TransformMember( T& obj, std::string memberName, V&& transform )
	{
		static_assert(HasAccessors<T>, "Class T must have AccessorsImpl defined in order to be reflected");

		auto transformVisitor = [&obj, &memberName, transform = std::forward<V>( transform )]( auto& acc ) mutable
		{
			if ( memberName == std::get<0>( acc ) )
				transform( std::get<1>( acc )(obj).get() );
		};

		VisitTuple( Accessors<T>::Get(), transformVisitor );
	}

	template<class T, class... Args>
	bool InvokeFunction( std::string functionName, Args&&... args )
	{
		static_assert(HasFuncs<T>, "Class T must have FuncsImpl defined in order to be invoked through reflection");

		bool found = false;
		auto invokeVisitor = [&found, &functionName, argTuple = std::forward_as_tuple( args... )]( auto& func )
		{
			if constexpr ( std::is_invocable_v<decltype(std::get<1>( func )), Args...> )
			{
				if ( functionName == std::get<0>( func ) )
				{
					std::apply( func, argTuple );
				}
			}
		};

		VisitTuple( Funcs<T>::Get(), invokeVisitor );
		return found;
	}

	template<class T>
	std::vector<std::string_view> ListMembers()
	{
		static_assert(HasAccessors<T>::value, "Class T must have AccessorsImpl defined in order to be reflected");

		std::vector<std::string_view> list;

		auto listVisitor = [&list]( auto& acc ) mutable
		{
			list.emplace_back( std::get<0>( acc ) );
		};

		VisitTuple( Accessors<T>::Get(), listVisitor );
		return list;
	}

	template<class T>
	std::vector<std::string_view> ListFreeFunctions()
	{
		static_assert(HasFuncs<T>::value, "Class T must have FuncsImpl defined in order to be invoked through reflection");

		std::vector<std::string_view> list;

		auto listVisitor = [&list]( auto& func )
		{
			if constexpr ( std::is_invocable_v<decltype(std::get<1>( func ))> )
				list.emplace_back( std::get<0>( func ) );
		};

		VisitTuple( Funcs<T>::Get(), listVisitor );
		return list;
	}

	template<class Attr, class T>
	constexpr bool HasAttribute( T t )
	{
		auto callableWithId = []( const Attr& ) { return true; };
		return t.VisitMetadata( callableWithId );
	}

	template<class Attr, class T>
	std::optional<decltype(std::declval<Attr&>().value)> TryFindValueAttr( T t )
	{
		decltype(std::declval<Attr&>().value) result;
		auto get = [&result]( const Attr& a )
		{
			result = a.value;
			return true;
		};

		if ( t.VisitMetadata( get ) )
		{
			return result;
		}

		return std::nullopt;
	}
	
	template<class V, class T, size_t I>
	constexpr std::optional<size_t> VariantFindImpl()
	{
		if constexpr ( std::variant_size_v<V> == I )
			return std::nullopt;
		else if constexpr ( std::is_same_v<std::variant_alternative_t<I, V>, std::decay_t<T>> )
			return I;
		else
			return VariantFindImpl<V, T, I + 1>();
	}

	template<class V, class T>
	constexpr std::optional<size_t> VariantFind()
	{
		return VariantFindImpl<V, T, 0>();
	}

	template<class V, size_t I>
	void DefaultInitVariantFromIndexImpl( size_t index, V& v )
	{
		if constexpr ( I < std::variant_size_v<V> )
		{
			if ( index == I )
				v = std::variant_alternative_t<I, V>();
			else
				DefaultInitVariantFromIndexImpl<V, I + 1>( index, v );
		}
	}

	template<class V>
	V DefaultInitVariantFromIndex( size_t index )
	{
		V v;
		DefaultInitVariantFromIndexImpl<V, 0>( index, v );
		return std::move( v );
	}
}