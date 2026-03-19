

LDLIBS = -lSDL3 -lSDL3_image -lSDL3_ttf

obj_files = \
	GUI.o \
	LoadData.o \
	Main.o \
	ProgressStory.o \
	UI_element_creation.o \
	OpenFiles.o \
	LoadAssets.o \
	ParseStory.o

all: text_rpg

%.o: %.cxx
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -std=c++20 -c $< -o $@

text_rpg: $(obj_files)
	$(CXX) $(LDLIBS) $? -o text_rpg

.PHONY: clean
clean:
	rm -f *.o
