#include "estacao.h"
/**
* Alterado por Lucas Nogueira (10/2022)
*
* Criando .DIA e .MES aceitando todos os status
* NÃO USAR ESSA SCRIPT PARA VALORES .DIA
* SAÍDA APENAS PARA PEGAR O .MES
**/

class_mes::class_mes( int mes, int ano ){
	this->mes=mes;
	this->ano=ano;
	this->maxd=ndias(mes,ano);
	this->valor=UNSET;
	this->valor_txt=UNSET;
	this->dias.clear();
	this->dias.assign( this->maxd, UNDEF );
	this->status.clear();
	this->status.assign( this->maxd, 0 );
	this->nvalid=0;
}

class_mes::class_mes( int mes, int ano, float valor ){
	this->mes=mes;
	this->ano=ano;
	this->maxd=ndias(mes,ano);
	this->valor=valor;
	this->valor_txt=UNSET;
	this->dias.clear();
	this->dias.assign( this->maxd, UNDEF );
	this->status.clear();
	this->status.assign( this->maxd, 0 );
	this->nvalid=0;
}

int class_mes::get_nvalid( void ){
	return( this->nvalid );
}

int class_mes::get_undefs( void ){
	int count = 0;

	for(int i = 1; i <= ndias(this->mes,this->ano); i++){
		//conta dias UNDEF
		if(this->get_dia/*_val*/(i) == UNDEF){
			count++;
		}
	}

	return count;
}

int class_mes::get_nonz( void ){
	int k=0;
	for(int i=1;i<=31;i++){
		float val=this->get_dia(i);
		if(val!=0&&val!=UNDEF)k++;
	}
	return(k);
}

float class_mes::get_dia( int dia ){
	if((dia >= 1) && (dia <= this->maxd) /*&& this->status[dia-1] < 4*/){
		return(this->dias[dia-1]);
	}
	return(UNDEF);
}

// float class_mes::get_dia_val( int dia ){
// 	if((dia >= 1) && (dia <= this->maxd)){
// 		return(this->dias[dia-1]);
// 	}
// 	return(UNDEF);
// }

int class_mes::get_stat( int dia ){
	if(dia>=1&&dia<=this->maxd) return(this->status[dia-1]);
	return(0);
}

float class_mes::get_val( void ){
	for(int i = 1; i <= ndias(this->mes,this->ano); i++){

		//soma o total mensal
		if(this->get_dia/*_val*/(i) != UNDEF){
			if (valor == UNSET) valor = 0;
			this->valor += this->get_dia/*_val*/(i);
		}
	}

	if(this->valor == UNSET) this->valor = UNDEF;

	return(this->valor);
}

float class_mes::get_val_txt( void ){
	return this->valor_txt;
}
void class_mes::set_val_txt( float new_val ){
	this->valor_txt = new_val;
}

int class_mes::get_mes( void ){
	return( this->mes );
}

int class_mes::get_ano( void ){
	return( this->ano );
}

int class_mes::add_dia( int dia, float val, int stat ){
	if( dia >= 1 && dia <= this->maxd ){

		// Status 4 serão lidos, porém se o 4  for o ultimo dia do mês, ele precisa ser tratado
		if(stat == 4){
			if(this->status[dia-1]==0){
				this->nvalid++;
			}
			if(val == UNDEF){
				//ultimo dia do mês e indefinido é um erro da estação
				if(dia == this->maxd){
					for (int i = (dia-1); i > 0 && (this->get_stat(i) == 4); i--){
						// para todos os valores 0 com status 4 anteriores

						if(this->get_dia(i) != 0){
							break;
						}
						// Valores 0 colocados anteriormente precisam ser removidos
						this->status[i-1] = 4;
						this->dias[i-1] = UNDEF;
					}
				}
				// acumulados com valor undef receberão 0 para entrar na conta de total mensal
				else{
					this->status[dia-1] = 4;
					this->dias[dia-1] = 0;
				}
			}
			else{
				this->status[dia-1] = 4;
				this->dias[dia-1] = val;

				for (int i = (dia-1); (i > 0) && (this->get_stat(i) == 0); i--){
					// Status 0 colocados antes de 4 precisam receber valor 0 para somar corretamente o mes
					if(this->get_dia(i) != UNDEF){
						break;
					}
					this->status[i-1] = 0;
					this->dias[i-1] = 0;
				}
			}
		}
		//Se o valor não for válido, então considera como UNDEF
		else if((val == UNDEF) || (val < 0)){

			if(this->status[dia-1] != 0) this->nvalid--;

			this->status[dia-1] = 0;
			this->dias[dia-1] = UNDEF;
		}
		else {
			if(this->status[dia-1]==0){
				this->nvalid++;
			}

			if(stat >= 0 && stat <= 4){
				this->status[dia-1]=stat;
			}
			else{
				this->status[dia-1]=1;
			}
			this->dias[dia-1]=val;
		}
	}
	return(0);
}

