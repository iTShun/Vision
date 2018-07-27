#ifndef GraphicsContext_h
#define GraphicsContext_h

#include "Graphics/Exports.h"
#include "Core/Object.h"

#include <vector>

namespace graphics {

/** Base class for providing Windowing API agnostic access to creating and managing graphics context.*/
class API_GRAPHICS GraphicsContext : public core::Object
{
public:

	/** Return whether a valid and usable GraphicsContext has been created.*/
	virtual bool valid() const = 0;

public:

	virtual bool isSameKindAs(const Object* object) const { return dynamic_cast<const GraphicsContext*>(object) != 0; }
	virtual const char* libraryName() const { return "graphics"; }
	virtual const char* className() const { return "GraphicsContext"; }

protected:

	GraphicsContext();
	GraphicsContext(const GraphicsContext&, const core::CopyOp&);

	virtual ~GraphicsContext();

	virtual Object* cloneType() const { return 0; }
	virtual Object* clone(const core::CopyOp&) const { return 0; }

};

}

#endif /* GraphicsContext_h */