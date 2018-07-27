#ifndef Object_h
#define Object_h

#include "Core/Referenced.h"
#include "Core/ref_ptr.h"
#include "Core/CopyOp.h"
#include "Core/Notify.h"

#include <string>
#include <vector>

namespace core {
    
#define _ADDQUOTES(def) #def
#define ADDQUOTES(def) _ADDQUOTES(def)

	/** META_Object macro define the standard clone, isSameKindAs and className methods.
	* Use when subclassing from Object to make it more convenient to define
	* the standard pure virtual clone, isSameKindAs and className methods
	* which are required for all Object subclasses.*/
#define META_Object(library,name) \
        virtual core::Object* cloneType() const { return new name (); } \
        virtual core::Object* clone(const core::CopyOp& copyop) const { return new name (*this,copyop); } \
        virtual bool isSameKindAs(const core::Object* obj) const { return dynamic_cast<const name *>(obj)!=NULL; } \
        virtual const char* libraryName() const { return #library; }\
        virtual const char* className() const { return #name; }

/** Base class/standard interface for objects which require IO support,
cloning and reference counting.
Based on GOF Composite, Prototype and Template Method patterns.
*/
class API_CORE Object : public Referenced
{
public:


	/** Construct an object. Note Object is a pure virtual base class
	and therefore cannot be constructed on its own, only derived
	classes which override the clone and className methods are
	concrete classes and can be constructed.*/
	inline Object() :Referenced() {}

	inline explicit Object(bool threadSafeRefUnref) :Referenced(threadSafeRefUnref) {}

	/** Copy constructor, optional CopyOp object can be used to control
	* shallow vs deep copying of dynamic data.*/
	Object(const Object&, const CopyOp& copyop = CopyOp::kSHALLOW_COPY);

	/** Clone the type of an object, with Object* return type.
	Must be defined by derived classes.*/
	virtual Object* cloneType() const = 0;

	/** Clone an object, with Object* return type.
	Must be defined by derived classes.*/
	virtual Object* clone(const CopyOp&) const = 0;

	virtual bool isSameKindAs(const Object*) const { return true; }


	/** return the name of the object's library. Must be defined
	by derived classes. The OpenSceneGraph convention is that the
	namespace of a library is the same as the library name.*/
	virtual const char* libraryName() const = 0;

	/** return the name of the object's class type. Must be defined
	by derived classes.*/
	virtual const char* className() const = 0;

	/** return the compound class name that combines the library name and class name.*/
	std::string getCompoundClassName() const { return std::string(libraryName()) + std::string("::") + std::string(className()); }


	/** Set whether to use a mutex to ensure ref() and unref() are thread safe.*/
	virtual void setThreadSafeRefUnref(bool threadSafe);

	/** Set the name of object using C++ style string.*/
	virtual void setName(const std::string& name) { _name = name; }

	/** Set the name of object using a C style string.*/
	inline void setName(const char* name)
	{
		if (name) setName(std::string(name));
		else setName(std::string());
	}

	/** Get the name of object.*/
	inline const std::string& getName() const { return _name; }


protected:

	/** Object destructor. Note, is protected so that Objects cannot
	be deleted other than by being dereferenced and the reference
	count being zero (see core::Referenced), preventing the deletion
	of nodes which are still in use. This also means that
	Nodes cannot be created on stack i.e Node node will not compile,
	forcing all nodes to be created on the heap i.e Node* node
	= new Node().*/
	virtual ~Object();

	std::string _name;

private:

	/** disallow any copy operator.*/
	Object & operator = (const Object&) { return *this; }
};

template<typename T>
T* clone(const T* t, const core::CopyOp& copyop = core::CopyOp::kSHALLOW_COPY)
{
	if (t)
	{
		core::ref_ptr<core::Object> obj = t->clone(copyop);

		T* ptr = dynamic_cast<T*>(obj.get());
		if (ptr)
		{
			obj.release();
			return ptr;
		}
		else
		{
			LOG_WARN << "Warning: core::clone(const T*, core::CopyOp&) cloned object not of type T, returning NULL." << std::endl;
			return 0;
		}
	}
	else
	{
		LOG_WARN << "Warning: core::clone(const T*, core::CopyOp&) passed null object to clone, returning NULL." << std::endl;
		return 0;
	}
}

template<typename T>
T* clone(const T* t, const std::string& name, const core::CopyOp& copyop = core::CopyOp::kSHALLOW_COPY)
{
	T* newObject = core::clone(t, copyop);
	if (newObject)
	{
		newObject->setName(name);
		return newObject;
	}
	else
	{
		LOG_WARN << "Warning: core::clone(const T*, const std::string&, const core::CopyOp) passed null object to clone, returning NULL." << std::endl;
		return 0;
	}
}

template<typename T>
T* cloneType(const T* t)
{
	if (t)
	{
		core::ref_ptr<core::Object> obj = t->cloneType();

		T* ptr = dynamic_cast<T*>(obj.get());
		if (ptr)
		{
			obj.release();
			return ptr;
		}
		else
		{
			LOG_WARN << "Warning: core::cloneType(const T*) cloned object not of type T, returning NULL." << std::endl;
			return 0;
		}
	}
	else
	{
		LOG_WARN << "Warning: core::cloneType(const T*) passed null object to clone, returning NULL." << std::endl;
		return 0;
	}
}
    
}

#endif /* Object_h */
