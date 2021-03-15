/*** temporary file ***
	Must Be Revised 
*/
#ifndef __COMMON_H__
#define __COMMON_H__

#define MAX_QUEUE_SIZE 1000000
#define READ false
#define WRITE true

using namespace std;

/*Address Topology
X_START
-> A_ROW_START
-> A_COL_START
-> WEIGHT_START
-> XW_START
-> AXW_START
*/

enum Type {X, A_ROW, A_COL};

// for multi processing
// indicate start address of data
// use e.g. arr[OFFSET]
struct Offset {
	uint64_t row_offset;
	uint64_t col_offset;
};

// DataController flag struct
struct DCFlag {
	bool x_over;
	bool a_row_over;
	bool a_col_over;
};

// Reader flag struct
struct ReaderFlag {
	bool req_need;
	bool q_empty;
};

// for accelerator boundary check
struct ERData {
	uint64_t rowindex;
	uint64_t colindex;
	uint64_t address;
	int pre_w_fold;
	int pre_w_fold_start;
	int pre_repeat;
	bool is_end;
};

struct XRData {
	float ifvalue;
	bool is_end;
};

struct RowInfo {
	uint64_t row_start;
	uint64_t row_end;
};

// for request control
struct RCNT {
	int tot_read_cnt;
	int pre_read_cnt;
	int read_cnt_acm;
};

#endif