std::string class_mes::print( std::string cod ){
	std::ostringstream buf;
	if(cod.size()==8)buf<<"0"<<cod;
	else buf<<"000000000";
	buf<<std::setw(4)<<this->ano;
	buf<<std::setfill('0')<<std::setw(4)<<this->mes;
	buf<<"  777.7  777.7 0000000000000000 "<<std::setfill(' ');
	buf.precision(1);
	for(int j=1;j<=31;j++)buf<<" "<<std::setw(5)<<std::fixed<<this->get_dia(j);
	return(buf.str());
}

bool class_mes::operator==( class_mes comp ){
	for(int i=1;i<=31;i++)if(this->get_dia(i)!=comp.get_dia(i))return(false);
	return(true);
}

//-----------------------------------------------------------------------------

class_estacao::class_estacao( void ){
	this->cod.clear();
	this->meses.clear();
}

class_estacao::class_estacao( std::string cod ){
	this->cod=cod;
	this->meses.clear();
}

std::string class_estacao::get_cod( void ){
	return(this->cod);
}

int class_estacao::get_anoi( void ){
	if(this->meses.size()==0)return(0);
	return(this->meses.front().get_ano());
}

int class_estacao::get_anof( void ){
	if(this->meses.size()==0)return(0);
	return(this->meses.back().get_ano());
}
uint class_estacao::get_nmeses( void ){
	return(this->meses.size());
}
uint class_estacao::size( void ){
	return(this->meses.size());
}

class_mes class_estacao::get_mes( uint indc ){
	return(this->meses[indc]);
}
class_mes class_estacao::get_mes( int ano, int mes ){
	uint i=0;
	while(i<this->meses.size()&&this->meses[i].get_ano()<ano)i++;
	while(i<this->meses.size()&&this->meses[i].get_ano()==ano&&this->meses[i].get_mes()<mes)i++;
	if(i<this->meses.size()&&this->meses[i].get_ano()==ano&&this->meses[i].get_mes()==mes)return(this->meses[i]);
	class_mes vazio(mes,ano);
	return(vazio);
}

float class_estacao::get_dia( int ano, int mes, int dia ){
	uint i=0;
	while(i<this->meses.size()&&this->meses[i].get_ano()<ano)i++;
	while(i<this->meses.size()&&this->meses[i].get_ano()==ano&&this->meses[i].get_mes()<mes)i++;
	if(i<this->meses.size()&&this->meses[i].get_ano()==ano&&this->meses[i].get_mes()==mes)return(this->meses[i].get_dia(dia));
	return(UNDEF);
}

// bool class_estacao::has_accumulated(int ano, int mes){
// 	class_mes mes_dado = this->get_mes(ano,mes);

