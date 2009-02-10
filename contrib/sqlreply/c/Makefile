
%.o: %.c
	@echo Making $<
	@$(CC) -c $*.c -o $*.o

all: michal

michal: michal.o
	@echo Linking
	@$(CC) michal.o -L/usr/lib/mysql -lmysqlclient -lz -o michal

clean:
	@rm michal michal.o
	