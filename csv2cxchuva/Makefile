VERM = \e[01;31m
VERD = \e[01;32m
AMAR = \e[01;33m
AZUL = \e[01;34m
CIAN = \e[01;36m
NORM = \e[0m

CC	= g++
CFLAGS	= -Wall -O3 -I./ -std=c++17
LDFLAGS = -lstdc++fs
OBJs	= estacao_mes.o utils.o
BIN	= hw2dia

all: $(BIN)

$(BIN): hw2dia.cpp $(OBJs)
	@echo -e "$(AMAR)$^ $(VERM)=> $(VERD)$@$(NORM)"
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

estacao_mes.o: estacao_mes.cpp estacao.h typesdef.h utils.h
	@echo -e "$(AMAR)$< $(VERM)=> $(VERD)$@$(NORM)"
	@$(CC) $(CFLAGS) -c -o $@ $<

utils.o: utils.cpp utils.h typesdef.h
	@echo -e "$(AMAR)$< $(VERM)=> $(VERD)$@$(NORM)"
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo -e "$(VERM)$(OBJs) $(BIN)$(NORM)"
	@rm -f $(OBJs) $(BIN)
