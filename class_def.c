/*
    Handle a class definition.
*/

#include "logging.h"
#include "errors.h"


/*
    When this is entered, a name has been encountered that gived the name of 
    the class is being defined. Before any other actions are taken the name 
    needs be retreived and the context pushed.

    The class definition has the form of
    name(inherit1, inherit2, ...) {class body}

    The inherited classes must already have a definition. The class body 
    consists of variable definitions and function definitions.
*/
void do_class(void)
{
    ENTER();
    RET();
}

// classes can have a scope operator? 