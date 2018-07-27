#include "CopyOp.h"
#include "Core/Object.h"

namespace core {

#define COPY_OP( TYPE, FLAG ) \
TYPE* CopyOp::operator() (const TYPE* obj) const \
{ \
    if (obj && _flags&FLAG) \
        return core::clone(obj, *this); \
    else \
        return const_cast<TYPE*>(obj); \
}

COPY_OP(Object, kDEEP_COPY_OBJECTS)

Referenced* CopyOp::operator() (const Referenced* ref) const
{
	return const_cast<Referenced*>(ref);
}

}