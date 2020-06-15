#include <iostream>
#include <queue>
#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <map>
#include <iomanip>
#include <cstdint>

template <typename Grid>
int64_t getMin(Grid& theGrid, int64_t x, int64_t y, int64_t sumOfNumSoFar, std::vector<string>& words) {
	theGrid.at(x).at(y) = 1;
	//current location is x,y i want all the things left of x and below y...
	long long int min = 0;
	string word = "";
	for (int i = 0; i < y - x; ++i)
	{
		//iterates for every needed combination
		long long int left = theGrid.at(x).at(i + x);
		long long int below = theGrid.at(x + i + 1).at(y);

		if (min > left + below + sumOfNumSoFar || min == 0) {
			min = left + below + sumOfNumSoFar;
			if (x == 0 && y == theGrid.size() - 1) {
				word = words.at(i + x);
			}
		}
	}

	return min;
}

int main(int argc, char const* argv[])
{
	std::vector<string> words;
	std::vector<long long int> nums;
	std::vector<long long int> sumsOfNums;
	std::vector<std::vector<long long int>> theGrid;
	std::map<string, int> stringToInt;
	string word = "";
	int64_t num = 0;
	ifstream inputFile(argv[1], ios::in);

	if (inputFile.is_open())
	{
		while (inputFile.tellg() != -1) {
			inputFile >> word >> num;
			stringToInt[word] = num;
			if (word != "\n")
			{
				//avoid that pesky extra line at the end of files!!
				words.push_back(word);
			}
		}
	}
	else {
		std::cout << "File Failed to open\n";
		return 1;
	}

	inputFile.close();
	std::sort(words.begin(), words.end());

	conar auto numOfWords = words.size();
	sumsOfNums.resize(numOfWords);
	theGrid.resize(numOfWords + 1);

	for (int i = 0; i < numOfWords; ++i)
	{
		nums.push_back(stringToInt[words.at(i)]);
	}

	sumsOfNums.at(0) = nums.at(0);

	// sum up all of the numbers
	for (int i = 1; i < numOfWords; ++i)
	{
		sumsOfNums.at(i) = sumsOfNums.at(i - 1) + nums.at(i);
	}

	//files are read in and stored in vectors
	//initiate theGrid to 0 and the original counts.
	for (int rr = 0; rr < numOfWords; ++rr) {
		theGrid.at(rr).resize(numOfWords + 1);
		theGrid.at(rr).at(rr) = 0;
		theGrid.at(rr).at(rr + 1) = nums.at(rr);
	}

	// do the last set.
	theGrid.at(numOfWords).resize(numOfWords + 1);
	theGrid.at(numOfWords - 1).at(numOfWords) = nums.at(numOfWords - 1);
	theGrid.at(numOfWords).at(numOfWords) = 0;


	// goes through the grid from the bottom to the top
	// tried to be cache friendly and use what i learned from MicroP
	for (int rr = numOfWords; 0 <= rr; --rr) {

		for (int cc = rr + 2; numOfWords >= cc; ++cc) {

			int sum = sumsOfNums.at(cc - 1);
			if (rr != 0) {
				sum -= sumsOfNums.at(rr - 1);
			}

			theGrid.at(rr).at(cc) = getMin(theGrid, rr, cc, sum, words);
		}
	}

	long long int minWeight = theGrid.at(0).at(numOfWords);
	std::cout << '\n' << minWeight << '\n';

	return 0;
}
