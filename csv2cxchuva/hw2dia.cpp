#include "estacao.h"
#include <experimental/filesystem>
#include <string>

using namespace std;
namespace fs = std::experimental::filesystem;

int main(int argc, char *argv[]){
    class_estacao est;
    if(argc!=3){
        cout<<"use: "<<argv[0]<<" <DIR_ENTRADA/> <DIR_SAIDA/>"<<endl;
        return(1);
    }

    int result = 0;
    for(const auto& entry : fs::directory_iterator(argv[1])) {
        if(entry.path().extension() == ".txt") {
            est.open_hw(entry.path().string());
            result += est.save_dia(argv[2]) + est.save_mes(argv[2]);
        }
    }

    return result;
}
