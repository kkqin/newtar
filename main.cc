#include "mytar.h"

int main(int argc, char* argv[]) {
	mytar::NTar tar{argv[1]};
	tar.parsing([](std::map<long long, std::shared_ptr<mytar::Block>> m){
		for(auto it : m) {
			std::cout << it.first << " " << it.second->filename << std::endl;
		}
	});

	//auto res = tar.extract_file("CQjtKCjtubBPbuCnSHashFRPfeuYHOasdsQndsxvKJICaXufkyxrsdfgxFMtjHSIfpBMuuwJrYSAsDhfjYfhRZe_rZ_PDgvsiYEdW_lolasdfasdfasdfsadfadsfsadfsadfas/picohash.h");
	//if( !res )
	//	std::cout << "only support single file." << std::endl;

	//tar.show_all_file();

	//auto block = tar.get_file_block("CQjtKCjtubBPbuCnSHashFRPfeuYHOasdsQndsxvKJICaXufkyxrsdfgxFMtjHSIfpBMuuwJrYSAsDhfjYfhRZe_rZ_PDgvsiYEdW_lolasdfasdfasdfsadfadsfsadfsadfas/picohash.h");

	return 0;
}
