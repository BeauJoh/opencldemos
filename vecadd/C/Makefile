UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CCLIBS += -framework OpenCL
else
	CCLIBS += -lOpenCL -lm 
	CFLAGS += -std=gnu99
endif

SOURCE_C_FILES = main.c

all:	c_vecadd

c_vecadd: $(SOURCE_C_FILES)
	$(CC) $(SOURCE_C_FILES) $(CFLAGS) -o c_vecadd $(CCLIBS)

clean:
	rm c_vecadd

