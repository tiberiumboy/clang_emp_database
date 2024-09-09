TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default test
	
default: $(TARGET)

test:
	./$(TARGET) -f ./mynewdb.db -n
	./$(TARGET) -f ./mynewdb.db -a "John Doe,123 Sheep St.,120"
	./$(TARGET) -f ./mynewdb.db -a "Kevin Hart,123 New Hemisphere Dr.,69"
	./$(TARGET) -f ./mynewdb.db -a "Snoop Dawg,1000 New Orlean Dr.,420"
	./$(TARGET) -f ./mynewdb.db -l
	./$(TARGET) -f ./mynewdb.db -r "John Doe"
	./$(TARGET) -f ./mynewdb.db -u "Kevin Hart,123 Sheep St.,360"
	./$(TARGET) -f ./mynewdb.db -l

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET): $(OBJ)
	gcc -o $@ $? -g

obj/%.o : src/%.c
	gcc -ggdb -c $< -o $@ -Iinclude
