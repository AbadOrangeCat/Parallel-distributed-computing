
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>
#include <thread> 
#include <bits/stdc++.h>
#include <Mutex>
#include <condition_variable>

using namespace std;

std::mutex mtx;
std::condition_variable cv;
vector<string> finished;

vector<string> split(const string& s, char delim) {
	vector<string> result;
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		if (item != " " || item != "") {
			result.push_back(item);
		}
		
	}
	return result;
}

int check(vector<string> vs) {
	int res = 0;
	for (int i = 0; i < vs.size(); i++) {
		if (std::find(finished.begin(), finished.end(), vs[i]) != finished.end())
		{
			res += 1;
		}
	}
	return res;
}

void print_id(string s, vector<string> vs, string head) {
	std::unique_lock<std::mutex> lck(mtx);
	bool ready = false;
	int res = 0;
	if (vs.size() == 0) {
		ready = true;
		finished.push_back(head);
		cv.notify_all();
	}
	else {
		if (vs.size() == check(vs)) {
			ready = true;
			finished.push_back(head);
			cv.notify_all();
		}
	}
	
	while (!ready) {
		if (vs.size() == check(vs)) {
			ready = true;
			finished.push_back(head);
			cv.notify_all();
		}
		cv.wait(lck); 
	}
	cv.notify_all();

	const char* cc = s.c_str();
	cout << "Compiling "<< head <<" using " << cc << endl;
	system(cc);
}

int main()
{
	ifstream infile;
	infile.open("MakefileComplex");
	
	if (!infile.is_open()) {
		cout << "Cannot find a file named 'Makefile'.";
		return 0;
	}

	cout << "Reading......" << endl;

	vector<string> target;
	vector<vector<string>> dependencies;
	vector<string> command;
	string line;
	bool single = true;
	while (getline(infile, line)) {
		if (single) {
			vector<string> v = split(line, ':');
			string tar = v[0].substr(0, v[0].length() - 1);
			target.push_back(tar);
			vector<string> x = split(v[1], ' ');
			for (int i = 0; i < x.size(); i++) {
				if (x[i] == "") {
					x.erase(x.begin() + i);
				}
				if (x[i].find(".cpp") != string::npos) {
					x.erase(x.begin() + i);
				}
				if (x[i].find(".h") != string::npos) {
					x.erase(x.begin() + i);
				}
			}
			dependencies.push_back(x);
		}
		else {
			command.push_back(line);
		}
		single = !single;
	}
	cout << "Finish Reading......" << endl;

	cout << "Start Compile......" << endl;

	std::thread threads[command.size()];
	for (int i = command.size() - 1; i > -1; i--) {
		threads[i] = std::thread(print_id, command[i], dependencies[i], target[i]);
	}
	for (auto& th : threads) th.join();

	cout << "Finished Compile......" << endl;
	return 0;
}