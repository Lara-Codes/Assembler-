# Assembler-
This is a C program to convert assembly code into an executable file in hexadecimal.

Depending on which operating system you use, you may need to use a different version of the LCC program to run the assembler (written by my Professor Anthony Dos Reis). a1test.a was also created by Professor Reis to test the a1.c program. See his book: C and C++ Under the Hood. 

To run, first enter the same repository as both the LCC, assembler, and assembly language programs. 
Compile a1.c: 

gcc -o <programName> a1.c 

Run a1.c and create executable file for assembly code in a1test.a (will generate an a1test.e file): 

On mac: 
./<programName> a1test.a 

To run a1test.e file: 
Drag the LCC file into your terminal and it will prompt you for your name. Type in your name. 
Then, simply enter the name of the program you'd like to run: 

a1test.e 

Alternatively, depending on the machine you're using, you can just type the command: 
lcc a1test.e 
instead of dragging the LCC program into the terminal/command line. 

This will create a .lst and .bst file. 
