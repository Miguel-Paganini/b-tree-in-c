CC= gcc
CFLAGS= -o

main:
	$(CC) $(CFLAGS) main main.c b-tree.c

clean:
	rm main