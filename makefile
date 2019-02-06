# PROGRAM:   Simulated Shell
# PROGRAMMER: Jason Tokarski

ASSIGN=SimulatedShell.c

# Rule to link object code files to create executable file
$(ASSIGN): $(ASSIGN).o
	gcc -Wall -o z1805312_A3 $(ASSIGN).o
   
# Rules to compile source code files to object code
$(ASSIGN).o: $(ASSIGN).c
	gcc -g -Wall -c $(ASSIGN).c
         
# Pseudo-target to remove object code and executable files
clean:
	-rm *.o $(ASSIGN)

