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
	tile_num = stoull(argv[2]); // tile 나누는 수
	v_num = ceil((float)v/tile_num); 
	ifstream openFile(argv[3]);
	cout << argv[3] << endl;

	if (openFile.is_open()) {
		for (uint64_t i = 0; i < v * tile_num; i++)
			new_csr.push_back(vector<uint64_t> ());
		string line, tmp;
		getline(openFile, line);
		stringstream ss(line);

		while (getline(ss, tmp, ' ')) {
			if (tmp == "\n")
				break;
			row.push_back(stoull(tmp));
		}
		getline(openFile, line);
		stringstream tt(line);
		for (uint64_t i = 0; i < v; i++) {
			uint64_t v_cnt = row[i+1] - row[i];
			for (uint64_t j = 0; j < v_cnt; j++) {
				getline(tt, tmp, ' ');
				uint64_t edge = stoull(tmp);
				uint64_t index = edge / v_num;
				if (index == tile_num)
					index--;
				new_csr[i + v*index].push_back(stoull(tmp));
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
		string p = argv[4];

		string output_path = p + "_vt.txt";
		cout << output_path << endl;
		ofstream output(output_path);

		output<<"0 ";
		uint64_t v_acm = 0;
		for (uint64_t i = 0; i < v * tile_num; i++) {
			v_acm += new_csr[i].size();
			output<<v_acm;
			if (i != v * tile_num - 1)
				output<<" ";
			else
				output<<endl;
		}
		for (uint64_t i = 0; i < v * tile_num; i++) {
			uint64_t v_cnt = new_csr[i].size();
			for (uint64_t j = 0; j < v_cnt; j++) {
				output<<new_csr[i][j];
				if (i == v * tile_num -1 && j == v_cnt -1 )
					output<<endl;
				else
					output<<" ";
			}
		}
		output.close();
	}
	return 0;
}

/* 파라미터 순서 ---- "./vt v tile_num file_path output name" "*/