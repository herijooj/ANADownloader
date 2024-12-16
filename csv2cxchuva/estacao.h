#ifndef ESTACAO_H_
#define ESTACAO_H_

#include <string>
#include "typesdef.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "utils.h"


//Alterado por Lucas Nogueira
#define UNDEF 		777.7f	// Código de valor faltante do laboratótio
#define UNSET 		-1.0f	// Usarei para valores ainda não definidos
#define MAX_UNDEF	2		// Quantidade máximas dias UNDEF para o total mensal não ser considerado UNDEF
#define EPSILON		1.0f	//Erro máximo permitido ao comparar dois valores float

class class_mes{
	public:
		//Construtores
					class_mes		( int, int );
					class_mes		( int, int, float );

		int 		get_nvalid 		( void );

		//Retorna a quantidade de dias indefinidos
		int			get_undefs		( void );

		//Retorna a quantidade de dias com valores diferentes de 0 e UNDEF daquele mês
		int 		get_nonz 		( void );

		//Retorna o valor do dia com status diferente de 4 (acumulado)
		float		get_dia 		( int );

		//Retorna o valor do dia independente do status
		float		get_dia_val 	( int );

		//Retorna o valor total do mês, lido no arquivo txt da estação
		float 		get_val_txt		( void );
		void 		set_val_txt		( float );

		int 		get_stat 		( int );
		float		get_val 		( void );
		int 		get_mes 		( void );
		int 		get_ano 		( void );
		int 		add_dia 		( int, float, int = 1 );
		std::string print			( std::string );
		bool		operator==		( class_mes );

	private:
		int 		mes,ano,maxd,nvalid;
		float		valor, valor_txt;
		vfloat		dias;
		vint		status;
};

#ifndef VC_MES
#define VC_MES
typedef std::vector < class_mes > vc_mes;
#endif

class class_estacao{
	public:
					class_estacao	( void );
					class_estacao	( std::string );
		std::string	get_cod 		( void );
		int 		get_anoi		( void );
		int 		get_anof		( void );
		uint 		get_nmeses		( void );
		uint 		size			( void );
		class_mes	get_mes 		( uint );
		class_mes	get_mes 		( int, int );
		float 		get_dia			( int, int, int );

		//Retorna se há algum valor acumulado (status 4) no mês e ano passado
		bool		has_accumulated	(int, int);

/*		bool		ano_exist		( int ); */
		bool		mes_exist		( int, int );
/*		int 		add_ano 		( int );
*/		int 		add_mes 		( class_mes );
		int 		add_dia 		( int, int, int, float );
		int 		save_dia		( void );
		int 		save_dia		( std::string );
		int 		save_mes		( std::string );
		int 		open_dia		( std::string );
		int 		open_hw 		( std::string );
	private:
/*		int 		indice			( int );
*/		std::string	cod;
		vc_mes		meses;
};
#ifndef VC_EST
#define VC_EST
typedef std::vector < class_estacao > vc_estacao;
#endif

class class_estacoes{
	public:
		bool			exist			( std::string );
		bool			ano_exist		( std::string, int );
		bool			mes_exist		( std::string, int, int );
		int 			create			( std::string );
		int 			get_mdb			( std::string );
		int 			get_ldb			( std::string );
		int 			save_ldb		( std::string );
		int 			add_ano 		( std::string, int );
		int 			add_mes 		( std::string, int, int );
		int 			add_dia 		( std::string, int, int, int, float );
		int 			add_est 		( class_estacao );
		int 			save_all		( void );
		uint			size			( void );
		int 			indice			( std::string );
		class_estacao	at				( uint );
		class_estacao	operator[]		( uint );
	private:
		vc_estacao	bd;
};

#endif //ESTACAO_H_
