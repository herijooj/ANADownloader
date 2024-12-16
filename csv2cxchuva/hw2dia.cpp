#include "estacao.h"
#include <experimental/filesystem>
#include <string>
#include <algorithm>
#include <vector>

using namespace std;
namespace fs = std::experimental::filesystem;

int main(int argc, char *argv[]){
    if(argc!=3){
        cout<<"use: "<<argv[0]<<" <DIR_ENTRADA/> <DIR_SAIDA/>"<<endl;
        return(1);
    }

    // Get all files in directory
    vector<fs::path> files;
    for(const auto& entry : fs::directory_iterator(argv[1])) {
        if(entry.path().extension() == ".txt") {
            files.push_back(entry.path());
        }
    }

    // Sort files to ensure consistent processing order
    sort(files.begin(), files.end());

    int result = 0;
    for(const auto& file : files) {
        class_estacao est;
        est.open_hw(file.string());
        result += est.save_dia(argv[2]) + est.save_mes(argv[2]);
        printf("Processed %s\n", file.string().c_str());
    }

    printf("Total files processed: %zu\n", files.size());
    return result;
}
