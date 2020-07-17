#include <iostream>
#include "zlib.h"
#include <cstring>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

#define CHECK_ERR(err, msg)                        \
  {                                                \
    if (err != Z_OK) {                             \
      fprintf(stderr, "%s error: %d\n", msg, err); \
      exit(1);                                     \
    }                                              \
  }


void test_inflate(Byte *compr, uLong comprLen, Byte *uncompr, uLong *uncomprLen) {

	int err;
	z_stream d_stream; /* decompression stream */
	d_stream.zalloc = NULL;
	d_stream.zfree = NULL;
	d_stream.opaque = NULL;
	d_stream.next_in = compr;
	d_stream.avail_in = 0;
	d_stream.next_out = uncompr;
	err = inflateInit2(&d_stream, MAX_WBITS + 16);
	CHECK_ERR(err, "inflateInit");

	while (d_stream.total_out < *uncomprLen && d_stream.total_in < comprLen) {
		d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
		err = inflate(&d_stream, Z_NO_FLUSH);
		if (err == Z_STREAM_END)
			break;
		CHECK_ERR(err, "inflate");
	}
	err = inflateEnd(&d_stream);
	*uncomprLen = d_stream.total_out;

}

int main() {

	std::string input_file_path = "zippedFile.gz";

	// using utility gz methods

	char gz_char_array[input_file_path.length() + 1];
	std::strcpy(gz_char_array, input_file_path.c_str());

	gzFile in_file_gz = gzopen(gz_char_array, "rb");
	char unzip_buffer[8192];
	int unzipped_bytes;
	std::vector<char> unzipped_data;

	while (true) {
		unzipped_bytes = gzread(in_file_gz, unzip_buffer, 8192);
		if (unzipped_bytes > 0) {
			unzipped_data.insert(unzipped_data.end(), unzip_buffer, unzip_buffer + unzipped_bytes);
		} else {
			break;
		}
	}

	gzclose(in_file_gz);

	std::cout << "size of unzipped data: " << unzipped_data.size() << std::endl;

	// obtain char array

	ifstream ifs(input_file_path, ios::binary | ios::ate);
	ifs.seekg(0, ios::beg);
	int begin = ifs.tellg();
	int end = begin;
	int len = 0;

	ifs.seekg(0, ios_base::end);
	end = ifs.tellg();
	len = end - begin;

	char *data = new char[len];
	ifs.seekg(0, ios_base::beg);
	ifs.read(data, len);
	ifs.close();

	uLong buffer_out_size = (len * 10); // initial value 10 times bigger than input size, later will be overwritten with the correct output size
	char *buffer_out = new char[buffer_out_size];

	test_inflate((Byte *)data, len, (Byte *)buffer_out, &buffer_out_size);

	std::cout << "size of unzipped data via char array: " << buffer_out_size << std::endl;

	return 0;
}
