#include "mytar.h"

int main() {
	mytar::WholeTar tar{"new.tar"};
	tar.parsing();
	tar.show_all_file();

	auto res = tar.extract_file("CQjtKCjtubBPbuCnSHashFRPfeuYHOasdsQndsxvKJICaXufkyxrsdfgxFMtjHSIfpBMuuwJrYSAsDhfjYfhRZe_rZ_PDgvsiYEdW_lolasdfasdfasdfsadfadsfsadfsadfas/picohash.h");
	if( !res )
		std::cout << "only support single file." << std::endl;

	return 0;
}
