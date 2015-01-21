UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CXXLIBS += -framework OpenCL
	CXXFLAGS+= -Wno-deprecated-declarations
else
	CXXLIBS += -lOpenCL 
endif

SOURCE_CPP_FILES = main.cpp

all:	c++_vecadd

c++_vecadd: $(SOURCE_CPP_FILES)
	$(CXX) $(SOURCE_CPP_FILES) $(CXXFLAGS) -o c++_vecadd $(CXXLIBS)

clean:
	rm c++_vecadd

