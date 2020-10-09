#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <map>
#include <queue>

struct _tar_head;
using TAR_HEAD = struct _tar_head;

namespace mytar {

struct spe_tar{
	spe_tar(long long sz, bool ok, long long fs):
		offsize(sz), is_longname(ok), filesize(fs) {}
	long long offsize;
	bool is_longname;
	long long filesize;
};

using Block = struct spe_tar;

class Hub{	
	static Hub* m_instance;
public:
	std::map<std::string, std::shared_ptr<Block>> m_result;
	static Hub* instance() {
		if(!m_instance) {
			m_instance = new Hub;
		}
		return m_instance;
	}

	std::shared_ptr<Block> get_block(const std::string name);
};

class WholeTar {
public:
	WholeTar(const char* file);
	~WholeTar();
	void parsing();
	void show_all_file();
	bool extract_file(const std::string name); 
private:
	std::ifstream m_file;
	std::string m_name;
};

}
