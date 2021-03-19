/*** temporary file ***
	Must Be Revised 
*/
#ifndef __DATAREADER_H__
#define __DATAREADER_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <queue>
#include <cmath>
#include <stdexcept>
#include "Common.h"

using namespace std;

/* Given data Format
data1
 * nxmxk (AXW)
 * cin0: m height of weight
 * cin1: k width of w
 * cin2: n height of x (#of nodes)
 * W는 들어올 필요 없음... 사이클 계산이 목적
 * cin3: read x by one line

data2
 * cin4: rowIdx of A by CSR
 * cin5: colIdx of A by CSR
 * 최종적으로 처리해서 변수에 저장
*/



class DataReader {
public:
	DataReader(string path1, string path2);
	~DataReader();
	queue<uint64_t> adjrowindex;
	queue<uint64_t> adjcolindex;
private:
	queue<float> GetFloatVal(string line, char delimiter);
	queue<uint64_t> GetUint64Val(string line, char delimiter);
	bool ReadData(string path1, string path2);
};

#endif