#ifndef _Prototypes_Hpp
#define _Prototypes_Hpp

int ProcessMudFunctions();
int MainLoop();

const char *Format ( const char *fmt, ... );
bool SameString ( const std::string &a, const std::string &b );
bool SameString ( const char *astr, const char *bstr );
bool SameString ( const char *astr, const std::string bstr );
bool SameString ( const std::string &astr, const char *bstr );
bool IsSameList ( const std::string &nameToFind, const std::string &namelist );
std::string SmashTilde ( std::string str );
std::string FixAmp ( const std::string &str );
std::string RemoveChar ( std::string str, char v );
char* CapitalizeC ( const char *str );
char *ChopC ( const char *argument, char *first_arg );
std::string ChopString ( const std::string &argument, std::string &first );
const char *StripNewLine ( const char *str );
std::string CapitalSentence ( const std::string &str );
std::string GetArg ( const std::string &str, int arg );
std::string GetAfter ( const std::string &str, int arg );
const char *SmashSpace ( const std::string &str_sm );

#endif
