#pragma once

#include "pch.h"
#include "Reflection.h"
#include "Concepts.h"

#include <memory>
#include <optional>
#include <variant>

namespace Strategy
{
	class SerializeBuffer
	{
	public:

		enum class State
		{
			Read,
			Write
		};

		void PrepareWrite()
		{
			ReadWriteState = State::Write;
			Position = 0;
			Data.clear();
		}

		void PrepareRead()
		{
			ReadWriteState = State::Read;
			Position = 0;
		}

		void WriteInt( int32_t i )
		{
			EnsureReadyForWrite( 4 );
			std::memcpy( Data.data() + Position, &i, 4 );
			Position += 4;
		}

		int32_t ReadInt()
		{
			int32_t i;
			std::memcpy( &i, Data.data() + Position, 4 );
			Position += 4;
			return i;
		}

		void WriteBool( bool b )
		{
			EnsureReadyForWrite( 1 );
			Data[Position] = b ? 1 : 0;
			Position += 1;
		}

		int32_t ReadBool()
		{
			bool b = Data[Position] != 0;
			Position += 1;
			return b;
		}

	private:

		void EnsureReadyForWrite( size_t bytes )
		{
			if ( Data.size() < Position + bytes )
				Data.resize( Position + bytes, 0 );
		}

		State ReadWriteState = State::Write;
		size_t Position = 0;
		std::vector<uint8_t> Data;
	};

	namespace Detail
	{
		template<class T>
		struct BinarySerializer;

		template<class T>
		void BinaryWriteInternal( const T& obj, SerializeBuffer& buf )
		{
			BinarySerializer<std::decay_t<decltype(obj)>>::Write( obj, buf );
		}

		template<class T>
		void BinaryReadInternal( T& obj, SerializeBuffer& buf )
		{
			return BinarySerializer<std::decay_t<decltype(obj)>>::Read( obj, buf );
		}

		template<>
		struct BinarySerializer<int32_t>
		{
			static void Write( const int32_t& obj, SerializeBuffer& buf )
			{
				buf.WriteInt( obj );
			}

			static void Read( int32_t& obj, SerializeBuffer& buf )
			{
				obj = buf.ReadInt();
			}
		};

		template<>
		struct BinarySerializer<bool>
		{
			static void Write( const bool& obj, SerializeBuffer& buf )
			{
				buf.WriteBool( obj );
			}

			static void Read( bool& obj, SerializeBuffer& buf )
			{
				obj = buf.ReadBool();
			}
		};

		template<class T>
		struct BinarySerializer<std::optional<T>>
		{
			static void Write( const std::optional<T>& obj, SerializeBuffer& buf )
			{
				buf.WriteBool( static_cast<bool>(obj) );
				if ( obj )
					BinaryWrite( *obj, buf );
			}

			static void Read( std::optional<T>& obj, SerializeBuffer& buf )
			{
				obj = {};

				if ( buf.ReadBool() )
				{
					obj = T{};
					BinaryRead( obj, buf );
				}
			}
		};

		template<class T>
		struct BinarySerializer<std::unique_ptr<T>>
		{
			static void Write( const std::unique_ptr<T>& obj, SerializeBuffer& buf )
			{
				buf.WriteBool( static_cast<bool>(obj) );
				if ( obj )
					BinaryWrite( *obj, buf );
			}

			static void Read( std::unique_ptr<T>& obj, SerializeBuffer& buf )
			{
				obj.reset();

				if ( buf.ReadBool() )
				{
					obj = std::make_unique<T>();
					BinaryRead( obj, buf );
				}
			}
		};

		template<class T>
			requires(Concepts::NonStringRange<T>)
		struct BinarySerializer<T>
		{
			static void Write( const T& obj, SerializeBuffer& buf )
			{
				buf.WriteInt( static_cast<int32_t>(std::ranges::size( obj )) );

				for ( const auto& e : obj )
					BinaryWrite( e, buf );
			}

			static void Read( T& obj, SerializeBuffer& buf )
			{
				int32_t size = buf.ReadInt();

				for ( int32_t i = 0; i < size; ++i )
				{
					if constexpr ( Concepts::ResizableRange<T> )
					{
						BinaryRead( obj.emplace_back(), buf );
					}
					else if constexpr ( Concepts::FixedRange<T> )
					{
						BinaryRead( obj[i], buf );
					}
				}
			}
		};

		template<class... Alts>
		struct BinarySerializer<std::variant<Alts...>>
		{
			using SerializerTag = void;

			static void Write( const std::variant<Alts...>& obj, SerializeBuffer& buf )
			{
				buf.WriteInt( static_cast<int32_t>(obj.index()) );
				std::visit( [&buf]( const auto& o ) { BinaryWrite( o, buf ); }, obj );
			}

			static void Read( std::variant<Alts...>& obj, SerializeBuffer& buf )
			{
				obj = Reflection::DefaultInitVariantFromIndex<std::variant<Alts...>>( static_cast<size_t>(buf.ReadInt()) );
				std::visit( [&buf]( auto& o ) { BinaryRead( o, buf ); }, obj );
			}
		};

		template<class T> requires Reflection::HasAccessors<T>
		struct BinarySerializer<T>
		{
			static void Write( const T& obj, SerializeBuffer& buf )
			{
				auto visitor = [&obj, &buf]( const auto& accessor )
					{
						BinarySerializer<std::decay_t<decltype(std::get<1>( accessor )(obj).get())>>::Write( std::get<1>( accessor )(obj), buf );
					};

				Reflection::VisitTuple( Accessors<T>::Get(), visitor );
			}

			static void Read( T& obj, SerializeBuffer& buf )
			{
				auto visitor = [&obj, &buf]( const auto& accessor )
					{
						BinarySerializer<std::decay_t<decltype(std::get<1>( accessor )(obj).get())>>::Read( std::get<1>( accessor )(obj), buf );
					};

				Reflection::VisitTuple( Accessors<T>::Get(), visitor );
			}
		};

		template<class T> requires Concepts::CustomSerialized<T>
		struct BinarySerializer<T>
		{
			static void Write( const T& obj, SerializeBuffer& buf )
			{
				T::Serialize( obj, buf );
			}

			static void Read( T& obj, SerializeBuffer& buf )
			{
				T::Deserialize( obj, buf );
			}
		};
	}

	template<class T>
	void BinaryWrite( const T& obj, SerializeBuffer& buf )
	{
		buf.PrepareWrite();
		Detail::BinaryWriteInternal( obj, buf );
	}

	template<class T>
	void BinaryRead( T& obj, SerializeBuffer& buf )
	{
		buf.PrepareRead();
		Detail::BinaryReadInternal( obj, buf );
	}
}