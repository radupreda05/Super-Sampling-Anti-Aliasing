build:
	gcc -o antialiasing SSAA.c main.c -lpthread -Wall
  
clean:
	rm antialiasing
