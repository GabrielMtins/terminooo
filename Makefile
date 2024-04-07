all:
	gcc src/*.c -std='c99' -Wall -Wextra -lcurses -o terminooo

clean:
	rm terminoo
