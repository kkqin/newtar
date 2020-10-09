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

unsigned int oct2uint(const char* src, int read_size) {
        unsigned int result = 0;
        int i = 0;
        while(i < read_size) {
                unsigned int byte_num = (unsigned int)src[i++] - '0';
                result = (result << 3) | byte_num;
        }
        return result;
}

namespace mytar {

Hub* Hub::m_instance = NULL;

std::shared_ptr<Block> Hub::get_block(const std::string name) {
	auto it = m_result.find(name);
	if(it != m_result.end())
		return it->second;
	return nullptr;
}

WholeTar::WholeTar(const char* file) : m_name(file) {
	m_file.open(m_name, std::ifstream::in | std::ifstream::binary);
}

WholeTar::~WholeTar() {
	m_file.close();
}

void WholeTar::parsing() {
	std::queue<std::shared_ptr<TAR_HEAD>> judge_queue;
	bool longname_ = false;
	long long off_size_ = 0;
	unsigned read_size_ = 512;
	while(m_file) {
		std::shared_ptr<TAR_HEAD> tar = std::make_shared<TAR_HEAD>();
		m_file.read(tar->block, read_size_);

		tar->id = off_size_;	
		off_size_ += read_size_;
		if(judge_queue.size() >= 2)
			judge_queue.pop();

		judge_queue.push(tar);
		auto prev = judge_queue.front();
		auto file_size = oct2uint(prev->size, 11);
		auto block_size = strlen(tar->block) + 1;

		if(prev->type == lf_longname
				&& file_size == block_size && !strncmp(prev->ustar, "ustar", 5)) {
			tar->itype = HeadType::LONGNAME_HEAD;
			longname_ = true;
		}
		else if(!strncmp(tar->ustar, "ustar", 5)){
			tar->itype = HeadType::HEAD;
		}
		else {
			tar->itype = HeadType::BODY;
		}

		if( longname_ && tar->itype == HeadType::LONGNAME_HEAD) {
			Hub::instance()->m_result[tar->block] = std::make_shared<Block>(tar->id, true, file_size);
		}
		else if(tar->itype == HeadType::HEAD && tar->type != lf_longname && prev->itype != HeadType::LONGNAME_HEAD) {
			Hub::instance()->m_result[tar->name] = std::make_shared<Block>(tar->id, false, oct2uint(tar->size, 11));
		}
	}

	m_file.close();
}

void WholeTar::show_all_file() {
	for(auto it : Hub::instance()->m_result) {
		std::cout << "offsize:" << it.second->offsize << " " << it.first << std::endl;
	}
}

int recusive_mkdir(const char* dirname) {
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

bool WholeTar::extract_file(const std::string name) {
	auto block = Hub::instance()->get_block(name);

	if(block == nullptr)
		return false;

	auto start_pos = block->offsize;
	auto filesize = block->filesize;

	/// create file directly
	recusive_mkdir(name.c_str());
	std::ofstream o(name.c_str(), std::ofstream::binary);

	m_file.open(m_name, std::ifstream::in | std::ifstream::binary);
	start_pos += 512;
	m_file.seekg(start_pos);
	if(block->is_longname) {
		auto tar = std::make_shared<TAR_HEAD>();
		m_file.read(tar->block, 512);
		filesize = oct2uint(tar->size, 11);
		start_pos += 512;
	}

	if(!filesize)
		return false;
	while(filesize) {
		auto need_size = 512; 
		if(filesize < 512)
			need_size = filesize; 
			
		char* buffer = new char[need_size];
		m_file.read(buffer, need_size);
		o.write(buffer, need_size);
		delete [] buffer;
		filesize -= need_size;
	}

	o.close();
	m_file.close();
	return true;
}

}
