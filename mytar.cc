#include "mytar.h"
#include <cstring>
#include <filesystem>

#define  lf_oldnormal '\0'       /* normal disk file, unix compatible */
#define  lf_normal    '0'        /* normal disk file */
#define  lf_link      '1'        /* link to previously dumped file */
#define  lf_symlink   '2'        /* symbolic link */
#define  lf_chr       '3'        /* character special file */
#define  lf_blk       '4'        /* block special file */
#define  lf_dir       '5'        /* directory */
#define  lf_fifo      '6'        /* fifo special file */
#define  lf_contig    '7'        /* contiguous file */
#define  lf_longname  'L'

#pragma pack(1)
typedef struct _tar_head {
        union {
		struct {
			char name[100];
			char mode[8];
			char uid[8];
			char gid[8];
			char size[12];
			char mtime[12];
			char chksum[8];
			char type;
			char link_name[100];
			char ustar[8];
			char owner[32];
			char group[32];
			char major[32];
			char minor[32];
		};
                char block[512];
        };
	int itype;
	long long id;
}TAR_HEAD;

enum HeadType {
	HEAD = 1,
	BODY,
	LONGNAME_HEAD
};

static unsigned int oct2uint(const char* src, int read_size) {
        unsigned int result = 0;
        int i = 0;
        while(i < read_size) {
                unsigned int byte_num = (unsigned int)src[i++] - '0';
                result = (result << 3) | byte_num;
        }
        return result;
}

static int recusive_mkdir(const char* dirname) {
        const size_t len = strlen(dirname);
        if (!len)
                return -1;

        char* path = (char*)calloc(len + 1, sizeof(char));
        strncpy(path, dirname, len);

        if (path[len - 1] == '/')
                path[len - 1] = 0;

        for(char *p = path; *p; p++) {
                if (*p == '/') {
                        *p = '\0';

#ifdef  __linux__
                        std::filesystem::create_directory(path);
#elif   WIN32
                        create_directory(path);
#endif
                        *p = '/';
                }
        }

	free(path);
	return 0;
}

ifStreamPtr open_tar_file(const std::string& tarfile) {
	auto m = ifStreamPtr(new std::ifstream());
	m->open(tarfile, std::ifstream::in | std::ifstream::binary);
	return m;
}

static void arrange_block(const std::string& tarfile, std::shared_ptr<TAR_HEAD> tar, mytar::BlockPtr bl) {
	if(!bl->is_longname && tar->type == lf_normal) {
		bl->offset += 512;
	} else {
		auto file = open_tar_file(tarfile);
		bl->offset += 512;
		file->seekg(bl->offset);
		TAR_HEAD* tar = new TAR_HEAD;
		file->read(tar->block, 512);
		bl->filesize = oct2uint(tar->size, 11);
		bl->offset += 512; // point to start
		file->close();
	}

	if(tar->type == lf_link) {
		bl->is_hard_link = true; 
		bl->linkfilename = tar->link_name;
	}
}

namespace mytar {

static const char magic_word[24] = {
         0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00,
         0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00,
         0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00
};

bool is_tar_head(char* block) {
	if(!strncmp(&block[257], "ustar", 5)) {
		char* owner = &block[257 + 8];
		char* group = owner + 32;
		if(!strcmp(owner, group)) 
			return true;
	}

	for(auto i = 0; i < sizeof(magic_word); i++) {
		if(block[100 + i] != magic_word[i])
			return false;
	}
	return true;
}

class Parsing {
private:
	std::string snppiner = "|/-\\";

	void do_draw(int process) {
		std::cout << "\r";
		std::cout << "Parsing... " <<  snppiner[process % 4] << std::flush; 
	}
public:
	
        void do_parsing(bool show, std::shared_ptr<std::ifstream> m_file, 
			std::function<void(bool& longname_,std::queue<std::shared_ptr<TAR_HEAD>> judge_queue)> judge_func) {
		std::queue<std::shared_ptr<TAR_HEAD>> judge_queue;
		bool longname_ = false;
		long long off_set_ = 0;
		unsigned read_size_ = 512;
		unsigned process = 0;
		while(*m_file) {
			m_file->seekg(off_set_);
			std::shared_ptr<TAR_HEAD> tar = std::make_shared<TAR_HEAD>();
			m_file->read(tar->block, read_size_);

			off_set_ += 512;
			tar->id = off_set_;	
			auto inside_file_size = oct2uint(tar->size, 11);

			off_set_ += inside_file_size;
			auto res = inside_file_size % 512; 
                        if(res) {
                                off_set_ += (512 - res);
                        }

			if(judge_queue.size() >= 2)
				judge_queue.pop();

			judge_queue.push(tar);
			judge_func(longname_, judge_queue);

			if(show)
				do_draw(process++);
		}

		std::cout << std::endl;
		m_file->close();
        }
};

class Hub{	
	static Hub* m_instance;
public:
	std::map<std::string, std::map<long long, BlockPtr>> m_result; //offset, block

