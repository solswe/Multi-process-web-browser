CC=gcc
CFLAGS= -g -Wall `pkg-config --cflags gtk+-3.0 webkit2gtk-4.0`
LDFLAGS+=`pkg-config --libs gtk+-3.0 webkit2gtk-4.0`

LIB=/usr/lib/
SOURCES=wrapper.c wrapper.h 
OBJ1=browser
OBJ2=render

all:  $(SOURCES) $(OBJ1) $(OBJ2)

browser: $(SOURCES) browser.c
	$(CC) $(SOURCES) browser.c -L ${LIB} $(CFLAGS) $(LDFLAGS) -o browser

test: clean all
	clear
	./browser blacklist

solution:
	echo === RUNNING SOLUTION CODE ===
	./browser_solution blacklist
	echo === SOLUTION CODE COMPLETE ===

submission: clean
	@echo "Creating Tar Submission..."
	@read -p "Please enter your group number: " grou_num; \
	tar_name="group_"$$grou_num"_p1_submission.tar.gz";\
	rm -f $$tar_name;tar -czvf $$tar_name browser.c Makefile render wrapper.c wrapper.h blacklist;\
	echo "Submission Created: " $$tar_name

clean:
	rm -rf $(OBJ1)

