#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

int main(void) {
	int w_h, w_w, x_h;
	int sparse;

	srand((unsigned int)time(NULL));

	cout<<"w_h: ";
	cin>>w_h;
	cout<<"w_w: ";
	cin>>w_w;
	cout<<"x_h: ";
	cin>>x_h;
	cout<<"sparse(0~99): ";
	cin>>sparse;

	string	path = "xw_"+to_string(w_h)+"_"+to_string(w_w)+"_"+to_string(x_h)+".txt";

	ofstream output(path);
	output<<w_h<<endl;
	output<<w_w<<endl;
	output<<x_h<<endl;

	for (int i = 0; i < w_h * x_h; i++) {
		int num = rand()%101;
		if (num <= sparse)
			output<<0;
		else
			output<<num;
		if (i < w_h * x_h - 1)
			output<<" ";
	}
	output.close();

	return 0;
}
