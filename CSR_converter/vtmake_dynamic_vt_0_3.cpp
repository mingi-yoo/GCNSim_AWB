#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <utility>
#include <vector>

using namespace std;

uint64_t v;
uint64_t tile_num;
uint64_t v_num;
vector<vector<uint64_t>> new_csr;

vector<uint64_t> row;

int main(int argc, char** argv) {
	cout << endl;
	v = stoull(argv[1]); // vertex 숫자
	tile_num = 2; // tile 나누는 수
	v_num = ceil((float)v/tile_num);  // 몇개씩 나누어야하는지
	ifstream openFile(argv[2]);
	cout << argv[2] << endl;

	if (openFile.is_open()) {
		// 잘라서 아래로 붙이는 형태이기 때문
		for (uint64_t i = 0; i < v * (tile_num+1); i++) // 왼쪽 아래에서 또 나누어서 진행
			new_csr.push_back(vector<uint64_t> ());
		string line, tmp;
		getline(openFile, line);
		stringstream ss(line);

		while (getline(ss, tmp, ' ')) {
			if (tmp == "\n")
				break;
			row.push_back(stoull(tmp)); // CSR을 읽어들이기 때문에 쭉 row 읽어들인다
		}

		// 참고로 dynamic 실험에서는 2x2이면서 왼쪽 아래는 2x2로 또 쪼갤 것!
		getline(openFile, line);
		stringstream tt(line);
		for (uint64_t i = 0; i < v; i++) {
			// 위쪽 절반
			if(i < v/2) {
				uint64_t v_cnt = row[i+1] - row[i]; // 몇개 vertex인지....(해당 row에)
				for (uint64_t j = 0; j < v_cnt; j++) { // 그 대상들에 대해서
					getline(tt, tmp, ' '); // 우선 읽어들이고
					uint64_t edge = stoull(tmp); // to값을 uint64_t로 변환
					if(edge < v/4) {
						new_csr[i].push_back(stoull(tmp));
					} else if(edge < v/2) {
						new_csr[i + v/2].push_back(stoull(tmp));
					} else {
						new_csr[i + v*1.5].push_back(stoull(tmp));
					}
				}
			} else if(i >= v/2 && i < v*3/4) {
				uint64_t v_cnt = row[i+1] - row[i]; // 몇개 vertex인지....(해당 row에)
				for (uint64_t j = 0; j < v_cnt; j++) { // 그 대상들에 대해서
					getline(tt, tmp, ' '); // 우선 읽어들이고
					uint64_t edge = stoull(tmp); // to값을 uint64_t로 변환
					if(edge < v/2) {
						new_csr[i + v/2].push_back(stoull(tmp));
					} else if(edge < v*3/4) {
						new_csr[i + v*1.5].push_back(stoull(tmp));
					} else {
						new_csr[i + v*2].push_back(stoull(tmp));
					}
				}
			} else {
				uint64_t v_cnt = row[i+1] - row[i]; // 몇개 vertex인지....(해당 row에)
				for (uint64_t j = 0; j < v_cnt; j++) { // 그 대상들에 대해서
					getline(tt, tmp, ' '); // 우선 읽어들이고
					uint64_t edge = stoull(tmp); // to값을 uint64_t로 변환
					if(edge < v/2) {
						new_csr[i + v/2].push_back(stoull(tmp));
					} else if(edge < v*3/4) {
						new_csr[i + v*1.75].push_back(stoull(tmp));
					} else {
						new_csr[i + v*2.25].push_back(stoull(tmp));
					}
				}
			}
		}
		openFile.close();
		/*
		for (int i = 0; i < new_csr.size(); i++) {
			for (int j = 0; j <new_csr[i].size(); j++) {
				cout<<new_csr[i][j]<<" ";
			}
			cout<<endl;
		}
		*/
		string p = argv[3];

		string output_path = p + "_dynamic_vt_0_3.txt";
		cout << output_path << endl;
		ofstream output(output_path);

		output<<"0 ";
		uint64_t v_acm = 0;
		for (uint64_t i = 0; i < v * (tile_num+1); i++) {
			v_acm += new_csr[i].size();
			output<<v_acm;
			if (i != (uint64_t)(v * (tile_num+1)-1))
				output<<" ";
		}
		output<<endl;
		for (uint64_t i = 0; i < v * (tile_num+1); i++) {
			uint64_t v_cnt = new_csr[i].size();
			for (uint64_t j = 0; j < v_cnt; j++) {
				output<<new_csr[i][j];
				if (i == (uint64_t)(v * (tile_num+1) -1) && j == v_cnt -1 )
					output<<endl;
				else
					output<<" ";
			}
		}
		output.close();
	}
	return 0;
}

/* 파라미터 순서 ---- "./vt v file_path output name" "*/