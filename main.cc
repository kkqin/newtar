#include "mytar.h"

int main() {
	mytar::NTar tar{"new.tar"};
	tar.parsing([](std::map<std::string, std::shared_ptr<mytar::Block>> m){
		for(auto it : m) {
			std::cout << it.first << " " << it.second->offsize << std::endl;
		}
	});

	auto res = tar.extract_file("CQjtKCjtubBPbuCnSHashFRPfeuYHOasdsQndsxvKJICaXufkyxrsdfgxFMtjHSIfpBMuuwJrYSAsDhfjYfhRZe_rZ_PDgvsiYEdW_lolasdfasdfasdfsadfadsfsadfsadfas/picohash.h");
	if( !res )
		std::cout << "only support single file." << std::endl;

	//tar.show_all_file();

	auto block = tar.get_file_block("CQjtKCjtubBPbuCnSHashFRPfeuYHOasdsQndsxvKJICaXufkyxrsdfgxFMtjHSIfpBMuuwJrYSAsDhfjYfhRZe_rZ_PDgvsiYEdW_lolasdfasdfasdfsadfadsfsadfsadfas/picohash.h");

	return 0;
}
