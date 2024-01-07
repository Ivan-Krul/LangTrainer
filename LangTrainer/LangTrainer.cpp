#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <functional>

struct dictWord {
	std::string word;
	std::string mean;
	char type[3];
	std::string section;
};

std::vector<std::string> splitStringFromFile(const std::string& filename, char divider, const std::function<bool(const std::string&)>& func) {
	std::ifstream fin;
	std::string str;
	std::vector<std::string> strs;
	fin.open(filename);

	if (!fin.is_open()) {
		std::cout << "No\n";
		return strs;
	}

	while (!fin.eof()) {
		std::getline(fin, str, divider);
		if (func(str))
			strs.push_back(str);
	}
	fin.close();

	return strs;
}


std::vector<size_t> countElements(const std::string& str, char el) {
	std::vector<size_t> founded;

	for (size_t i = 0; i < str.size(); i++) {
		if (str[i] == el)
			founded.push_back(i);
	}
	return founded;
}

std::vector<std::string> extractColomnsFromRow(std::string row) {
	std::vector<std::string> colomns;

	auto foundedPos = countElements(row, '\t');
	size_t accumulatePos = 0;

	for (auto& founded : foundedPos) {

		colomns.push_back(row.substr(0, founded - accumulatePos));
		row.erase(row.cbegin(), row.cbegin() + (founded + 1 - accumulatePos));
		accumulatePos += founded + 1;
	}

	colomns.push_back(row);
	return colomns;
}

void updateMatrixSizeLev(std::vector<std::vector<size_t>>& matrix, size_t& width, size_t& height, const size_t origSize, const size_t checkSize) {
	size_t i = 0;
	if (width < origSize) { // width module
		size_t prevWidth = width;
		width = origSize;
		matrix.resize(width + 1);
		for (i = 0; i < width + 1; i++) { // fill by height
			if (matrix[i].size() <= height)
				matrix[i].resize(height + 1);
			matrix[i].front() = i;
		}
	}
	if (height < checkSize) { // height module
		height = checkSize;
		for (i = 0; i < width + 1; i++) // fill by width
			matrix[i].resize(height + 1);
		for (i = 0; i < height + 1; i++)
			matrix.front()[i] = i;
	}
}

// Wagner–Fischer algorithm
size_t countErrorsInStringLev(const std::string& orig, const std::string& check) {
	// all static to not spend time on pushing temporary variables in stack and then poping, dynamic programming anyway
	static std::vector<std::vector<size_t>> matrix;
	static size_t width = 0;
	static size_t height = 0;
	static size_t x;
	static size_t y;
	static bool subst;

	updateMatrixSizeLev(matrix, width, height, orig.size(), check.size());

	for (x = 0; x < orig.size(); x++) {
		for (y = 0; y < check.size(); y++) {
			subst = orig[x] != check[y];
			matrix[x + 1][y + 1] = std::min(matrix[x][y + 1]+1, std::min(matrix[x + 1][y]+1, matrix[x][y] + subst));
		}
	}
	return matrix[x][y];
}

size_t countErrorsInString(const std::string& orig, const std::string& check) {
	size_t count = abs((signed)orig.size() - (signed)check.size());
	for (size_t i = 0; i < std::min(orig.size(), check.size()); i++) {
		count += orig[i] != check[i];
	}
	return count;
}

void adaptStringRow(std::string& str) {
	auto tabs = countElements(str, '\t');
	for (size_t i = 2; i < tabs.size(); i++)
		str.erase(str.cbegin() + (tabs[i] - (i - 2)));
	size_t ind = str.substr(tabs[1]).find("-") + tabs[1];
	str.erase(str.cbegin() + ind, str.cbegin() + (ind + 2));
}

std::vector<dictWord> extractAllWords(const std::string& filename) {
	std::vector<dictWord> words;
	dictWord word;

	auto content = splitStringFromFile(filename, '\n', [](const std::string& str) { return (str.size() >= 10 || str[0] == '*') && str[0] != '#'; });
	std::vector<std::string> rowDisassembled;

	for (auto& row : content) {
		if (row[0] == '*')
			word.section = row.substr(1);
		else {
			adaptStringRow(row);
			rowDisassembled = extractColomnsFromRow(row);
			memcpy(word.type, rowDisassembled[0].substr(1, 2).c_str(), 3);
			word.word = rowDisassembled[1];
			word.mean = rowDisassembled[2];
			words.push_back(word);
		}
	}

	return words;
}

const dictWord& findAtLeastErrorsMean(const std::vector<dictWord>& words, const std::string& input) {
	size_t indexRecord = -1;
	size_t itselfRecord = -1;
	size_t lev = 0;

	for (size_t i = 0; i < words.size(); i++) {
		lev = countErrorsInStringLev(words[i].mean, input);
		if (itselfRecord > lev) {
			itselfRecord = lev;
			indexRecord = i;
		}
	}

	return words[indexRecord];
}

void testWordMean(const std::vector<dictWord>& words, size_t checkIndex, const std::string input, int& score) {
	auto& instCheck = words[checkIndex];
	size_t errors = countErrorsInStringLev(instCheck.mean, input);
	if (errors == 0) {
		std::cout << "Absolutely correct!\n";
		score += 100;
	} else {
		auto instCorrect = findAtLeastErrorsMean(words, input);
		if (countErrorsInStringLev(instCorrect.mean, input) == 0 && instCorrect.word == instCheck.word) {
			std::cout << "Hold on, did you mean " << std::quoted(instCorrect.mean) << '.';
			score += 90;
		}
		else if (errors == 1) {
			std::cout << "Correct, but typo.";
			score += 70;
		}
		else {
			if ((float)(instCheck.mean.size() - errors) / (float)instCheck.mean.size() > 0.75f) {
				std::cout << "Ehh... Decent.";
				score += 25 * (float)(abs((signed)(instCheck.mean.size() - errors))) / (float)instCheck.mean.size();
			}
			else {
				std::cout << "Too bad.";
				score -= 75 - 75 * (float)(abs((signed)(instCheck.mean.size() - errors))) / (float)instCheck.mean.size();
			}
			std::cout << " Also can be " << std::quoted(instCorrect.word) << ", what translates as " << std::quoted(instCorrect.mean) << " as " << instCorrect.type << " in section " << std::quoted(instCorrect.section) << '.';
		}
		std::cout << " Actual meaning is " << std::quoted(instCheck.mean) << '\n';
	}
	std::cout << '\n';
}

std::string preInputMean(const std::vector<dictWord>& words, size_t checkIndex) {
	auto& instCheck = words[checkIndex];
	std::string input;
	std::cout << "Word " << std::quoted(instCheck.word) << " is a " << instCheck.type << " in " << std::quoted(instCheck.section) << '\n';
	std::cout << "Translation: ";
	std::getline(std::cin >> std::ws, input);
	return input;
}

int main() {
	srand(time(NULL));
	auto list = extractAllWords("dictionary.txt");
	auto rnd = rand();
	int score = 0;
	std::string input = "a";

	while(input[0] != 126) {
		rnd = rand() % list.size();
		input = preInputMean(list, rnd);
		testWordMean(list, rnd, input, score);
		std::cout << "You have: " << score << " scores\n";
	}
	std::cout << "Session ended\n";
}
