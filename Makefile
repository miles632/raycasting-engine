CC = g++
CFLAGS = -std=c++23
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SOURCES = main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

EXECUTABLE = final

all: $(EXECUTABLE)

$(EXECUTABLE) : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXECUTABLE) $(LDFLAGS)
	chmod +x $(EXECUTABLE)

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)