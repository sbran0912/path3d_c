CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c17 -Ilib3d -I.
LDFLAGS = -lglfw -lGL -lm

LIB3D_SRCS = lib3d/glcore.c lib3d/lib3d_util.c lib3d/lib3d_mesh.c lib3d/lib3d.c
APP_SRCS = pathfinding.c main.c
SRCS = $(LIB3D_SRCS) $(APP_SRCS)
OBJDIR = obj
BINDIR = bin
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
TARGET = $(BINDIR)/path3d

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS) | $(BINDIR)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(OBJDIR)/lib3d/%.o: lib3d/%.c lib3d/lib3d.h lib3d/glcore.h | $(OBJDIR)/lib3d
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.c pathfinding.h lib3d/lib3d.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR) $(OBJDIR)/lib3d $(BINDIR):
	mkdir -p $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)