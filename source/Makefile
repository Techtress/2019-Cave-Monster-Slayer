CXX = g++

# Warnings frequently signal eventual errors:
CXXFLAGS=`sdl2-config --cflags` -g -W -Wall -Werror -std=c++14 -Weffc++ -Wextra -pedantic -O0 -I `sdl2-config --prefix`/include/ -I ./

LDFLAGS = `sdl2-config --libs` -lm -lexpat -lSDL2_ttf -lSDL2_image -lSDL2_mixer

OBJS = \
	entity/chunkexplosion.o \
	entity/aibehavior.o \
	entity/aicontroller.o \
	entity/playercontroller.o \
	entity/animation.o \
	entity/animationset.o \
	entity/animationstate.o \
	entity/hitbox.o \
	entity/actorphysics.o \
	entity/actorphysicsmodel.o \
	entity/actor.o \
	entity/actormodel.o \
	entityfactory.o \
	backdrop.o \
	debughud.o \
	lightmanager.o \
	canvas.o \
	physicsmanager.o \
	eventmanager.o \
	gamemanager.o \
	gamestate.o \
	appstatemanager.o \
	xmltag.o \
	xmlparser.o \
	soundset.o \
	soundmanager.o \
	image.o \
	imagefactory.o \
	gameconfig.o \
	clock.o \
	iomod.o \
	rendercontext.o \
	viewport.o \
	engine.o

OBJS_EXEC = main.o

OBJS_EDITOR = \
	leveleditor/main.o \
	leveleditor/editorstate.o \
	leveleditor/testingstate.o

EXEC = run
EDITOR = editor

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXEC): $(OBJS) $(OBJS_EXEC)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(OBJS_EXEC) $(LDFLAGS)

$(EDITOR): $(OBJS) $(OBJS_EDITOR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(OBJS_EDITOR) $(LDFLAGS)

clean:
	rm -rf $(OBJS) $(OBJS_EXEC)
	rm -rf $(EXEC)
