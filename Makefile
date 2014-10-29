CFLAGS := -std=gnu99

.PHONY : clean

24bmp32 : 24bmp32.o

clean:
	@rm -f 24bmp32 *.o

