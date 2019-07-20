/*
 *  Created by Phil Nash on 04/03/2012.
 *  Copyright (c) 2012 Two Blue Cubes Ltd. All rights reserved.
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef TWOBLUECUBES_CATCH_MATCHERS_HPP_INCLUDED
#define TWOBLUECUBES_CATCH_MATCHERS_HPP_INCLUDED

#include "catch_common.h"

#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

namespace Catch {
namespace Matchers {
    namespace Impl {
        template <bool C, typename T = void>
        using EnableIfT = typename std::enable_if<C, T>;

        class MatcherUntypedBase {
        public:
            MatcherUntypedBase() = default;
            MatcherUntypedBase ( MatcherUntypedBase const& ) = default;
            MatcherUntypedBase& operator = ( MatcherUntypedBase const& ) = delete;
            std::string toString() const;

        protected:
            virtual ~MatcherUntypedBase();
            virtual std::string describe() const = 0;
            mutable std::string m_cachedToString;
        };

        template <typename T>
        struct IsMatcher : std::is_base_of<MatcherUntypedBase, T> {};

#ifdef __clang__
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

        template<typename ObjectT>
        struct MatcherMethod {
            virtual bool match( ObjectT const& arg ) const = 0;
        };

#ifdef __clang__
#    pragma clang diagnostic pop
#endif

        template<typename T>
        struct MatcherBase : MatcherUntypedBase, MatcherMethod<T> {
        };

        template <typename... Ms>
        class MatchAllOf : public MatcherUntypedBase {
          private:
            static constexpr size_t N = sizeof...(Ms);

            template <size_t I>
            struct Iter {
              template <typename ArgT>
              bool match( ArgT const& a ) const {
                  if (!std::get<I>( m.m_matchers ).match( a ))
                    return false;
                  return Iter<I+1>{ m }.match( a );
              }
              void describe( std::string& description ) const {
                  if (I > 0)
                    description += " and ";
                  description += std::get<I>( m.m_matchers ).toString();
                  return Iter<I + 1>{ m }.describe( description );
              }
              MatchAllOf const& m;
            };
            template <>
            struct Iter<N> {
              Iter( MatchAllOf const& ) {}
              template <typename ArgT>
              bool match( ArgT const& ) const {
                return true;
              }
              void describe( std::string& ) const {}
            };

          public:
            MatchAllOf( Ms const& ...ms ) : m_matchers( ms... ) {}

            template <typename ArgT>
            bool match( ArgT const& arg ) const {
              return Iter<0>{ *this }.match( arg );
            }

            std::string describe() const override {
                std::string description;
                description.reserve( 4 + N*32 );
                description += "( ";
                Iter<0>{ *this }.describe( description );
                description += " )";
                return description;
            }

            std::tuple<Ms...> m_matchers;
        };

        template <typename... Ms>
        class MatchAnyOf : public MatcherUntypedBase {
          private:
            static constexpr size_t N = sizeof...(Ms);

            template <size_t I>
            struct Iter {
              template <typename ArgT>
              bool match( ArgT const& a ) const {
                if ( std::get<I>( m.m_matchers ).match( a ) )
                  return true;
                return Iter<I + 1>{ m }.match( a );
              }
              void describe( std::string& description ) const {
                  if ( I > 0 )
                    description += " or ";
                  description += std::get<I>( m.m_matchers ).toString();
                  return Iter<I + 1>{ m }.describe( description );
              }
              const MatchAnyOf& m;
            };
            template <>
            struct Iter<N> {
              Iter( const MatchAnyOf& ) {}
              template <typename ArgT>
              bool match( ArgT const& ) const { return false; }
              void describe( std::string& ) const { }
            };

          public:
            MatchAnyOf( const Ms& ...ms ) : m_matchers( ms... ) {}

            template <typename ArgT>
            bool match( ArgT const& arg ) const {
              return Iter<0>{ *this }.match( arg );
            }

            std::string describe() const override {
                std::string description;
                description.reserve( 4 + N*32 );
                description += "( ";
                Iter<0>{ *this }.describe( description );
                description += " )";
                return description;
            }

            std::tuple<Ms...> m_matchers;
        };

        template <typename M>
        class MatchNotOf : public MatcherUntypedBase {
        public:
          explicit MatchNotOf( M const& m ) : m_matcher(m) {}

          std::string describe() const override {
            return "not " + m_matcher.describe();
          }
          template <typename T>
          bool match( T const& v ) const {
            return !m_matcher.match( v );
          }

        private:
          M m_matcher;
        };

        template <typename M>
        MatchNotOf<M> Not( M const& m ) {
            return MatchNotOf<M>( m );
        }

        template <typename... Ms>
        MatchAllOf<Ms...> And( Ms const& ...ms ) {
          return MatchAllOf<Ms...>( ms... );
        }

        template <typename... Ms>
        MatchAnyOf<Ms...> Or( Ms const& ...ms ) {
          return MatchAnyOf<Ms...>( ms... );
        }

        template<typename M,
                 typename = EnableIfT<IsMatcher<M>::value>>
        auto operator ! ( M const& m ) -> decltype( Not( m ) ) {
          return Not( m );
        }

        template <typename M1, typename M2,
                  typename = EnableIfT<IsMatcher<M1>::value &&
                                       IsMatcher<M2>::value>>
        auto operator && ( M1 const& m1, M2 const& m2 ) -> decltype( And( m1, m2 ) ) {
          return And( m1, m2 );
        }

        template <typename M1, typename M2,
                  typename = EnableIfT<IsMatcher<M1>::value &&
                                       IsMatcher<M2>::value>>
        auto operator || ( M1 const& m1, M2 const& m2 ) -> decltype( Or( m1, m2 ) ) {
          return Or( m1, m2 );
        }

    } // namespace Impl

    using Matchers::Impl::Not;
    using Matchers::Impl::And;
    using Matchers::Impl::Or;

} // namespace Matchers

using namespace Matchers;
using Matchers::Impl::MatcherBase;

} // namespace Catch

#endif // TWOBLUECUBES_CATCH_MATCHERS_HPP_INCLUDED
