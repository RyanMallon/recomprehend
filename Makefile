#
# This file is public domain
#
# Ryan Mallon, 2015
#
#

recomprehend_games	+=	game_tr.o		\
				game_cc.o		\
				game_oo.o		\
				game_tm.o

recomprehend_objects	:=	$(recomprehend_games)	\
				recomprehend.o		\
				game_data.o 		\
				dump_game_data.o	\
				opcode_map.o		\
				game.o			\
				dictionary.o		\
				strings.o		\
				file_buf.o		\
				image_data.o		\
				graphics.o		\
				util.o

recomprehend_prog	:= 	recomprehend

image_view_objects	:=	image_view.o		\
				graphics.o		\
				image_data.o		\
				file_buf.o		\
				util.o

image_view_prog		:=	image_view

progs			:=	$(recomprehend_prog)	\
				$(image_view_prog)

cflags	:= -g -Wall
lflags	:= -lSDL2

all: $(progs)

%.o: %.c
	@echo "  CC $@"
	@$(CC) $(cflags) $< -c -o $@

$(recomprehend_prog): $(recomprehend_objects)
	@echo "  LD $@"
	@$(CC) $(recomprehend_objects) $(lflags) -o $@

$(image_view_prog): $(image_view_objects)
	@echo "  LD $@"
	@$(CC) $(image_view_objects) $(lflags) -o $@

clean:
	@echo "  CLEAN"
	@rm -f *.o $(progs)