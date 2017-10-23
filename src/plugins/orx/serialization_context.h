#ifndef SERIALIZATION_CONTEXT_H
#define SERIALIZATION_CONTEXT_H

#include <QString>
#include <vector>

namespace Orx {

struct SerializationContext
{
    SerializationContext() :
        m_CondenseGraphics(true)
    {}

    // If true, when an Atlas has only one graphic, the generated graphic object will
    // contain the texture path and no atlas object will be generated.
    bool    m_CondenseGraphics;
};


}

#endif // SERIALIZATION_CONTEXT_H
