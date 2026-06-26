all: chat
chat: peer_discovery.c main.c
	gcc -Wall -Werror main.c peer_discovery.c tui.c chat.c -o ./bin/chat -lncurses
run: chat
	./bin/chat
