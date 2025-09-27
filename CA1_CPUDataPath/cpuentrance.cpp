#include "CPU.h"

// #include <sstream>
// #include <iostream>
// #include <fstream>
// using namespace std;

#include <iostream>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
		cout << "No file name entered. Exiting..." << endl;
		return -1;
	}

    ifstream infile(argv[1]); // input file stream

    if (!(infile.is_open() && infile.good())) {
		cout << "Error opening file!" << endl;
		return -1; 
	}

    // initialize 4KB instruction memory data
    bitset<8> instMem[4096];

    string line;
    int dec;
    int instIdx = 0;

    while(getline(infile, line)) {
        dec = stoi(line);
        bitset<8> inst(dec);
        instMem[instIdx] = inst; 
        instIdx = instIdx + 1;
    }

    infile.close();

    int maxPC = instIdx;

    // initialize CPU
    CPU myCPU;
    int PC = 0;

    bitset<32> curr;
	instruction instr = instruction(curr);
	bool done = true;
    bitset<32> opCodeMask(63);

	while (done == true) // Each iteration is equal to one clock cycle.  
	{
		//fetch
		curr = myCPU.Fetch(instMem);
		instr = instruction(curr);

		// decode
		done = myCPU.Decode(&instr);

		if (done == false) // break from loop so stats are not mistakenly updated
			break;

		if (myCPU.readPC() > maxPC)
			break;
	}


	cout << "(" << myCPU.RegAccess(10) << "," << myCPU.RegAccess(11) << ")" << endl;



    return 0;
}