/*** temporary file ***
	Must Be Revised 
*/
#include "DataController.h"

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

extern uint64_t w_h, w_w, x_h, x_w, a_w, a_h;

extern int w_fold;

DataController::DataController(int id,
							   queue<uint64_t> adjrowindex, 
							   queue<uint64_t> adjcolindex) {
	this->id = id;
	this->adjrowindex = adjrowindex;
	adjrowindex_archive = adjrowindex;
	this->adjcolindex = adjcolindex;
	adjcolindex_archive = adjcolindex;
	flag.x_over = false;
	flag.a_row_over = false;
	flag.a_col_over = false;
}

DataController::~DataController() {}

Type DataController::ReturnDataType(uint64_t address) {
	if (address >= X_START && address < A_ROW_START)
		return X;
	else if (address >= A_ROW_START && address < A_COL_START)
		return A_ROW;
	else if (address >= A_COL_START && address < WEIGHT_START)
		return A_COL;
}

queue<float> DataController::XDataReturn() {
	queue<float> ret;
	return ret;
}

queue<uint64_t> DataController::AdjRowDataReturn() {
	int count = CACHE_LINE_COUNT;
	queue<uint64_t> ret;

	if (adjrowindex.size() < CACHE_LINE_COUNT)
		count = adjrowindex.size();


	// for debugging
	//cout<<"ADJROWINDEX POPPED"<<endl;

	for (int i = 0; i < count; i++) {
		// for debugging
		//cout<<adjrowindex.front()<<" ";

		ret.push(adjrowindex.front());
		adjrowindex.pop();
	}

	//for debugging
	//cout<<endl;

	return ret;
}

queue<uint64_t> DataController::AdjColDataReturn() {
	int count = CACHE_LINE_COUNT;
	queue<uint64_t> ret;

	if (adjcolindex.size() < CACHE_LINE_COUNT)
		count = adjcolindex.size();


	// for debugging
	//cout<<"ADJCOLINDEX POPPED"<<endl;

	for (int i = 0; i < count; i++) {
		// for debugging
		//cout<<adjcolindex.front()<<" ";

		ret.push(adjcolindex.front());
		adjcolindex.pop();
	}

	//for debugging
	//cout<<endl;

	return ret;
}

bool DataController::NeedRefill(Type type) {
	if (type == A_ROW)
		return adjrowindex.empty();
	else if (type == A_COL)
		return adjcolindex.empty();
}

void DataController::RefillQueue(Type type) {
	if (type == A_ROW)
		adjrowindex = adjrowindex_archive;
	else if (type == A_COL)
		adjcolindex = adjcolindex_archive;
}