// 	for (size_t i = 1; i < 31; i++){
// 		//Se há algum dia naquele mês com status 4
// 		if(mes_dado.get_stat(i) == 4){
// 			return true;
// 		}
// 	}
// 	return false;
// }
/*
float class_estacao::get_dia( int ano, int mes, int dia ){
	if(this->ano_exist(ano))return(this->anos[this->indice(ano)].get_dia(mes,dia));
	return(UNDEF);
}

bool class_estacao::ano_exist( int ano ){
	if( ano >= this->anoi && ano <= this->anof && ano > 0 ) return(true);
	return(false);
}
*/
bool class_estacao::mes_exist( int ano, int mes ){
	uint i=0;
	while(i<this->meses.size()&&this->meses[i].get_ano()<ano)i++;
	while(i<this->meses.size()&&this->meses[i].get_ano()==ano&&this->meses[i].get_mes()<mes)i++;
	if(i<this->meses.size()&&this->meses[i].get_ano()==ano&&this->meses[i].get_mes()==mes)return(true);
	return(false);
}
/*
int class_estacao::add_ano( int ano ){
	if(this->ano_exist(ano))return(-1);
	if(this->anoi==0){
		this->anoi=this->anof=ano;
		class_ano novo(ano);
		anos.push_back(novo);
	} else {
		if(ano<this->anoi){
			for(int i=ano;i<this->anoi;i++){
				class_ano novo(i);
				anos.push_back(novo);
			}
			this->anoi=ano;
		} else {
			for(int i=this->anof+1;i<=ano;i++){
				class_ano novo(i);
				anos.push_back(novo);
			}
			this->anof=ano;
		}
	}
	return(0);
}
*/
int class_estacao::add_mes( class_mes novo_mes ){
	int ret=0;
	if(novo_mes.get_nvalid()<1)return(-1);
	if(this->meses.size()>0){
		vc_mes::iterator it = this->meses.end();
		while( it != this->meses.begin() && novo_mes.get_ano() < (*(it-1)).get_ano() )it--;
		while( it != this->meses.begin() && novo_mes.get_ano() == (*(it-1)).get_ano() && novo_mes.get_mes() <= (*(it-1)).get_mes())it--;

		//Verificação de datas repetidas na mesma estação
		if( it != this->meses.end() && novo_mes.get_ano() == (*it).get_ano() && novo_mes.get_mes() == (*it).get_mes() ){

			//bool escreve = false; //alteração

			for(int i=1;i<=31;i++){

				float valA = (*it).get_dia(i);
				float valB = novo_mes.get_dia(i);

				//Valor bruto (valA) sempre será lido antes do valor consistido (valB)

				//Diz na saída quais valores brutos foram trocados
				if( valA == UNDEF && valB != UNDEF){
					(*it).add_dia(i,valB);
					//std::cout<<"- "<<this->get_cod()<<" ("<<i<<"/"<<novo_mes.get_mes()<<"/"<<novo_mes.get_ano()<<")";
					//std::cout<<": valor UNDEF trocado por: "<<valB<<std::endl;
				}
				// Se há valores diferentes, escreve na saída quais meses tem diferenças
				else if(valB != UNDEF && (abs(valA - valB) >= 1)){
					//escreve = true;
					ret++;
				}
			}
			// if(escreve){
			// 	std::cout<<"__________________________\n";
			// 	std::cout<<this->get_cod()<<"\n"; //alteração
			// 	std::cout<< (*it).get_mes() <<"/"<<(*it).get_ano()<<"\n";
			// 	std::cout<<"__________________________\n";
			// }
		} else this->meses.insert(it,novo_mes);
	} else this->meses.push_back(novo_mes);
	return(ret);
}

int class_estacao::add_dia( int ano, int mes, int dia, float val ){
	uint i=0;
	while(i<this->meses.size()&&this->meses[i].get_ano()<ano)i++;
	while(i<this->meses.size()&&this->meses[i].get_ano()==ano&&this->meses[i].get_mes()<mes)i++;
	if(i<this->meses.size()&&this->meses[i].get_ano()==ano&&this->meses[i].get_mes()==mes)return(this->meses[i].add_dia(dia,val));
	return(-1);
}