	static Hub* instance() {
		if(!m_instance) {
			m_instance = new Hub;
		}
		return m_instance;
	}

	BlockPtr get_block(const std::string tarfile, const long long offset);
};

Hub* Hub::m_instance = NULL;

BlockPtr Hub::get_block(const std::string tarfile, const long long offset) {
	auto it = m_result.find(tarfile);
	if(it != m_result.end()) {
		auto files = it->second;
		auto it2 = files.find(offset);
		if(it2 != files.end()) {
			return it2->second;
		}
		else
			return nullptr;
	}
	return nullptr;
}

BlockPtr StandardTar::get_file_block(const long long offset) {
	auto block = Hub::instance()->get_block(m_name, offset);
	return block;
}

NTar::NTar(const char* file) : StandardTar(file) {
	m_file = open_tar_file(file);
}

void NTar::parsing(std::function<void(std::map<long long, BlockPtr>)> func, bool verbose) {
	Parsing core_parse;
	core_parse.do_parsing(verbose, m_file, [this](bool& longname_, std::queue<std::shared_ptr<TAR_HEAD>> judge_queue) {
		auto prev = judge_queue.front();
		auto file_size = oct2uint(prev->size, 11);
		auto tar = judge_queue.back();
		auto block_size = strlen(tar->block) + 1;

		auto prev_dect = is_tar_head( prev->block );
		auto now_dect = is_tar_head( tar->block );

		if(prev->type == lf_longname
				&& file_size == block_size && prev_dect) {
			tar->itype = HeadType::LONGNAME_HEAD;
			longname_ = true;
		}
		else if(now_dect){
			tar->itype = HeadType::HEAD;
		}
		else {
			tar->itype = HeadType::BODY;
		}

		if(longname_ && tar->itype == HeadType::LONGNAME_HEAD) {
			auto block = std::make_shared<Block>(tar->id, true, file_size, tar->block);
			arrange_block(m_name, tar, block);
			Hub::instance()->m_result[m_name].insert({tar->id, block});
		}
		else if(tar->itype == HeadType::HEAD && tar->type != lf_longname && prev->itype != HeadType::LONGNAME_HEAD) {
			auto block = std::make_shared<Block>(tar->id, false, file_size, tar->name);
			arrange_block(m_name, tar, block);
			Hub::instance()->m_result[m_name].insert({tar->id, block});
		}
	});

	func(Hub::instance()->m_result[m_name]);
}

BlockPtr NTar::get_file_block(const long long name) {
	return StandardTar::get_file_block(name);
}

void NTar::show_all_file() {
	for(auto it : Hub::instance()->m_result[m_name]) {
		std::cout << "offset:" << it.second->offset << "name: " << it.second->filename << std::endl;
	}
}

ifStreamPtr NTar::back_file() {
	m_file = open_tar_file(m_name);
	m_file->seekg(0);
	return m_file;
}

/*bool NTar::extract_file(const std::string name) {
	auto block = Hub::instance()->get_block(m_name, name);

	if(block == nullptr)
		return false;

	auto start_pos = block->offset;
	auto filesize = block->filesize;

	/// create file directly
	recusive_mkdir(name.c_str());
	std::ofstream o(name.c_str(), std::ofstream::binary);

	m_file = open_tar_file(m_name);
	m_file->seekg(start_pos);

	if(!filesize)
		return false;
	while(filesize) {
		auto need_size = 512; 
		if(filesize < 512)
			need_size = filesize; 
			
		char* buffer = new char[need_size];
		m_file->read(buffer, need_size);
		o.write(buffer, need_size);
		delete [] buffer;
		filesize -= need_size;
	}

	o.close();
	m_file->close();
	return true;
}*/

////////////////

} // namspace 
