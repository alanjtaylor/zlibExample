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


int ZEXPORT gzipuncompress2(Bytef *dest, uLongf *destLen, std::vector<char> source_data, uLong *sourceLen) {
  z_stream stream;
  int err;
  const uInt max = (uInt)-1;
  uLong len, left;
  Byte buf[1]; /* for detection of incomplete stream when *destLen == 0 */

  len = *sourceLen;
  if (*destLen) {
    left = *destLen;
    *destLen = 0;
  } else {
    left = 1;
    dest = buf;
  }

  stream.next_in = (z_const Bytef *)source_data.data();
  stream.avail_in = 0;
  stream.zalloc = (alloc_func)0;
  stream.zfree = (free_func)0;
  stream.opaque = (voidpf)0;

  err = inflateInit2(&stream, MAX_WBITS + 16);
  if (err != Z_OK)
    return err;

  stream.next_out = dest;
  stream.avail_out = 0;

  do {
    if (stream.avail_out == 0) {
      stream.avail_out = left > (uLong)max ? max : (uInt)left;
      left -= stream.avail_out;
    }
    if (stream.avail_in == 0) {
      stream.avail_in = len > (uLong)max ? max : (uInt)len;
      len -= stream.avail_in;
    }
    err = inflate(&stream, Z_NO_FLUSH);
  } while (err == Z_OK);

  *sourceLen -= len + stream.avail_in;
  if (dest != buf)
    *destLen = stream.total_out;
  else if (stream.total_out && err == Z_BUF_ERROR)
    left = 1;

  inflateEnd(&stream);
  return err == Z_STREAM_END ? Z_OK : err == Z_NEED_DICT ? Z_DATA_ERROR : err == Z_BUF_ERROR && left + stream.avail_out ? Z_DATA_ERROR : err;
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

	std::cout << "size of unzipped data via gz open/read/close: " << unzipped_data.size() << std::endl;

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

	std::vector<char> compressed_data(data, data + len);
	std::vector<char> uncompressed_data;

	// buffer out length with a factor of 10 greater than the compressed length, will later be overwritten
	uLong buffer_out_length = (len * 10);
	uLong compressed_length = len;
	char *buffer_out = new char[buffer_out_length];

	while (true) {

		auto ret = gzipuncompress2((Byte *)buffer_out, &buffer_out_length, compressed_data, &compressed_length);

		// add to the uncompressed data vec
		uncompressed_data.insert(uncompressed_data.end(), buffer_out, buffer_out + buffer_out_length);
		// re-size compressed data vec
		std::vector<char>(compressed_data.begin() + compressed_length, compressed_data.end()).swap(compressed_data);

		// re-set the size of buffer_out_size and compressed_length
		compressed_length = len;
		buffer_out_length = len * 10;

		if (ret != 0) {
			break;
		}
	}

	delete[] buffer_out;

	std::cout << "size of unzipped data via char array decompression: " << uncompressed_data.size() << std::endl;

	return 0;
}
