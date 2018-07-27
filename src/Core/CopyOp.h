#ifndef CopyOp_h
#define CopyOp_h

#include "Core/Exports.h"

namespace core {

class Referenced;
class Object;

/** Copy Op(erator) used to control whether shallow or deep copy is used
* during copy construction and clone operation.*/
class API_CORE CopyOp
{

public:

	enum Options
	{
		kSHALLOW_COPY = 0,
		kDEEP_COPY_OBJECTS = 1 << 0,

		kDEEP_COPY_ALL = 0x7FFFFFFF
	};

	typedef unsigned int CopyFlags;

	inline CopyOp(CopyFlags flags = kSHALLOW_COPY) :_flags(flags) {}
	virtual ~CopyOp() {}

	void setCopyFlags(CopyFlags flags) { _flags = flags; }
	CopyFlags getCopyFlags() const { return _flags; }

	virtual Referenced*     operator() (const Referenced* ref) const;
	virtual Object*         operator() (const Object* obj) const;

protected:

	CopyFlags _flags;
};

}

#endif /* CopyOp_h */