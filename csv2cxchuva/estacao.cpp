
int class_estacao::open_hw( std::string fname ){
    if(fname.size()>0){
        std::ifstream fin( fname.c_str() );
        std::string buf;
        float total_txt = 0;

        this->cod=basename(fname);
        while( getline(fin,buf) ) {
            // Verifica se a linha tem conteúdo suficiente antes de processar
            if(buf.size() < 2 || buf[0]=='/') {
                continue;
            }

            buf += ";"; // Adiciona separador no final
            
            int consist=0, ano=0, mes=0;
            float dias[31] = {UNDEF}; // Inicializa com UNDEF
            int vstat[31] = {0};      // Inicializa com 0
            
            std::string token;
            int wrd = 0;
            size_t pos = 0;
            
            while((pos = buf.find(';')) != std::string::npos) {
                token = buf.substr(0, pos);
                buf.erase(0, pos + 1);
                
                try {
                    if(wrd == 1) {
                        consist = atoi(token.c_str());
                    } 
                    else if(wrd == 2) {
                        size_t slash_pos = token.find_first_of("/");
                        if(slash_pos != std::string::npos) {
                            std::string date = token.substr(slash_pos + 1);
                            size_t second_slash = date.find_first_of("/");
                            if(second_slash != std::string::npos) {
                                mes = atoi(date.substr(0, second_slash).c_str());
                                ano = atoi(date.substr(second_slash + 1).c_str());
                            }
                        }
                    }
                    else if(wrd == 5) {
                        if(token.find_first_of(',') != std::string::npos) {
                            token[token.find_first_of(",")] = '.';
                        }
                        total_txt = token.empty() ? UNDEF : atof(token.c_str());
                    }
                    else if(wrd >= 13 && wrd <= 43) {
                        if(token.find_first_of(',') != std::string::npos) {
                            token[token.find_first_of(",")] = '.';
                        }
                        dias[wrd-13] = token.empty() ? UNDEF : atof(token.c_str());
                    }
                    else if(wrd >= 44 && wrd <= 74) {
                        vstat[wrd-44] = token.empty() ? 
                            (dias[wrd-44] == UNDEF ? 0 : 1) : 
                            atoi(token.c_str());
                    }
                }
                catch(std::exception& e) {
                    std::cerr << "Error processing file " << fname << ": " << e.what() << std::endl;
                    continue;
                }
                
                wrd++;
            }

            if(consist == 1) {
                class_mes novo_mes(mes, ano);
                novo_mes.set_val_txt(total_txt);
                for(int i=1; i<=31; i++) {
                    novo_mes.add_dia(i, dias[i-1], vstat[i-1]);
                }
                if(novo_mes.get_nvalid() > 0) {
                    this->add_mes(novo_mes);
                }
            }
        }
        fin.close();
    }
    return 0;
}
