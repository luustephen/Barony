#define BASE_DATA_DIR "./"
#if 1
	#define WINDOWS
#endif

#if 0
	#define APPLE
#endif

#if 0
	#define LINUX
#endif

#if 1
	#define STEAMWORKS
#endif

#if 1
	#define USE_FMOD
#endif

#if 0
	#define USE_OPENAL
#endif

#if 0
	#define USE_TREMOR
#endif

#if 0
	#define PANDORA
#endif

#if 0
	/*
	 * https://msdn.microsoft.com/en-us/library/cc230318.aspx
	 * A DWORD is a 32-bit unsigned integer (range: 0 through 4294967295 decimal). Because a DWORD is unsigned, its first bit (Most Significant Bit (MSB)) is not reserved for signing. 
	 * This type is declared as follows:
	 * typedef unsigned long DWORD, *PDWORD, *LPDWORD;
	 */
	typedef unsigned long DWORD;
#endif
