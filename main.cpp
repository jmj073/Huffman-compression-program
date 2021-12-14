#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include "huffman.hpp"

// Messages

#define ENTER_HELP "Enter '-h' as argument for help.\n"

#define INVALID_ARG "Invalid argument received.\n" ENTER_HELP

#define INVALID_OPTION_COMBINATION "Invalid option combination,\n" ENTER_HELP

#define SAME_PATH "Source and destination cannot be the same."

// Exit Code

#define EC_GOOD 0
#define EC_INVALID_ARG 1
#define EC_UNABLE_TO_OPEN_FILE 2
#define EC_INVALID_OPTION_COMBINATION 3
#define EC_SAME_PATH 4

// Options
#define ENCODE			01
#define DECODE			02
#define HELP			04
#define PRINT_SIZE		010
//#define SOURCE_BINARY	020 // If the bit in this position is 0, then the input stream is in text mode.
//#define DESTINATION_BINARY 040 // If the bit in this position is 0, then the output stream is in text mode.

using namespace std;

void PrintHelp()
{
	cout << "usage: app_name [options] source destination\n"
			"ex) huffman -e -s source.txt destination.huf\n"
			"  options:\n"
			"    -h  (help) print help. No source and destination input required.\n"
			"    -e  (encode) Compress the source and save it to the destination.\n"
			"    -d  (decode) Decompress the source and save it to the destination.\n"
			"    -s  (size) Print the size of the source file and destination file.\n"
			"  source:\n"
			"    Path to the target file to be compressed or decompressed.\n"
			"    Cannot be the same as the destination\n"
			"  destination:\n"
			"    The path to the file in which to save the compressed or unpacked results.\n"
			"    Cannot be the same as the source\n";
}

void PrintSize(const char* src, const char* dst)
{
	auto source_size = filesystem::file_size(src);
	auto destination_size = filesystem::file_size(dst);

	cout << "source: " << source_size << "bytes, ";
	cout << "destination: " << destination_size << "bytes, ";

	if (source_size >= destination_size) {
		cout << "decrease: " << source_size - destination_size << "bytes, ";
		cout << (1 - (double)destination_size / source_size) * 100 << '%';
	}
	else {
		cout << "increase: " << destination_size - source_size << "bytes, ";
		cout << (double)destination_size / source_size - 1 * 100 << '%';
	}
	cout << endl;
}

int main(int argc, char* argv[])
{
	if (argc == 1) {
		cout << INVALID_ARG;
		return EC_INVALID_ARG;
	}

	int options = 0;

	int i;
	for (i = 1; i < argc && argv[i][0] == '-'; i++) {
		if (!strcmp(argv[i] + 1, "e")) {
			if (options & (DECODE | HELP)) {
				cout << INVALID_OPTION_COMBINATION;
				return EC_INVALID_OPTION_COMBINATION;
			}
			options |= ENCODE;
		}
		else if (!strcmp(argv[i] + 1, "d")) {
			if (options & (ENCODE | HELP)) {
				cout << INVALID_OPTION_COMBINATION;
				return EC_INVALID_OPTION_COMBINATION;
			}
			options |= DECODE;
		}
		else if (!strcmp(argv[i] + 1, "h")) {
			if (options & (ENCODE | DECODE | PRINT_SIZE)) {
				cout << INVALID_OPTION_COMBINATION;
				return EC_INVALID_OPTION_COMBINATION;
			}
			options |= HELP;
		}
		else if (!strcmp(argv[i] + 1, "s")) {
			if (options & HELP) {
				cout << INVALID_OPTION_COMBINATION;
				return EC_INVALID_OPTION_COMBINATION;
			}
			options |= PRINT_SIZE;
		}
	}

	if (options & (ENCODE | DECODE)) {
		if (argc - i != 2) {
			cout << INVALID_ARG;
			return EC_INVALID_ARG;
		}
		ifstream is{ argv[i] , ios_base::in | ios_base::binary };
		if (!is.good()) {
			cout << "Cannot open source file.";
			return EC_UNABLE_TO_OPEN_FILE;
		}
		ofstream os{ argv[i + 1] , ios_base::out | ios_base::binary };
		if (!os.good()) {
			cout << "Cannot open destination file.";
			return EC_UNABLE_TO_OPEN_FILE;
		}

		if (options & ENCODE) {
			Huffman::Encoding(is, os);
		}
		else
			Huffman::Decoding(is, os);
		
		if (options & PRINT_SIZE)
			PrintSize(argv[i], argv[i + 1]);
	}
	else if (options & HELP) {
		PrintHelp();
	}
	else {
		cout << INVALID_ARG;
		return EC_INVALID_ARG;
	}

	return EC_GOOD;
}
