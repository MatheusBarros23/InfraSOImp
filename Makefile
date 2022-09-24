# This is a makefile for Shell3.c ... .... ... (add outros se tiver)
# Made by Matheus Pereira do Rego Barros (mprb) due to first implementation for Infra SW discipline

	$(CC) = gcc

shell:
	$(CC) shell3.c -lpthread -o shell

Clean:
		rm *.o shell