int class_estacao::save_dia( void ){
	return( this->save_dia( "." ) );
}
int class_estacao::save_dia( std::string dirname ){
	if(this->meses.size()==0)return(-1);
	std::string fname = dirname+"/"+this->cod+".DIA";
//	std::string fname = mkfileuniq( dirname+"/"+this->cod, ".DIA" );
	if(FileExists(fname)){
		int tst = this->open_dia(fname);
		if(tst>0){
			std::string cmd = "mv " + fname + " " + mkfileuniq( dirname+"/"+this->cod, ".DIA" );
			std::cout<<cmd<<std::endl;
			std::cout<<"ndifs: "<<tst<<std::endl;
			if(system(cmd.c_str())!=0)return(-2);
		}
	}
//	std::cout<<fname<<std::endl;
	std::ofstream fout( fname.c_str(), std::ios_base::trunc );
	int ano_ant = this->meses[0].get_ano();
	int mes_ant = this->meses[0].get_mes()-1;

	for(uint i=0;i<this->meses.size();i++){
		bool preenche = true;
		do {
			mes_ant++;
			if( mes_ant > 12 ){ mes_ant=1, ano_ant++; }
			if( mes_ant == this->meses[i].get_mes() && ano_ant == this->meses[i].get_ano() )preenche = false;

			else {
				class_mes mes_undef(mes_ant,ano_ant);
				fout<<mes_undef.print(this->cod)<<std::endl;
			}
		} while(preenche);
		fout<<this->meses[i].print(this->cod)<<std::endl;
	}
	fout.close();
	return(0);
}
int class_estacao::save_mes( std::string dirname ){

	if(this->meses.size()==0)return(-1);

	std::string fname = dirname+"/"+this->cod+".MES";
	if(FileExists(fname)){
		int tst = this->open_dia(fname);
		if(tst>0){
			std::string cmd = "mv " + fname + " " + mkfileuniq( dirname+"/"+this->cod, ".MES" );
			std::cout<<cmd<<std::endl;
			std::cout<<"ndifs: "<<tst<<std::endl;
			if(system(cmd.c_str())!=0)return(-2);
		}
	}
	std::ofstream fout( fname.c_str(), std::ios_base::trunc );

	uint ano_ini = this->meses.front().get_ano();
	uint ano_fim = this->meses.back().get_ano();
	for(uint ano=ano_ini;ano<=ano_fim;ano++){
		fout<<" "<<this->cod<<ano<<"                               ";
		fout.precision(1);

		for(uint mes=1;mes<=12;mes++){
			float print_val = 0;
			float total = this->get_mes(ano,mes).get_val();
			float total_txt = this->get_mes(ano,mes).get_val_txt();

			if( total_txt > 0 && total_txt != UNDEF && (total_txt - total > EPSILON)){
				printf("+ %s: %02d/%d", this->get_cod().c_str(),mes,ano);
				printf("+ Soma dos valores mensais (%.2f) != total mensal (%.2f)\n", total, total_txt);
			}

			// Se há mais que MAX_UNDEF valores indefinidos, o mês será considerado indefinido
			if(this->get_mes(ano,mes).get_undefs() > MAX_UNDEF){
				print_val = UNDEF;
			}
			// Calcula o total mensal
			else{
				print_val = total;

				// Sinaliza meses com valores acumulados para análise posterior
				// if(this->has_accumulated(ano,mes)){
				// 	printf("# Acumulado:%s:%02d:%02d:%6.2f\n",this->cod.c_str(),mes,ano,print_val);
				// }
			}

			fout<<" "<<std::setw(6)<<std::fixed<<print_val;
		}
		fout<<std::endl;
	}
	fout.close();
	return(0);
}
int class_estacao::open_dia( std::string fname ){
	if(fname.size()>0){
		this->cod=basename(fname);
		std::ifstream fin( fname.c_str() );
		int MAX_LENGTH = 255;
		char line[MAX_LENGTH];
		int ret = 0;
		while( fin.getline(line, MAX_LENGTH) ) {
			std::string buf = line;
			class_mes novo_mes(atoi(buf.substr(15,2).c_str()),atoi(buf.substr(9,4).c_str()));
			for(int dia=1;dia<=31;dia++)novo_mes.add_dia(dia,atof(buf.substr(49+(dia-1)*6,6).c_str()));
			if(novo_mes.get_nvalid()>0) ret += this->add_mes(novo_mes);
		}
		fin.close();
		return(ret);
	}
	return(-1);
}

