
all: clean list.test.c run

list.test.c:
	gcc -std=c99 -pedantic -Wall -I./include -I./include/list ./src/list/list.c ./tests/list.test.c -o ./tests/list.test

.PHONY: clean run

clean:
	rm -f ./tests/list.test

run:
	@echo " "
	@echo "Running list.test:"
	./tests/list.test || true && rm -f ./tests/list.test
