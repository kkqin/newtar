#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <map>
#include <queue>
#include <functional>
#include <any>

struct _tar_head;
using TAR_HEAD = struct _tar_head;

using ifStreamPtr = std::shared_ptr<std::ifstream>;
ifStreamPtr open_tar_file(const std::string& tarfile);

namespace mytar {

struct spe_tar{
	spe_tar(long long sz, bool ok, long long fs):
		offsize(sz), is_longname(ok), filesize(fs) {}
	long long offsize;
	bool is_longname;
	long long filesize;
};
using Block = struct spe_tar;
using BlockPtr = std::shared_ptr<Block>;

struct WholeTar {
	WholeTar(const char* file);
	~WholeTar();
	void parsing(std::function<void(std::map<std::string, std::shared_ptr<Block>>)> func);
	void show_all_file();
	bool extract_file(const std::string name); 
	std::shared_ptr<Block> get_file_block(const std::string& name);

	std::shared_ptr<std::ifstream> m_file;
	std::string m_name;
};

}
