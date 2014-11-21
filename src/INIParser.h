#ifndef TR_INI_PARSER_H
#define TR_INI_PARSER_H

#include <string>
#include "minINI/minIni.h"

class INIParser
{
	public:
		INIParser(const char* Filename);
		~INIParser();

		void        SetFile(  const char* Filename);
		bool        GetBool(  const char* Section, const char* Key, bool Default = false);
		uint32_t    GetInt(   const char* Section, const char* Key, uint32_t Default = 0);
		uint64_t    GetLong(  const char* Section, const char* Key, uint64_t Default = 0);
		std::string GetString(const char* Section, const char* Key, const char* Default = "");

	private:
		std::string File;
};

#endif
