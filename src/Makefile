CC = gcc
CFLAGS = -fno-stack-protector -z execstack -Wall -I. -Iutil -Iatm -Ibank -Irouter -Idatalayers -Iencrypt
HEADERS = atm/atm.h bank/bank.h datalayers/commands.h datalayers/database.h datalayers/formats.h encrypt/encrypt.h ports.h router/router.h util/hash_table.h util/list.h encrypt/aes-256.h
UTILS = datalayers/commands.c datalayers/database.c util/hash_table.c util/list.c encrypt/encrypt.c encrypt/aes-256.c

all: bin bin/atm bin/bank bin/router bin/encrypt-test bin/init

bin:
	mkdir -p bin

bin/atm : atm/atm-main.c atm/atm.c ${HEADERS} ${UTILS}
	${CC} ${CFLAGS} -D _CP_ATM_MODE_ atm/atm.c atm/atm-main.c ${UTILS} -o bin/atm

bin/bank : bank/bank-main.c bank/bank.c ${HEADERS} ${UTILS}
	${CC} ${CFLAGS} bank/bank.c bank/bank-main.c ${UTILS} -o bin/bank

bin/router : router/router-main.c router/router.c ${HEADERS}
	${CC} ${CFLAGS} router/router.c router/router-main.c -o bin/router

test : util/list.c util/list_example.c util/hash_table.c util/hash_table_example.c
	${CC} ${CFLAGS} util/list.c util/list_example.c -o bin/list-test
	${CC} ${CFLAGS} util/list.c util/hash_table.c util/hash_table_example.c -o bin/hash-table-test

bin/encrypt-test : encrypt/encrypt-test.c ${HEADERS} ${UTILS}
	${CC} ${CFLAGS} -g encrypt/encrypt-test.c ${UTILS} -o bin/encrypt-test

bin/init : init.c
	${CC} ${CFLAGS} init.c -o bin/init

clean:
	cd bin && rm -f atm bank router init encrypt-test list-test hash-table-test
	rm -f *.card *.atm *.bank
