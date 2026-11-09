# Paralleler Build mit gcc (der CMake/Ninja-Build nutzt clang).
CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c17 -Ilib3d
LDFLAGS = -lglfw -lGL -lm

SRCS = lib3d/glcore.c lib3d/lib3d_util.c lib3d/lib3d_mesh.c lib3d/lib3d.c \
       src/pathfinding.c src/main.c
OBJDIR = obj
BINDIR = bin
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
TARGET = $(BINDIR)/path3d

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJDIR) $(BINDIR)
