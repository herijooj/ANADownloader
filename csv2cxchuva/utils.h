#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include "typesdef.h"
#include <sys/stat.h>

//Retorna a quantidade de dias dado um mês e um ano.
int 			ndias		( int, int );

std::string 	basename	( std::string );
vint			expand		( std::string );
bool			FileExists	( std::string );
std::string 	mkfileuniq	( std::string, std::string );

#endif //UTILS_H_
