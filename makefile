CXX += -std=c++11 -stdlib=libc++ -O3

EXECUTABLE = buildSphere

SRC = lib
INCLUDE = include
OBJDIR = build

CFLAGS += -I$(INCLUDE) -Wimplicit-function-declaration -Wall -Wextra -pedantic

all: $(EXECUTABLE)

buildSphere: $(OBJDIR)/buildSphere.o
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS)$(OBJDIR)/buildSphere.o -o buildSphere

$(OBJDIR)/buildSphere.o: $(SRC)/buildSphere.c++
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) $< -o $(OBJDIR)/buildSphere.o

clean:
	rm -f $(OBJDIR)/*.o $(EXECUTABLE)
