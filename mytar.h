#pragma once

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

bool is_tar_head(char* block); 

struct spe_tar{
	spe_tar() {}
	spe_tar(long long sz, bool ok, long long fs, std::string n):
		offset(sz), 
		is_longname(ok), 
		filesize(fs),
		filename(n)
	{}
		
	long long offset;
	bool is_longname;
	long long filesize;
	std::string filename;
	bool is_hard_link;
	std::string linkfilename;
};
using Block = struct spe_tar;
using BlockPtr = std::shared_ptr<Block>;

class StandardTar{
public:
	StandardTar(const char* name) : m_name(name) {}
	virtual ~StandardTar() { m_file->close(); }
	virtual void parsing(std::function<void(std::map<long long, BlockPtr>)> func, bool verbose) = 0;
	virtual BlockPtr get_file_block(const long long offset); 
protected:
	ifStreamPtr m_file;
	std::string m_name;
};

class NTar : public StandardTar {
public:
	NTar(const char* file);
	virtual void parsing(std::function<void(std::map<long long, BlockPtr>)> func, bool verbose=false) override;
	void show_all_file();
	//bool extract_file(const std::string name); 
	virtual BlockPtr get_file_block(const long long offset) override;
	ifStreamPtr back_file(); 
};

}
