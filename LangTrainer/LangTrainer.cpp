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

	for (size_t i = 0; i < words.size(); i++) {
		if (itselfRecord > countErrorsInString(words[i].mean, input)) {
			itselfRecord = countErrorsInString(words[i].mean, input);
			indexRecord = i;
		}
	}

	return words[indexRecord];
}

void testWordMean(const std::vector<dictWord>& words, size_t checkIndex, const std::string input) {
	auto& instCheck = words[checkIndex];
	size_t errors = countErrorsInString(instCheck.mean, input);
	if (errors == 0) {
		std::cout << "Absolutely correct!\n";
	} else {
		auto instCorrect = findAtLeastErrorsMean(words, input);
		if (countErrorsInString(instCorrect.mean, input) == 0 && instCorrect.word == instCheck.word)
			std::cout << "Hold on, did you mean " << std::quoted(instCorrect.mean) << '.';
		else if (errors == 1)
			std::cout << "Correct, but typo.";
		else {
			if ((float)(instCheck.mean.size() - errors) / (float)instCheck.mean.size() > 0.5f)
				std::cout << "Ehh... Decent.";
			else
				std::cout << "Too bad.";
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
	while(true) {
		rnd = rand() % list.size();
		testWordMean(list, rnd, preInputMean(list, rnd));
	}
}
