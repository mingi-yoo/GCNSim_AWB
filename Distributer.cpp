/*
 * multi unit simulation을 위한 Distributer
*/
#include "Distributer.h"

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

extern int DIVISION_TYPE;

uint64_t w_fold;



// multiple and accumulate
//uint64_t mac1_count; XW
//uint64_t mac2_count; AXW
// result matrix row #

Distributer::Distributer(IniParser *ini, DataReader *data) {
	// >>>>> set data space for each of accelerators
	tot_acc = ini->tot_acc; // 전체 unit 갯수
	adjcolindex = new queue<uint64_t>[tot_acc];
	adjrowindex = new queue<uint64_t>[tot_acc];

	// set MAC count
	w_fold = w_w;
	this->data = data;

	// set Address
	a_col_offset = new uint64_t[tot_acc];
	a_row_offset = new uint64_t[tot_acc];
	axw_offset = new uint64_t[tot_acc];
	a_row_info = new RowInfo[tot_acc];
	axw_count = new uint64_t[tot_acc];

	int cnt = ceil((float)x_h/tot_acc);

	for (int i = 0; i < tot_acc; i++)
	{
		a_col_offset[i] = 0x0010000000000000 * i;
		a_row_offset[i] = 0x0010000000000000 * i;
		axw_offset[i] = 0x0010000000000000 * i;
	}
	
	Distribute();
}

Distributer::~Distributer() {
	delete adjcolindex;
	delete adjrowindex;
	delete a_col_offset;
	delete a_row_offset;
	delete axw_offset;
	delete a_row_info;
}

// Distribute data to each accelerator
void Distributer::Distribute() {
	int size = ceil((float)x_h/tot_acc);
	int col_size = 0;
	int next_start = col_size;
	uint64_t last = 0;
	queue<uint64_t> tmp_adjcol = data->adjcolindex;
	queue<uint64_t> tmp_adjrow = data->adjrowindex;


	//csr divide
	if (DIVISION_TYPE == 0) {
		/*
		adjrowindex[0].push(tmp_adjrow.front());
		tmp_adjrow.pop();
		for (int i = 0; i < tot_acc; i++) {
			for (int j = 0; j < size; j++) {
				if (j == 0 && i != 0)
					adjrowindex[i].push(0);
				adjrowindex[i].push(tmp_adjrow.front() - last);
				tmp_adjrow.pop();
				col_size = adjrowindex[i].back() + last;
				if (j == size-1 || j+size*i == x_h-1) {
					for (int k = next_start; k < col_size; k++) {
						adjcolindex[i].push(tmp_adjcol.front());
						tmp_adjcol.pop();
					}
					next_start = col_size;
				}
				if (j+size*i == x_h-1)
					break;
					
			}
			last = last + adjrowindex[i].back();
		}
		for (int i = 0; i < tot_acc; i++) {
			a_row_info[i].row_start = size  * i;
			if (i > 1)
				a_row_info[i-1].row_end = a_row_info[i].row_start - a_row_info[i-1].row_start - 1;
			axw_count[i] = size;
		}
		a_row_info[tot_acc - 1].row_end = x_h - 1;
		axw_count[tot_acc-1] = x_h - a_row_info[tot_acc - 1].row_start;
		*/
		adjrowindex[0] = tmp_adjrow;
		adjcolindex[0] = tmp_adjcol;
		a_row_info[0].row_start = 0;
		a_row_info[0].row_end = x_h;
		axw_count[0] = x_h;
	}
	else if (DIVISION_TYPE == 1) {
		uint64_t unit_edge = ceil((float)tmp_adjcol.size()/tot_acc);
		tmp_adjrow.pop();
		int row = 0;
		for (int i = 0; i < tot_acc; i++) {
			adjrowindex[i].push(0);
			a_row_info[i].row_start = row;
			while (true) {
				col_size = adjrowindex[i].back();
				adjrowindex[i].push(tmp_adjrow.front() - last);
				tmp_adjrow.pop();
				col_size = adjrowindex[i].back() - col_size;
				row++;
				for (int j = 0; j < col_size; j++) {
					adjcolindex[i].push(tmp_adjcol.front());
					tmp_adjcol.pop();
				}
				if (tmp_adjcol.empty()) {
					while (!tmp_adjrow.empty()) {
						adjrowindex[i].push(tmp_adjrow.front() - last);
						tmp_adjrow.pop();
					}
					break;
				}
				if (adjrowindex[i].back() >= unit_edge) {
					last = last + adjrowindex[i].back();
					break;
				}
			}
			a_row_info[i].row_end = row-1;
		}
		a_row_info[tot_acc-1].row_end = x_h-1;
		for (int i = 0; i <tot_acc; i++) {
			axw_count[i] = adjrowindex[i].size()-1;
		}
	}
	queue<uint64_t> empty;
	data->adjrowindex = empty;
	data->adjcolindex = empty;
}

// for debugging
void Distributer::Print()  {
	for (int i = 0; i < tot_acc; i++)
	{
		cout<<"ID: "<<i<<endl;
		cout<<"X"<<endl;
		
		cout<<"AdjRow"<<endl;
		uint64_t cnt = adjrowindex[i].size();
		queue<uint64_t> tmp_adjrow = adjrowindex[i];
		for (int j = 0; j < cnt; j++)
		{
			cout<<tmp_adjrow.front()<<" ";
			tmp_adjrow.pop();
		}
		cout<<endl;
		cout<<"AdjCol"<<endl;
		cnt = adjcolindex[i].size();
		queue<uint64_t> tmp_adjcol = adjcolindex[i];
		for (int j = 0; j < cnt; j++)
		{
			cout<<tmp_adjcol.front()<<" ";
			tmp_adjcol.pop();
		}
		cout<<endl;
		
		cout<<"ID: "<<i<<" END"<<endl;
	}
}

int Distributer::ReturnID(uint64_t address) {
	uint64_t check = address & 0x00ffffffffffffff;

	check = check / 0x0010000000000000;

	return (int)check;
}
