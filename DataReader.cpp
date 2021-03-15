/*
 * 기본적인 시뮬레이션을 위한 데이터
 * 읽어들이는 DataReader
*/
#include "DataReader.h"

using namespace std;

extern uint64_t CACHE_LINE_BYTE; 
extern uint64_t CACHE_LINE_COUNT;
extern uint64_t UNIT_BYTE;

extern uint64_t A_COL_START;  
extern uint64_t A_ROW_START;  
extern uint64_t X_START;  
extern uint64_t WEIGHT_START; 
extern uint64_t XW_START;
extern uint64_t AXW_START;

uint64_t w_h, w_w, x_h, x_w, a_w, a_h;

DataReader::DataReader(string path1, string path2) { 
	ReadData(path1, path2); 
}

DataReader::~DataReader() {}

queue<float> DataReader::GetFloatVal(string line, char delimiter) {
	queue<float> internal;
	stringstream ss(line);
	string temp;

	while (getline(ss, temp, delimiter)) {
		if (temp == "\n")
			break;
		internal.push(stof(temp));
	}

	return internal;
}

queue<uint64_t> DataReader::GetUint64Val(string line, char delimiter) {
	queue<uint64_t> internal;
	stringstream ss(line);
	string temp;

	while (getline(ss, temp, delimiter)) {
		if (temp == "\n")
			break;
		internal.push(stoull(temp));
	}

	return internal;
}

bool DataReader::ReadData(string path1, string path2) {
	ifstream openFile1(path1);
	ifstream openFile2(path2);
	if (openFile1.is_open()) {
		//get weight_h
		string line;
		getline(openFile1, line);
		w_h = stoi(line);
		x_w = w_h;
		getline(openFile1, line);
		//get weight_w(is equal to x_h)
		w_w = stoi(line);
		getline(openFile1, line);
		x_h = stoi(line);
		a_h = x_h;
		a_w = a_h;
		openFile1.close();
	}
	else {
		throw invalid_argument("Cannot open datafile.");
	}

	if (openFile2.is_open()) {
		string line;
		getline(openFile2, line);
		adjrowindex = GetUint64Val(line, ' ');
		getline(openFile2, line);
		adjcolindex = GetUint64Val(line, ' ');
		openFile2.close();
	}
	else {
		throw invalid_argument("Cannot open datafile.");
	}

	return true;
}
