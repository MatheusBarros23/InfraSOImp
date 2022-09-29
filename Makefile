# This is a makefile for Shell3.c ... .... ... (add outros se tiver)
# Made by Matheus Pereira do Rego Barros (mprb) due to first implementation for Infra SW discipline

	$(CC) = gcc

shell:
	$(CC) shell.c -o shell

functions.h:
	$(CC) functions.c -o functions

clean:
		rm shell

#PERGUNTAR SE PRECISA EXCLUIR O RESTO!! OU SO O SHELL EM QUESTAO!
