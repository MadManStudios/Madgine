#pragma once

#include "Generic/genericlib.h"
#include "Meta/metalib.h"

#if defined(CodeGen_EXPORTS)
#    define MADGINE_CODEGEN_EXPORT DLL_EXPORT
#else
#    define MADGINE_CODEGEN_EXPORT DLL_IMPORT
#endif

#include "codegenforward.h"

#include <sstream>
#include <stack>
#include <variant>