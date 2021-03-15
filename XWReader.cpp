/*** temporary file ***
	Must Be Revised 
*/
#include "XWReader.h"

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

extern uint64_t w_fold;

extern int MECHA_TYPE;
extern int UNIT_W_READ;

XWReader::XWReader(int id, uint64_t a_col_size) : q_space(0), req_cnt(0) {
	this->id = id;
	this->a_col_size = a_col_size;
	tot_req = a_col_size * w_fold;
	flag.q_empty = true;
	flag.can_receive = true;
}

XWReader::~XWReader() {}

ERData XWReader::TransferData() {
	ERData ret = xwq_archive.front();
	xwq_archive.pop();
	q_space -= 2;
	if (!IsEndOperation())
		flag.can_receive = true;

	return ret;
}

ERData XWReader::Request() {
	ERData ret = xwq.front();
	xwq.pop();
	req_cnt++;
	if (xwq.empty())
		flag.q_empty = true;

	return ret;
}

bool XWReader::IsEndOperation() {
	return (req_cnt == tot_req);
}

void XWReader::ReceiveData(ERData data) {
	//cout<<"XWR) RECEIVE DATA: "<<dec<<data.colindex<<endl;
	xwq.push(data);
	xwq_archive.push(data);
	q_space += 2;
	flag.q_empty = false;
	if (MAX_QUEUE_SIZE - q_space < 2)
		flag.can_receive = false;
}

void XWReader::TurnOffFlag() {
	flag.can_receive = false;
}