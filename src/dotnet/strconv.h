#ifndef _embedder_src_dotnet_strconv_h
#define _embedder_src_dotnet_strconv_h

#include <string>

std::wstring StringWide(std::string str);
std::string StringTight(std::wstring str);

#endif