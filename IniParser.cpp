/*** temporary file ***
	Must Be Revised 
*/
#include "IniParser.h"

using namespace std;

uint64_t CACHE_LINE_BYTE; // 한번에 읽을 수 있는 cache line byte 크기 (일반적으로 64)
uint64_t CACHE_LINE_COUNT; // 한번 캐시라인에 (64/8 = 8... 유효숫자 8)
uint64_t UNIT_BYTE; // cachelinebyte/cachelinecnt

uint64_t A_COL_START;  
uint64_t A_ROW_START;  
uint64_t X_START;  
uint64_t WEIGHT_START; 
uint64_t XW_START;
uint64_t AXW_START;

int SYSTOLIC_DIM;

int MAX_DRAM;
// 교체 알고리즘 선정
uint64_t CACHE_REPL_ALGO;

int DIVISION_TYPE;
int MECHA_TYPE;

int UNIT_W_READ;

string MEM_TYPE;

// Bloom Filter
uint64_t BLOOM_FILTER_NUM;
uint64_t BF_OVERLAP;
float BF_RATIO;
uint64_t CACHE_WAY_N;

// Moving Median
uint64_t MOV_MED_BLOCK_NUM;

IniParser::IniParser(string path) { ReadIni(path); }

IniParser::~IniParser() {}

bool IniParser::ReadIni(string path)
{
	ifstream openFile(path);
	if (openFile.is_open()) {
		string line;
		while (getline(openFile, line)) {
			string delimiter = " = ";
			if (string::npos == line.find(delimiter)) 
				delimiter = "=";
			string token1 = line.substr(0, line.find(delimiter));
			string token2 = line.substr(line.find(delimiter)+delimiter.length(), line.length());
			m_table[token1] = token2;
		}
		openFile.close();
		ParseIni();
		return true;
	}
	else {
		throw invalid_argument("Cannot open inifile");
	}
}

bool IniParser::Contain(string name)
{
	if (m_table.find(name) == m_table.end()) 
		return false;
	return true;
}

bool IniParser::GetBool(string name)
{
	if (Contain(name)) {
		if (m_table[name][0] == 't' || m_table[name][0] == 'T')
			return true;
		else
			return false;
	}
	else {
		throw invalid_argument("Not exist.");
	}
}

string IniParser::GetString(string name)
{
	if (Contain(name)) {
		if (m_table[name].find("\"") == string::npos)
			return m_table[name];
		else
			return m_table[name].substr(1, m_table[name].length() - 2);
	}
	else {
		throw invalid_argument("Not exist.");
	}
}

float IniParser::GetFloat(string name)
{
	if (Contain(name))
		return stof(m_table[name]);
	else
		throw invalid_argument("Not exist.");
}

int IniParser::GetInt(string name)
{
	if (Contain(name))
		return stoi(m_table[name]);
	else
		throw invalid_argument("Not exist.");
}

uint64_t IniParser::GetUint64(string name)
{
	if (Contain(name))
		return stoull(m_table[name], NULL, 0);
	else
		throw invalid_argument("Not exist.");
}

void IniParser::ParseIni()
{
	gbsize = GetUint64("GlobalBufferSize");
	xbsize = GetUint64("XBufferSize");
	tot_acc = GetUint64("TotalNumOfAcc");
	MAX_DRAM = GetInt("TotalNumOfDRAM");
	CACHE_LINE_BYTE = GetUint64("CacheLineByte");
	CACHE_LINE_COUNT = GetUint64("CacheLineCount");
	UNIT_BYTE = CACHE_LINE_BYTE / CACHE_LINE_COUNT;
	X_START = GetUint64("XStartAddress");
	A_ROW_START = GetUint64("ARowStartAddress");
	A_COL_START = GetUint64("AColStartAddress");
	WEIGHT_START = GetUint64("WeightStartAddress");
	XW_START = GetUint64("XWResultStartAddress");
	AXW_START = GetUint64("AXWResultStartAddress");
	SYSTOLIC_DIM = GetInt("SystolicDimension");
	CACHE_REPL_ALGO = GetUint64("CacheReplacementAlgo");
	DIVISION_TYPE = GetInt("CSRDivisionType");
	MECHA_TYPE = GetInt("MechanismType");
	UNIT_W_READ = GetInt("UnitWeightReadBlock");
	BLOOM_FILTER_NUM = GetUint64("BloomFilterNum");
	BF_RATIO = GetFloat("BloomFilterRatio");
	BF_OVERLAP = GetUint64("BloomFilterOverlap");
	CACHE_WAY_N = GetUint64("CacheWayN");
	MEM_TYPE = GetString("MemoryType");
	//MOV_MED_BLOCK_NUM = GetUint64("MovingMedianBlockNum");
}
