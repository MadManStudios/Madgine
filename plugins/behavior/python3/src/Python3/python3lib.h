#pragma once

#include "Madgine/resourceslib.h"
#include "Madgine/rootlib.h"
#include "Madgine/debuglib.h"
#include "Madgine/behaviorlib.h"

#if defined(Python3_EXPORTS)
#    define MADGINE_PYTHON3_EXPORT DLL_EXPORT
#else
#    define MADGINE_PYTHON3_EXPORT DLL_IMPORT
#endif

#define PY_SSIZE_T_CLEAN
#define MS_NO_COREDLL
#define Py_ENABLE_SHARED

//Do not use the debug build of Python if it is not installed
#if defined(_DEBUG) && !defined(HAS_PYTHON3_DEBUG_LIB)
#    undef _DEBUG
#    define PY_UNDEFFED_DEBUG
#endif


#include "Python.h"

#ifdef PY_UNDEFFED_DEBUG
#    define _DEBUG
#endif

#include "python3compat.h"

#include "python3forward.h"
