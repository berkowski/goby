// copyright 2010 t. schneider tes@mit.edu
// ocean engineering graudate student - mit / whoi joint program
// massachusetts institute of technology (mit)
// laboratory for autonomous marine sensing systems (lamss)    
// 
// this file is part of goby-util, a collection of utility libraries
//
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this software.  If not, see <http://www.gnu.org/licenses/>.

#ifndef STRING20100713H
#define STRING20100713H

#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <limits>

#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/logical.hpp>
#include <boost/numeric/conversion/cast.hpp>


namespace goby
{

    namespace util
    {   
        /// \name goby::util::as, a "do-the-right-thing" type casting tool
        //@{
        
        template<typename To>
            typename boost::enable_if<boost::is_arithmetic<To>, To>::type
            _as_from_string(const std::string& from)
        {
            try { return boost::lexical_cast<To>(from); }
            catch(boost::bad_lexical_cast&)
            {
                // return NaN or maximum value supported by the type
                return std::numeric_limits<To>::has_quiet_NaN
                    ? std::numeric_limits<To>::quiet_NaN()
                    : std::numeric_limits<To>::max();
            }
        }

        // only works properly for enums with a defined 0 value
        template<typename To>
            typename boost::enable_if<boost::is_enum<To>, To>::type
            _as_from_string(const std::string& from)
        {
            try { return static_cast<To>(boost::lexical_cast<int>(from)); }
            catch(boost::bad_lexical_cast&)
            {
                return static_cast<To>(0);
            }
        }

        
        template<typename To>
            typename boost::enable_if<boost::is_class<To>, To>::type
            _as_from_string(const std::string& from)
        {
            try { return boost::lexical_cast<To>(from); }
            catch(boost::bad_lexical_cast&)
            {
                return To();
            }
        }

        template <>
            inline bool _as_from_string<bool>(const std::string& from)
        {
            return (boost::iequals(from, "true") || boost::iequals(from, "1"));
        }        

        template <>
            inline std::string _as_from_string<std::string>(const std::string& from)
        {
            return from;
        }
        
        template<typename To, typename From>
            std::string _as_to_string(const From& from)
        {
            try { return boost::lexical_cast<std::string>(from); }
            catch(boost::bad_lexical_cast&)
            {
                return std::string();
            }
        }
        
        /// specialization of as() for bool -> string
        template <>
            inline std::string _as_to_string<std::string, bool>(const bool& from)
        {
            return from ? "true" : "false";
        }

        template <>
            inline std::string _as_to_string<std::string, std::string>(const std::string& from)
        {
            return from;
        }
        
        template<typename To, typename From>
            typename boost::disable_if<boost::is_same<To,From>, To>::type
            _as_numeric(const From& from)
        {
            try { return boost::numeric_cast<To>(from); }
            catch(boost::bad_numeric_cast&)
            {
                // return NaN or maximum value supported by the type
                return std::numeric_limits<To>::has_quiet_NaN
                    ? std::numeric_limits<To>::quiet_NaN()
                    : std::numeric_limits<To>::max();
            }
        }        
        
        template<typename To, typename From>
            typename boost::enable_if<boost::is_same<To,From>, To>::type
            _as_numeric(const From& from)
        {
            return from;
        }

        template<typename To>
            To as(const std::string& from)
        {
            return _as_from_string<To>(from);
        }

        template<typename To, typename From>
            typename boost::enable_if<boost::is_same<To, std::string>, To>::type
            as(const From& from)
        {
            return _as_to_string<To,From>(from);
        }

        template<typename To, typename From>
            typename boost::enable_if<boost::mpl::and_<boost::is_arithmetic<To>,
            boost::is_arithmetic<From> >, To>::type
            as(const From& from)
        {
            return _as_numeric<To, From>(from);
        }

        // not much better we can do for enums than static cast them ...
        template<typename To, typename From>
            typename boost::enable_if<boost::mpl::and_<boost::is_enum<To>,
            boost::is_arithmetic<From> >, To>::type
            as(const From& from)
        {
            return static_cast<To>(from);
        }

    }
}
#endif
