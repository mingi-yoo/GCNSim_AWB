#ifndef __DISTRIBUTER_H__
#define __DISTRIBUTER_H__

#include "IniParser.h"
#include "DataReader.h"
#include "Common.h"
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

class Distributer {
public:
	Distributer(IniParser *ini, DataReader *data);
	~Distributer();
	queue<uint64_t> *adjcolindex;
	queue<uint64_t> *adjrowindex;
	uint64_t tot_acc;
	DataReader *data;
	uint64_t *a_col_offset;
	uint64_t *a_row_offset;
	uint64_t *axw_offset;
	RowInfo *a_row_info;
	// for boundary of accelerator operation
	uint64_t *axw_count;
	void Print();
	int ReturnID(uint64_t address);
private:
	void Distribute();
};

#endif
