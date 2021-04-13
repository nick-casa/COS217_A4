all: ftGood

ftGood: dynarray.o node.o ft.o ft_client.o
	gcc217 -g $^ -o $@

dynarray.o: dynarray.c dynarray.h
	gcc217 -g -c $<

ft_client.o: ft_client.c ft.h a4def.h
	gcc217 -g -c $<


ft.o: ft.c  dynarray.h ft.h a4def.h node.h
	gcc217 -g -c $<

node.o: node.c dynarray.h node.h a4def.h
	gcc217 -g -c $<
