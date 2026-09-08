collision_finder: main.c pdf_io.c toy_hash.c
	gcc -Wall -O2 -o collision_finder main.c pdf_io.c toy_hash.c

clean:
	rm -f collision_finder test_output.pdf