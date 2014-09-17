obj=main.o
obj1=main.c main.h

readfile:$(obj)
	cc -o readfile $(obj)

main.o: main.h
	cc -c main.c
	@echo  "done!"


.PHONY:clean
clean:
	-rm -f readfile *.o