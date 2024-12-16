#ifndef TYPESDEF_H_
#define TYPESDEF_H_

#include <vector>

#ifndef VFLOAT
#define VFLOAT
typedef std::vector< float > vfloat;
#endif

#ifndef VVFLOAT
#define VVFLOAT
typedef std::vector< vfloat > vvfloat;
#endif

#ifndef V1FLOAT
#define V1FLOAT
typedef std::vector< float > v1float;
#endif

#ifndef V2FLOAT
#define V2FLOAT
typedef std::vector< v1float > v2float;
#endif

#ifndef V3FLOAT
#define V3FLOAT
typedef std::vector< v2float > v3float;
#endif

#ifndef V4FLOAT
#define V4FLOAT
typedef std::vector< v3float > v4float;
#endif

#ifndef VINT
#define VINT
typedef std::vector< int > vint;
#endif

#ifndef UINT
#define UINT
typedef unsigned int uint;
#endif

#ifndef VUINT
#define VUINT
typedef std::vector< uint > vuint;
#endif

#endif
