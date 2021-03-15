/*** temporary file ***
	Must Be Revised 
*/
#ifndef __INIPARSER_H__
#define __INIPARSER_H__

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <stdexcept>
#include <cmath>
#include "Common.h"

using namespace std;

class IniParser {
public:
	IniParser(string path);
	~IniParser();
	uint64_t gbsize; //Global Buffer size
	uint64_t xbsize; //Small X Buffer size
	uint64_t tot_acc; //Total number of accelerator
	string outputfilename;

private:
	map<string, string> m_table;
	bool Contain(string name);
	bool GetBool(string name);
	string GetString(string name);
	float GetFloat(string name);
	int GetInt(string name);
	uint64_t GetUint64(string name);
	void ParseIni();
	bool ReadIni(string path);
};

#endif