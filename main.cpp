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
#define EC_JUST_ERROR -1
#define EC_INVALID_ARG -2
#define EC_UNABLE_TO_OPEN_FILE -3
#define EC_INVALID_OPTION_COMBINATION -4
#define EC_SAME_PATH -5

// Options
#define ENCODE			01
#define DECODE			02
#define HELP			04
#define PRINT_SIZE		010
#define REMOVE_SOURCE	020

using namespace std;
namespace fs = std::filesystem;

void PrintHelp();
void PrintSize(const fs::path& src, const fs::path& dst);
int FillOption(int& option, char* str);

int main(int argc, char* argv[])
try {
	int options = 0;

	int i;
	for (i = 1; i < argc && argv[i][0] == '-'; i++)
		FillOption(options, argv[i] + 1);

	if (options & HELP) {
		PrintHelp();
		return EC_GOOD;
	}

	fs::path dst_path;

	if (options & ENCODE) {
		if (argc - i == 2) {
			dst_path = argv[i + 1];
			if (fs::is_directory(dst_path))
				dst_path = dst_path / fs::path(argv[i]).filename().replace_extension("huf");
		}
		else if (argc - i == 1) {
			dst_path = fs::path(argv[i]).replace_extension("huf");
		}
		else {
			cerr << INVALID_ARG;
			return EC_INVALID_ARG;
		}

		if (argv[i] == dst_path) {
			cerr << SAME_PATH;
			return EC_SAME_PATH;
		}

		ofstream os{ dst_path, ios_base::binary };
		if (!os.good()) {
			auto ec = make_error_code(huf_errc::invalid_fstream);
			throw fs::filesystem_error{ "main", dst_path, ec };
		}

		Huffman::Compress(argv[i], os);
	}
	else if (options & DECODE) {
		if (argc == i) {
			cerr << INVALID_ARG;
			return EC_INVALID_ARG;
		}

		ifstream is{ argv[i], ios_base::binary };
		if (!is.good()) {
			auto ec = make_error_code(huf_errc::invalid_fstream);
			throw fs::filesystem_error{ "main", argv[i], ec };
		}

		dst_path = (argc - i == 2) ? argv[i + 1] : fs::path(argv[i]).parent_path();

		if (argv[i] == dst_path) {
			cerr << SAME_PATH;
			return EC_SAME_PATH;
		}

		dst_path /= Huffman::DecompressRetFilename(is, dst_path);

	}
	else {
		cerr << INVALID_ARG;
		return EC_INVALID_ARG;
	}

	if (options & PRINT_SIZE)
		PrintSize(argv[i], dst_path);

	if (options & REMOVE_SOURCE)
		fs::remove_all(argv[i]);

	return EC_GOOD;
}
catch (fs::filesystem_error& e) {
	cout << e.what() << endl;
	return e.code().value();
}
catch (exception& e) {
	cout << e.what() << endl;
	return EC_JUST_ERROR;
}
catch (...) {
	cout << "Unknown error";
}

void PrintHelp()
{
	cout << "usage: app_name [options] source [destination]\n"
			"ex) huffman -e -s -del source.txt destination.huf\n"
			"  options:\n"
			"    All options are compared by first letter only\n"
			"    -h  (help) print help. No source and destination input required.\n"
			"    -e  (encode) Compress the source and save it to the destination.\n"
			"    -d  (decode) Decompress the source and save it to the destination.\n"
			"    -s  (size) Print the size of the source file and destination file.\n"
			"    -r  (delete) Delete source file.\n"
			"  source:\n"
			"    Path to the target file to be compressed or decompressed.\n"
			"    Cannot be the same as the destination\n"
			"  destination:\n"
			"    The path to the file in which to save the compressed or unpacked results.\n"
			"    Cannot be the same as the source\n";
}

uintmax_t GetDirectorySize(const fs::path& dir_path)
{
	uintmax_t size = 0;
	for (const auto& entry : fs::recursive_directory_iterator(dir_path))
		size += entry.file_size();
	
	return size;
}

void PrintSize(const fs::path& src, const fs::path& dst)
{
	auto source_size = fs::is_directory(src) ? 
		GetDirectorySize(src) : fs::file_size(src);
	auto destination_size = fs::is_directory(dst) ? 
		GetDirectorySize(dst) : fs::file_size(dst);
	
	cout << "source: " << source_size << "bytes, ";
	cout << "destination: " << destination_size << "bytes, ";

	if (source_size >= destination_size) {
		cout << "decrease: " << source_size - destination_size << "bytes, ";
		cout << (1 - (long double)destination_size / source_size) * 100 << '%';
	}
	else {
		cout << "increase: " << destination_size - source_size << "bytes, ";
		cout << ((long double)destination_size / source_size - 1) * 100 << '%';
	}
	cout << endl;
}

int FillOption(int& option, char* str)
{
	switch (*str) {
	case 'e':
		if (option & (DECODE | HELP)) {
			cerr << INVALID_OPTION_COMBINATION;
			return EC_INVALID_OPTION_COMBINATION;
		}
		option |= ENCODE;
		return EC_GOOD;
	case 'd':
		if (option & (ENCODE | HELP)) {
			cerr << INVALID_OPTION_COMBINATION;
			return EC_INVALID_OPTION_COMBINATION;
		}
		option |= DECODE;
		return EC_GOOD;
	case 'h':
		if (option & (ENCODE | DECODE | PRINT_SIZE | REMOVE_SOURCE)) {
			cerr << INVALID_OPTION_COMBINATION;
			return EC_INVALID_OPTION_COMBINATION;
		}
		option |= HELP;
		return EC_GOOD;
	case 's':
		if (option & HELP) {
			cerr << INVALID_OPTION_COMBINATION;
			return EC_INVALID_OPTION_COMBINATION;
		}
		option |= PRINT_SIZE;
		return EC_GOOD;
	case 'r':
		if (option & HELP) {
			cerr << INVALID_OPTION_COMBINATION;
			return EC_INVALID_OPTION_COMBINATION;
		}
		option |= REMOVE_SOURCE;
	default:
		return EC_INVALID_OPTION_COMBINATION;
	}
}