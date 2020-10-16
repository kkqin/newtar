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

enum class Mode {
	NORMAL_TAR = 0, 
	XVA
};

namespace mytar {

struct spe_tar{
	spe_tar(long long sz, bool ok, long long fs):
		offset(sz), is_longname(ok), filesize(fs) {}
	long long offset;
	bool is_longname;
	long long filesize;
};
using Block = struct spe_tar;
using BlockPtr = std::shared_ptr<Block>;

class StandardTar{
public:
	StandardTar(const char* name) : m_name(name) {}
	virtual ~StandardTar() { m_file->close(); }
	virtual void parsing(std::function<void(std::map<std::string, BlockPtr>)> func) = 0;
	virtual BlockPtr get_file_block(const std::string& name); 
protected:
	std::shared_ptr<std::ifstream> m_file;
	std::string m_name;
};


class NTar : public StandardTar {
public:
	NTar(const char* file);
	virtual void parsing(std::function<void(std::map<std::string, BlockPtr>)> func) override;
	void show_all_file();
	bool extract_file(const std::string name); 
	virtual BlockPtr get_file_block(const std::string& name);
};

class XTar : public StandardTar {
public:
	XTar(const char* file);
	virtual void parsing(std::function<void(std::map<std::string, BlockPtr>)> func) override;
	virtual BlockPtr get_file_block(const std::string& name);
};

}
