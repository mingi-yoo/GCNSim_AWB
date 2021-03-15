#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>

using namespace std;

int *rowindex;
vector<vector<int>> colindex;
int v;
int e;

int main(int argc, char** argv) {
	ifstream input(argv[1]);
	if (input.is_open()) {
		string line;
		getline(input, line);
		v = stoi(line);
		getline(input, line);
		e = stoi(line);
		rowindex = new int[v];
		for (int i = 0; i < v; i++) {
			rowindex[i] = 0;
			colindex.push_back(vector<int> ());
		}
		for (int i = 0; i < e; i++) {
			getline(input, line);
			stringstream ss(line);
			string tmp;
			bool first = true;
			while (getline(ss, tmp, ' ')) {
				int vertex;
				if (first) {
					vertex = stoi(tmp);
					rowindex[vertex]++;
					first = false;
				}
				else {
					colindex[vertex].push_back(stoi(tmp));
				}
			}
		}
		for (int i = 0; i < v; i++) {
			if (v != 0)
				rowindex[i] += rowindex[i-1];
			sort(colindex[i].begin(), colindex[i].end());
		}
	}
	else {
		throw invalid_argument("Cannot open datafile.");
	}

	ofstream output("output.txt");
	output<<"0 ";

	for (int i = 0; i < v; i++) {
		output<<rowindex[i];
		if (i != v-1)
			output<<" ";
		else
			output<<endl;
	}

	for (int i = 0; i < v; i++) {
		for (int j = 0; j < colindex[i].size(); j++) {
			output<<colindex[i][j];
			if (i != v-1 || j != colindex[i].size()-1)
				output<<" ";
		}
	}
}
