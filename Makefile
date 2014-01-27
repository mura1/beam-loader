TARGET := beam
SRCDIR := src/
OBJDIR := obj
INCDIR := -Iinclude
LIBDIR := -L.

CC := gcc
AR := ar

SRC := iff.c atom_table.c

OBJ := $(SRC:.c=.o)
LIB :=

CFLAGS:= -Wall -Wno-pointer-sign $(INCDIR) $(LIBDIR) $(LIB)

OBJLIST = $(addprefix $(OBJDIR)/,$(OBJ))

VPATH = $(SRCDIR) $(OBJDIR)


$(TARGET) : $(OBJLIST)
	$(AR) rcs $@.a $^
	$(CC) $(CFLAGS) $(LIBDIR) $(LIB) -o $@ $^

$(OBJDIR)/%.o : %.c
	mkdir -p $(OBJDIR) && $(CC) $(CFLAGS) -MMD -c -o $@ $<

-include $(OBJLIST:.o=.d)

clean:
	rm -rf $(TARGET) $(TARGET) $(OBJDIR) *.core *.a

.PHONY : all clean