int class_estacao::open_hw( std::string fname ){
	if(fname.size()>0){
		std::ifstream fin( fname.c_str() );
		std::string buf;
		float total_txt = 0;

		this->cod=basename(fname);
		while( getline(fin,buf) ) {
			buf+=";";
			if(buf.size()>2&&buf[0]!='/'){
				int consist=0,ano=0,mes=0;
				float dias[31];
				int vstat[31];
				for(int wrd=0;wrd<=74;wrd++){
					std::string::size_type idx = buf.find_first_of(';');
					std::string cmp = buf.substr(0,idx);
					buf=buf.substr(idx+1);
					if(wrd == 1){				// NivelConsistencia: 1 = Bruto, 2 = Consistido
						consist=atoi(cmp.c_str());
					} else if(wrd == 2){		// Data
						std::string date = cmp.substr(cmp.find_first_of("/")+1);
						mes = atoi(date.substr(0,cmp.find_first_of("/")).c_str());
						ano = atoi(date.substr(cmp.find_first_of("/")+1).c_str());
					} else if(wrd == 5){ //total mensal
						if(cmp.find_first_of(',')!=std::string::npos)cmp[cmp.find_first_of(",")]='.';
						if(cmp.size()>0) total_txt = atof(cmp.c_str());
						else total_txt = UNDEF;
					} else if(wrd >= 13 && wrd <=43){	// Chuva
						if(cmp.find_first_of(',')!=std::string::npos)cmp[cmp.find_first_of(",")]='.';
						if(cmp.size()>0) dias[wrd-13]=atof(cmp.c_str());
						else dias[wrd-13]=UNDEF;
					} else if(wrd >= 44 && wrd <=74){	// ChuvaStatus: 0 = Branco, 1 = Real, 2 = Estimado, 3 = Duvidoso, 4 = Acumulado
						if(cmp.size()>0){
							//std::cout<<"STATUS: "<<cmp.c_str()<<std::endl;
							vstat[wrd-44]=atoi(cmp.c_str());
						}
						else {
							if(dias[wrd-44]==UNDEF)vstat[wrd-44]=0;
							else vstat[wrd-44]=1;
						}
					}
				}
				if(consist==1){
					class_mes novo_mes(mes,ano);
					for(int i=1;i<=31;i++) novo_mes.add_dia(i,dias[i-1],vstat[i-1]);
					novo_mes.set_val_txt(total_txt);
					if(novo_mes.get_nvalid()>0)this->add_mes(novo_mes);
				}
			}
		}
		fin.close();
	}
	return(-1);
}
/*
int class_estacao::indice( int ano ){
	for(uint i=0;i<this->anos.size();i++) if(this->anos[i].get_ano()==ano) return( i );
	return(-1);
}*/
//-----------------------------------------------------------------------------

bool class_estacoes::exist( std::string cod ){
	for(uint i=0;i<this->bd.size();i++) if(this->bd[i].get_cod()==cod) return(true);
	return(false);
}

int class_estacoes::indice( std::string cod ){
	for(uint i=0;i<this->bd.size();i++) if(this->bd[i].get_cod()==cod) return(i);
	return(-1);
}

int class_estacoes::create( std::string cod ){
	if( this->exist(cod) ) return(-1);
	class_estacao nova(cod);
	vc_estacao::iterator it = this->bd.end();
	while( it != this->bd.begin() && nova.get_cod() < (*(it-1)).get_cod() )it--;
	this->bd.insert(it,nova);
	return(0);
}

int class_estacoes::save_all( void ){
	for(uint i=0;i<this->bd.size();i++) if(this->bd[i].save_dia()!=0) return(-1);
	return(0);
}

