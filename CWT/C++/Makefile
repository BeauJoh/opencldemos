UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CXXLIBS += -framework OpenCL
else
	CXXLIBS += -lOpenCL -lm 
endif

SOURCE_CPP_FILES = main.cpp

all:	c++_cwt

c++_cwt: $(SOURCE_CPP_FILES)
	$(CXX) $(SOURCE_CPP_FILES) $(CXXFLAGS) -o c++_cwt $(CXXLIBS)

clean:
	rm c++_cwt

