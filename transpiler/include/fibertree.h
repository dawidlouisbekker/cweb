#include <stdio.h>

typedef struct EventHandler
{
    const char* action;
    const char* function;
};


typedef struct HTML_ELEMENT {
    const char* id;
    const char* type;
    const char* styles;
    EventHandler* handlers;
};

typedef struct Component
{
    const char* name;
    Component* next;
    HTML_ELEMENT* children[];    
};
