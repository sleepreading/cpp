#pragma once
#include <string>
#include <vector>

class StringSpliter : public std::vector<std::string>
{
public:
	StringSpliter(const char* s, const char* sp) {
		std::string sTemp(""); 
		long len = (long)strlen(sp);
		for (long i=0; s[i]!=0; i++) {
			if (!strncmp(s+i, sp, len)) {
				if (sTemp.size() > 0)
					push_back(sTemp);
				else
					push_back("null");
				i += len-1;
				sTemp = "";
				continue;
			}
			sTemp += s[i];
		}
		push_back( sTemp=="" ? "null" : sTemp);
	};
};

