#include <catch.hpp>
#include <lz4cpp.hpp>

#include <chrono>
#include <sstream>

void testCompression(int size, int level)
{
	std::vector<int> data(size / 4);
	//create test data
	for (int i = 0; i < size / 4; ++i)
		data[i] = i/653;

	//compress
	std::stringstream s;
	auto start = std::chrono::steady_clock::now();
	{
		LZ4Compressor c(level);
		for (int offset=0; offset<size; offset+=LZ4Compressor::MAX_CHUNK_SIZE)
		{
			const char* mem = reinterpret_cast<const char*>(data.data()) + offset;
			const int len = std::min(size - offset, LZ4Compressor::MAX_CHUNK_SIZE);
			c.compress(s, mem, len);
		}
	}
	auto end = std::chrono::steady_clock::now();
	int compressedSize = static_cast<int>(s.tellp());
	int compressionMs = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());

	//decompress
	s.seekg(0);
	std::vector<int> output(size / 4);
	start = std::chrono::steady_clock::now();
	{
		LZ4Decompressor d;
		for (int offset = 0; offset < size;)
		{
			char* mem = reinterpret_cast<char*>(output.data()) + offset;
			const int len = size - offset;
			int chunkSize = d.decompress(mem, len, s);
			offset += chunkSize;
		}
	}
	end = std::chrono::steady_clock::now();
	int decompressionMs = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
	
	//compare
	bool ok = true;
	for (int i = 0; i < size / 4; ++i)
		ok = data[i] == output[i] ? ok : false;

	printf("s=% 8dKB, l=%d -> compressed % 8dKB, time compress=% 4dms, decompress=% 4dms, ok=%d\n",
		size/1024, level, compressedSize/1024, compressionMs, decompressionMs, int(ok));
	INFO("size: " << size << ", level: " << level);
	REQUIRE(ok);
}

TEST_CASE("TestCompression")
{
	std::vector<int> sizes{ 1 << 10, 1 << 12, 1 << 14, 1 << 16, 1 << 18, 1 << 20, 1 << 22, 1 << 24, 1 << 26 };
	std::vector<int> levels{
		LZ4Compressor::FAST_COMPRESSION, LZ4Compressor::MIN_COMPRESSION, LZ4Compressor::MAX_COMPRESSION };

	for (int size : sizes) for (int level : levels)
		testCompression(size, level);
}
