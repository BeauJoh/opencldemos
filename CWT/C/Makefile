UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CCLIBS += -framework OpenCL
else
	CCLIBS += -lOpenCL 
endif

SOURCE_C_FILES = main.c

all:	c_cwt

c_cwt: $(SOURCE_C_FILES)
	$(CC) $(SOURCE_C_FILES) -o c_cwt $(CCLIBS)

