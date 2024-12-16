#include "utils.h"

bool FileExists( std::string strFilename ){
	struct stat stFileInfo;
	if( stat(strFilename.c_str(),&stFileInfo) == 0 ) return(true);
	return(false);
}
int ndias( int mes, int ano ){

	//Verifica se é ano bissexto
	if(mes==2){
		if((ano%4 == 0) && ((ano%100 != 0) || (ano%400 == 0))) return(29);
		else return(28);
 	} 
	else {
		if((mes==4)||(mes==6)||(mes==9)||(mes==11)) {
			return(30);
		}
		else{
			return(31);
		}
	}
	return(-1);
}
std::string basename( std::string fname ){
	std::string::size_type idx;
	int ini;
	idx = fname.find_last_of("/");
	if(idx==std::string::npos)ini=0;
	else ini=idx+1;
	idx = fname.find_last_of(".");
	if(idx==std::string::npos)return(fname.substr(ini));
	else return(fname.substr(ini,idx-ini));
}
vint expand( std::string pattern ){
	vint res;
	do{
		std::string aux=pattern.substr(0,pattern.find_first_of(','));
		if(aux.find_first_of('-')!=std::string::npos){
			int ini = atoi( aux.substr(0,aux.find_first_of('-')).c_str() );
			int fim = atoi( aux.substr(aux.find_first_of('-')+1).c_str() );
			if(fim<ini){int tmp=ini;ini=fim;fim=tmp;}
			for(int i=ini;i<=fim;i++)res.push_back( i );
		} else res.push_back( atoi(aux.c_str()) );
		if(pattern.find_first_of(',')!=std::string::npos)pattern=pattern.substr(pattern.find_first_of(',')+1);
		else pattern.clear();
	}while(pattern.size()>0);
	return(res);
}
std::string mkfileuniq( std::string pfix, std::string sfix ){
	std::string ret = pfix + sfix;
	int i=1;
	while(FileExists(ret)){
		std::ostringstream buf;
		buf << pfix << "_" << i << sfix;
		ret = buf.str();
		i++;
	}
	return(ret);
}

