# Description of files in this directory.

The files here are starter files for UVic SENG 265, Fall 2021
Assignment #3.  The files are as follows:

* ```makefile```: Needed to build the assignment. In order to
construct the ```process_cal3``` executable, type either ```make``` or
```make process_cal3```.

* ```process_cal3.c```: The C file containing the ```main()``` function.
(There must only exist one ```main``` in any program.)  This should be
suitably modified to complete the assignment.

* ```emalloc.[ch]```: Source code and header file for the
```emalloc``` function described in lecture. This is kept in its own
source-code file as it can be used independently in both
```process_cal3.c``` and ```listy.c```.

* ```ics.h```: The definition of the ```struct event_t``` type.

* ```listy.[ch]```: Linked-list routines based on the lectures. The
routines here may be added to or modified. Regardless, however,
students are responsible for any segmentation faults that occur as a
result of using this code.
