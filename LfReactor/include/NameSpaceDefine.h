//define namespace
//mlcai 2011-06-15

#ifndef NAMESPACEDEFINE_H_INCLUDED
#define NAMESPACEDEFINE_H_INCLUDED

#define NAMESPACE_NAME LfReactor

#define BEGIN_CXX_NAMESPACE_DEFINITION namespace NAMESPACE_NAME {

#define END_CXX_NAMESPACE_DEFINITION }

#define INC_NAME_HEADER(PATH, HEADER) PATH/HEADER

#define QUOTE_HEADER(H) #H

#define ADD_QUOTE(H) QUOTE_HEADER(H)

#if defined(WIN32)
	#if defined(WIN_EXPORT)
		#define DYNAMIC_LIB __declspec(dllexport)
	#else
		#define DYNAMIC_LIB __declspec(dllimport)
	#endif
#else
	#define DYNAMIC_LIB
#endif

#endif // NAMESPACEDEFINE_H_INCLUDED
