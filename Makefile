all: producer consumer

producer: producer.c message.h
	gcc producer.c -o producer

consumer: consumer.c message.h
	gcc consumer.c -o consumer

clean:
	rm -rf producer consumer
