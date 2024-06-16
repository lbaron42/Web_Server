
#include <iostream>
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include "Config.hpp"
#include "Log.hpp"

namespace {
	static char const	*tests_log = "tests/unit_tests/tests.log";
	static const char	*non_existing_file = "/pathDontExist/gostFile.conf";
	Log					log;
}

bool testSuccessFiles(const std::string& directory)
{
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(directory.c_str())) != NULL){
		while ((ent = readdir(dir)) != NULL){
			if (ent->d_type == DT_REG){
				std::string filePath = directory + "/" + ent->d_name;
				Config conf(log);
				if (conf.configInit(filePath.c_str())){
					log << Log::ERROR << filePath << std::endl;
					closedir(dir);
					return EXIT_FAILURE;
			 	}
			}
		}
	}
	closedir(dir);
	return EXIT_SUCCESS;
}

bool testErrorFiles(const std::string& directory)
{

	DIR *dir;
	struct dirent *ent;
	if((dir = opendir(directory.c_str())) != NULL){
		while ((ent = readdir(dir)) != NULL){
			if (ent->d_type == DT_REG){
			std::string filePath = directory + "/" + ent->d_name;
			Config conf(log);
			if (!conf.configInit(filePath.c_str())){
					log << Log::ERROR << filePath << std::endl;
					closedir(dir);
					return EXIT_FAILURE;
				}
			}
		}
	}
	closedir(dir);
	return EXIT_SUCCESS;
}



int main() {

	Config conf(log);


	log.set_output(new std::ofstream(tests_log, \
		std::ios_base::ate | std::ios_base::app), true);
	log.set_verbosity(Log::DEBUG);

	if(!conf.configInit(non_existing_file))
		return EXIT_FAILURE;
	if(testErrorFiles("./config/config_errors"))
		return EXIT_FAILURE;
	if(testSuccessFiles("./config/config_success"))
		return EXIT_FAILURE;


	return EXIT_SUCCESS;
}
