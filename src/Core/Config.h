#ifndef Config_h
#define Config_h

//#define NOTIFY_DISABLED
//#define USE_FLOAT_MATRIX
//#define USE_FLOAT_PLANE
//#define USE_FLOAT_BOUNDINGSPHERE
//#define USE_FLOAT_BOUNDINGBOX
//#define USE_REF_PTR_IMPLICIT_OUTPUT_CONVERSION
//#define USE_UTF8_FILENAME
//#define DISABLE_MSVC_WARNINGS
#define CORE_LIBRARY_STATIC

#define INIT_SINGLETON_PROXY(ProxyName, Func) static struct ProxyName{ ProxyName() { Func; } } s_##ProxyName;

#endif /* Config_h */
