#include "Object.h"

namespace core {
    
Object::Object(const Object& obj, const CopyOp& copyop) :
	Referenced(),
	_name(obj._name)
{
	
}

Object::~Object()
{
}

void Object::setThreadSafeRefUnref(bool threadSafe)
{
	Referenced::setThreadSafeRefUnref(threadSafe);
}
    
}
