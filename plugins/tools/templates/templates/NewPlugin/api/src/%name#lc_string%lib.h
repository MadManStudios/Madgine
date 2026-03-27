#pragma once



#if defined(%name%_EXPORTS)
#    define %author#uc_string%_%name#uc_string%_EXPORT DLL_EXPORT
#else
#    define %author#uc_string%_%name#uc_string%_EXPORT DLL_IMPORT
#endif
