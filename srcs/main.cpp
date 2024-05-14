#include <iostream>
#include <map>
#include <fstream>
#include <vector>
#include <string>


int main(int argc, char** argv)
{
    //Safeguard that the program uses only one argument
     if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    //Standarize the config files folder
    std::string path = "../Configs/";

    // Concatenate the standarized path folder to the desired .conf file
    std::string file = path + argv[1];

    // C++ Method to "Read from file"
    std::ifstream inputFile(path);
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return 1;
    }

    //Buffer line to retrive the file contents
    std::string line;

    // C++ Method to "Get Next Line"
    while (std::getline(inputFile, line)) {
        std::cout << line << std::endl;
    }

    // Close the file
    inputFile.close();
    return 0;
}