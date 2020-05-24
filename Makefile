all: producer consumer

producer: producer.c message.h
	gcc -D_REENTRANT producer.c -o producer -lpthread

consumer: consumer.c message.h
	gcc consumer.c -o consumer

clean:
	rm -rf producer consumer