int class_estacoes::get_mdb( std::string fname ){
	std::string tmpname = mkfileuniq("/tmp/estacao",".tmp");
	std::string cmd = "mdb-export -H -D \"%Y00%m\" "+fname+" Chuvas > "+tmpname;
	system(cmd.c_str());
	std::ifstream fin( tmpname.c_str() );

	std::cout<<"COMANDO: "<<cmd<<std::endl;

	std::string buf,cmp,codest,date;
	int consist=0,ano=0,mes=0;
	float dias[31];
	int vstat[31];
	while(fin.good()){
		fin >> buf;
		if(fin.good()){
			buf+=",";
			for(int wrd=0;wrd<80;wrd++){
				std::string::size_type idx = buf.find_first_of(",");
				cmp=buf.substr(0,idx);
				buf=buf.substr(idx+1);
				if(wrd == 5){						// EstacaoCodigo
					std::ostringstream tmp;
					tmp << std::right << std::setw( 8 ) << std::setfill( '0' ) << cmp;
					codest = tmp.str();
				} else if(wrd == 6){				// NivelConsistencia: 1 = Bruto, 2 = Consistido
					consist=atoi(cmp.c_str());
				} else if(wrd == 7){				// Data
					date=cmp.substr(1,8);
					ano=atoi(cmp.substr(1,4).c_str());
					mes=atoi(cmp.substr(7,2).c_str());
				} else if(wrd >= 18 && wrd <=48){	// Chuva
					if(cmp.size()>0) dias[wrd-18]=atof(cmp.c_str());
					else dias[wrd-18]=UNDEF;
				} else if(wrd >= 49 && wrd <=79){	// ChuvaStatus: 0 = Branco, 1 = Real, 2 = Estimado, 3 = Duvidoso, 4 = Acumulado
					if(cmp.size()>0) vstat[wrd-49]=atoi(cmp.c_str());
					else {
						if(dias[wrd-49]==UNDEF)vstat[wrd-49]=0;
						else vstat[wrd-49]=1;
					}
				}
			}
			if(consist==1){
				class_mes novo_mes(mes,ano);
				for(int i=1;i<=31;i++)novo_mes.add_dia(i,dias[i-1],vstat[i-1]);
				if(novo_mes.get_nvalid()>0){
					if( ! this->exist(codest) ) this->create(codest);
					this->bd[this->indice(codest)].add_mes(novo_mes);
				}
			}
		}
	}
	fin.close();
	cmd = "rm -f "+tmpname;
	system(cmd.c_str());
	return(0);
}
int class_estacoes::get_ldb( std::string fname ){/*
	std::ifstream fin( fname.c_str(), std::ios::in | std::ios::binary );
	if(!fin){
		std::cout<<"open: "<<fname<<" fail."<<std::endl;
		return(1);
	} else {
		std::ifstream::pos_type size = fin.tellg();
		char *p;
		char *memblock = new char [size];
		fin.seekg (0, std::ios::beg);
		fin.read (memblock, size);
		fin.close();
		p=memblock;

		uint dbsize;
		sscanf(p, sizeof(uint) );
		for(uint i=0;i<dbsize;i++){
			char tcod[8];
			uint estsize;
			fin.read( (char*)(&tcod), sizeof(char)*8 );
			fin.read( (char*)(&estsize), sizeof(uint) );
			std::string scod(tcod,8);
			for(uint k=0;k<estsize;k++){
				int ano, mes;
				float val;
				fin.read( (char*)(&ano), sizeof(int) );
				fin.read( (char*)(&mes), sizeof(int) );
				fin.read( (char*)(&val), sizeof(float) );
				class_mes mes_aux(mes,ano,val);
				for(int d=1;d<ndias(mes,ano);d++){
					float vald;
					int stat;
					fin.read( (char*)(&vald), sizeof(float) );
					fin.read( (char*)(&stat), sizeof(int) );
					mes_aux.add_dia(d,vald,stat);
				}
				if( ! this->exist(scod) ) this->create(scod);
				this->bd[this->indice(scod)].add_mes(mes_aux);
			}
		}
		delete[] memblock;
		std::cout<<"read: "<<dbsize<<" stations."<<std::endl;
	}*/
	return(0);
}/*
int class_estacoes::get_ldb( std::string fname ){
	std::ifstream fin( fname.c_str(), std::ios::in | std::ios::binary );
	if(!fin){
		std::cout<<"open: "<<fname<<" fail."<<std::endl;
		return(1);
	} else {
		uint dbsize;
		fin.read((char *)(&dbsize), sizeof(uint) );
		for(uint i=0;i<dbsize;i++){
			char tcod[8];
			uint estsize;
			fin.read( (char*)(&tcod), sizeof(char)*8 );
			fin.read( (char*)(&estsize), sizeof(uint) );
			std::string scod(tcod,8);
			for(uint k=0;k<estsize;k++){
				int ano, mes;
				float val;
				fin.read( (char*)(&ano), sizeof(int) );
				fin.read( (char*)(&mes), sizeof(int) );
				fin.read( (char*)(&val), sizeof(float) );
				class_mes mes_aux(mes,ano,val);
				for(int d=1;d<ndias(mes,ano);d++){
					float vald;
					int stat;
					fin.read( (char*)(&vald), sizeof(float) );
					fin.read( (char*)(&stat), sizeof(int) );
					mes_aux.add_dia(d,vald,stat);
				}
				if( ! this->exist(scod) ) this->create(scod);
				this->bd[this->indice(scod)].add_mes(mes_aux);
			}
		}
		fin.close();
		std::cout<<"read: "<<dbsize<<" stations."<<std::endl;
	}
	return(0);
}*/
int class_estacoes::save_ldb( std::string fname ){
	std::ofstream fout( fname.c_str(), std::ios_base::trunc|std::ios::binary  );
	if( !fout ) return(-1);
	uint dbsize=this->bd.size();
	fout.write( (char*)(&dbsize), sizeof(uint) );
	for(uint i=0;i<dbsize;i++){
		fout.write( this->bd[i].get_cod().c_str(), sizeof(char)*8 );
		uint estsize = this->bd[i].get_nmeses();
		fout.write( (char*)(&estsize), sizeof(uint) );
		for(uint k=0;k<estsize;k++){
			class_mes mes_aux = this->bd[i].get_mes(k);
			int ano = mes_aux.get_ano();
			int mes = mes_aux.get_mes();
			float val = mes_aux.get_val();
			fout.write( (char*)(&ano), sizeof(int) );
			fout.write( (char*)(&mes), sizeof(int) );
			fout.write( (char*)(&val), sizeof(float) );
			for(int d=1;d<ndias(mes,ano);d++){
				float vald = mes_aux.get_dia(d);
				int stat = mes_aux.get_stat(d);
				fout.write( (char*)(&vald), sizeof(float) );
				fout.write( (char*)(&stat), sizeof(int) );
			}
		}
	}
	fout.close();
	std::cout<<"write: "<<dbsize<<" stations."<<std::endl;
	return(0);
}
/*
bool class_estacoes::ano_exist( std::string cod,int ano ){
	if( ! this->exist(cod) ) return(false);
	int est = this->indice(cod);
	return( this->bd[est].ano_exist(ano) );
}

bool class_estacoes::mes_exist( std::string cod, int ano, int mes ){
	if( ! this->exist(cod) ) return(false);
	int est = this->indice(cod);
	return( this->bd[est].mes_exist(ano,mes) );
}

int class_estacoes::add_ano( std::string cod, int ano ){
	if( ! this->exist(cod) ) this->create(cod);
	return( this->bd[this->indice(cod)].add_ano(ano) );
}

int class_estacoes::add_mes( std::string cod, int ano, int mes ){
	if( ! this->exist(cod) ) this->create(cod);
	return( this->bd[this->indice(cod)].add_mes(ano,mes) );
}

int class_estacoes::add_dia( std::string cod, int ano, int mes, int dia, float val ){
	if( ! this->exist(cod) ) this->create(cod);
	return( this->bd[this->indice(cod)].add_dia(ano,mes,dia,val) );
}
*/
/*
int class_estacoes::add_est( class_estacao nova ){
	if( this->exist(nova.get_cod()) ) {
		std::cout<<"ESTACAO REPETIDA: "<<nova.get_cod()<<std::endl;
		return (-1);
	}

	vc_estacao::iterator it = this->bd.end();
	while( it != this->bd.begin() && nova.get_cod() < (*(it-1)).get_cod() )it--;
	this->bd.insert(it,nova);
	return(0);
}*/
int class_estacoes::add_est( class_estacao nova ){
	this->bd.push_back(nova);
	return(0);
}
uint class_estacoes::size( void ){
	return( this->bd.size() );
}
class_estacao	class_estacoes::at( uint i){
	return(this->bd[i]);
}
class_estacao	class_estacoes::operator[]( uint i){
	return(this->bd[i]);
}
//-----------------------------------------------------------------------------
