#ifndef TBIN_PROPERTYVALUE_HPP
#define TBIN_PROPERTYVALUE_HPP

#include <map>
#include <SFML/Config.hpp>
#include <string>

namespace tbin
{
    struct PropertyValue
    {
        public:
            enum Type
            {
                Bool    = 0,
                Integer = 1,
                Float   = 2,
                String  = 3,
            };

            Type type;
            union
            {
                bool b;
                sf::Int32 i;
                float f;
            } data;
            std::string dataStr;
    };

    typedef std::map< std::string, PropertyValue > Properties;
}

#endif // TBIN_PROPERTYVALUE_HPP
