main: 
	gcc server.c -o server
	gcc client.c -o client
thread:
	gcc -DTHREAD -pthread server.c -o server
process:
	gcc server.c -o server
client:
	gcc client.c -o client
clean:
	rm server
	rm client
