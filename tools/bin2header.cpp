
#include <filesystem>
#include <string>

bool FileWrite(const std::string& fileName, const uint8_t* data, size_t size) {
	std::ofstream file(fileName, std::ios::binary | std::ios::trunc);
	if (file.is_open()) {
		file.write((const char*)data, (std::streamsize)size);
		file.close();
		return true;
	}
	return false;
}

bool Bin2H(const char* data, size_t size, const std::string& dst_filename, const char* dataName) {
	std::string ss;
	ss += "const char ";
	ss += dataName ;
	ss += "[] = {";
	for (size_t i = 0; i < size; i++) {
		if (i % 32 == 0) {
			ss += "\n";
		}
		ss += std::to_string((unsigned int)data[i]) + ",";
	}
	ss += "\n};\n";
	return FileWrite(dst_filename, (uint8_t*)ss.c_str(), ss.length());
}

