//Lara Palombi  
#include <stdio.h>    // for I/O functions
#include <stdlib.h>   // for exit()
#include <string.h>   // for string functions
#include <ctype.h>    // for isspace(), tolower()
#include <time.h>     // for time functions
//also check for fp and sp 
FILE *infile, *outfile;
short pcoffset9, pcoffset11, imm5, imm9, offset6;
unsigned short symadd[500], macword, dr, sr, sr1, sr2, baser, trapvec, eopcode, wordvalue[500];
char outfilename[100], linesave[100], buf[100], *symbol[500], *p1, *p2, *cp,
     *mnemonic, *o1, *o2, *o3, *label, *temp; 
int stsize, linenum, rc, loc_ctr, num;
time_t timer;

// Case insensitive string compare
// Returns 0 if two strings are equal.
short int mystrcmpi(const char *p, const char *q) 
{
   char a, b;
   while (1) 
   {
      a = tolower(*p); b = tolower(*q);
      if (a != b) return a-b;
      if (a == '\0') return 0;
      p++; q++;
   }
}

// Case insensitive string compare
// Compares up to a maximum of n characters from each string.
// Returns 0 if characters compared are equal.
short int mystrncmpi(const char *p, const char *q, int n) 
{
   char a, b;
   int i;
   for (i = 1; i <= n; i++) 
   {
      a = tolower(*p); b = tolower(*q);
      if (a != b) return a-b;
      if (a == '\0') return 0;
      p++; q++;
   }
   return 0; 
}

void error(char *p)
{
   printf("%s\nLine number: %d\n Line: %s\n", p, linenum, linesave);

   // Code missing here:
   // Displays error message p points to, line number in linenum, and line in
   // linesave.
}
int isreg(char *p)
{   
    char* original = p; //Copying pointer to another variable so can reset later 
    char reg[2]; //Will store register in array of 2 
    int count = 0; 
    while(*p!='\0'){ //check to see if *p points to exactly 2 characters 
        count++; 
        p++; //incrementing pointer 
    }
    if(count!=2){ //can't be a reg if size!=2 
        return 0; 
    }
    p = original; //resetting pointer 
    reg[0] = *p; //placing 2 characters in array to store some value r0 - r8
    p++; 
    reg[1] = *p; 
    int num;
    sscanf(&reg[1], "%d", &num); //converting reg[1] to an integer
    if((reg[0] == 'f' && reg[1]=='p') || (reg[0] == 's' && reg[1]=='p') || (reg[0] == 'l' && reg[1]=='r')){
      return 1; 
    }
    if(reg[0]!='r' || num<0 || num>7){ //if first char isn't r or if second isn't between 0 and 7, not a register 
        return 0; //No other mnenomics start with r and have two characters. Also can't have a negative reg or one beyond 7. 
    }
    return 1; 
}

unsigned short getreg(char *p)              
{
    if(!isreg(p)){
        error("Not a register.\n");
    }
    char reg[2]; //will store in character array 
    int i = 0; 
    while(*p!='\0'){
        reg[i] = *p; 
        p++; 
        i++; 
    }
    if(!mystrcmpi(p, "fp")){
      return 5; 
    }
    else
    if(!mystrcmpi(p, "sp")){
      return 6; 
    } 
    else 
    if(!mystrcmpi(p, "lr")){
      return 7; 
    }

    switch(reg[1]){
        case '0':
            return 0; 
        case '1': 
            return 1;
        case '2': 
            return 2; 
        case '3': 
            return 3; 
        case '4': 
            return 4; 
        case '5': 
            return 5; 
        case '6': 
            return 6; 
        case '7': 
            return 7;
        default:
            error("Unknown error\n");
            return -1; 
    }
    /*
   Code missing here:
   Returns register number of the register whose name p points to.
   Calls error() if not passed a register name.
   */
}
unsigned short getadd(char *p) 
{
   int addressIndex = 0; 
   while(1){
      char* tempLabel = symbol[addressIndex];
      if(!tempLabel){
         error("Symbol not in symbol table.");
         break; 
      }
      if(strcmp(p, symbol[addressIndex])==0){
         return symadd[addressIndex]; 
      }
      addressIndex++; 
   }
    error("Unknown error.");
    return -1;
   // Code missing here:
   // Returns address of symbol p points to accessed from the symbol table.
   // Calls error() if symbol not in symbol table.
}

int main(int argc,char *argv[])
{
   if (argc != 2)
   {
      printf("Wrong number of command line arguments\n");
      printf("Usage: a1 <input filename>\n");
      exit(1);
   }

   // display your name, command line args, time
   time(&timer);      // get time
   printf("Lara Palombi   %s %s   %s", 
           argv[0], argv[1], asctime(localtime(&timer)));

   infile = fopen(argv[1], "r");
   if (!infile)
   {
      printf("Cannot open input file %s\n", argv[1]);
      exit(1);
   }

   // construct output file name
   strcpy(outfilename, argv[1]);        // copy input file name
   p1 = strrchr(outfilename, '.');      // search for period in extension
   if (p1)                              // name has period
   {
#ifdef _WIN32                           // defined only on Windows systems
      p2 = strrchr(outfilename, '\\' ); // compiled if _WIN32 is defined
#else
      p2 = strrchr(outfilename, '/');   // compiled if _WIN32 not defined
#endif
      if (!p2 || p2 < p1)               // input file name has extension?
         *p1 = '\0';                    // null out extension
   }
   strcat(outfilename, ".e");           // append ".e" extension

   outfile = fopen(outfilename, "wb");
   if (!outfile)
   {
      printf("Cannot open output file %s\n", outfilename);
      exit(1);
   }

   loc_ctr = linenum = 0;       // initialize, not required because global
   fwrite("oC", 2, 1, outfile); // output empty header

   // Pass 1
   printf("Starting Pass 1\n");
   while (fgets(buf, sizeof(buf), infile))
   {
      linenum++;  // update line number
      cp = buf;
      while (isspace(*cp))
         cp++;
      if (*cp == '\0' || *cp ==';'){  // if line all blank, go to next line
         continue;
      }
      strcpy(linesave, buf);        // save line for error messagesx
      if (!isspace(buf[0]))         // line starts with label (doesn't start with white space)
      {
         int valid_label = 1;
         label = strdup(strtok(buf, " \r\n\t:"));
         // Add code here that checks for a duplicate label, use strcmp()
         int symbolindex = 0; 
         while(1){
            char* tempLabel = symbol[symbolindex]; //setting a temporary label to check if symbol table has a null value 
            if(!tempLabel){
                  break; //break loop when no more labels to compare 
            }
            if(strcmp(label, symbol[symbolindex])==0){ //comparing labels using character pointers 
                  error("Duplicate label.\n");
                  valid_label = 0;
            }
            symbolindex++; //incrememnting symbol index
         }
         if(valid_label) {
            symbol[stsize] = label;
            symadd[stsize++] = loc_ctr;
            mnemonic = strtok(NULL," \r\n\t:"); // get ptr to mnemonic/directive
            o1 = strtok(NULL," \r\n\t,");      // get ptr to first operand
            // if(!mnemonic){
            //    loc_ctr++;
            //    continue; 
            // }
         }
      }
      else   // tokenize line with no label
      {
         mnemonic = strtok(buf, " \r\n\t");  // get ptr to mnemonic
         o1 = strtok(NULL, " \r\n\t,");       // get ptr to first operand
      }
      if (mnemonic == NULL)    // check for mnemonic or directive
         continue;

      if (!mystrcmpi(mnemonic, ".zero"))    // case insensitive compare
      {
         if (o1)
            rc = sscanf(o1, "%d", &num);    // get size of block from o1
         else
            error("Missing operand");
         if (rc != 1 || num > (65536 - loc_ctr) || num < 1)
            error("Invalid operand");
         loc_ctr = loc_ctr + num;
      }
      else{
         loc_ctr++;
      }
      if (loc_ctr > 65536)
         error("Program too big");
   }
   rewind(infile);

   // Pass 2
   printf("Starting Pass 2\n");
   loc_ctr = linenum = 0;      // reinitialize
   while (fgets(buf, sizeof(buf), infile))
   {
      linenum++;
      cp = buf;
      while (isspace(*cp))
         cp++;
      if (*cp == '\0' || *cp ==';')  // if line all blank, go to next line
         continue;
      strcpy(linesave, buf);        // save line for error messages

      if(!isspace(buf[0])){
         strtok(buf, " \r\n\t:"); //Doesn't need to input new labels in pass 2
         mnemonic = strtok(NULL, " \r\n\t:"); //tokenize first mnemonic/directive 
      } else {
         mnemonic = strtok(buf, " \r\n\t:"); //tokenize first mnemonic/directive 
      }

      o1 = strtok(NULL, " \r\n\t,"); //Tokenize first operand 
      o2 = strtok(NULL, " \r\n\t,"); //Tokenize second operand 
      o3 = strtok(NULL, " \r\n\t"); //tokenize third operand 

      // Code missing here:
      // Discard blank/comment lines, and save buf in linesave as in pass 1
      // Tokenize entire current line.
      // Do not make any new entries into the symbol table
      // if(!isspace(buf[0])){
      //    printf("\n%s", mnemonic);
      // }

      if(!mnemonic){ 
         //loc_ctr++; 
         continue; 
      }
      if (!mystrncmpi(mnemonic, "br", 2))    // case sensitive compares: if first two start with a branch instruction 
      {
         if (!mystrcmpi(mnemonic, "br" ))
            macword = 0x0e00;
         else
         if (!mystrcmpi(mnemonic, "brz" ))
            macword = 0x0000;
         else
         if (!mystrcmpi(mnemonic, "brnz" ))
            macword = 0x0200;
         else
         if (!mystrcmpi(mnemonic, "brn" ))
            macword = 0x0400;
         else
         if (!mystrcmpi(mnemonic, "brp" ))
            macword = 0x0600;
         else
         if (!mystrcmpi(mnemonic, "brlt" ))
            macword = 0x0800;
         else
         if (!mystrcmpi(mnemonic, "brgt" ))
            macword = 0x0a00;
         else
         if (!mystrcmpi(mnemonic, "brc" ))
            macword = 0x0c00;
         else
            error("Invalid branch mnemonic");
         pcoffset9 = (getadd(o1) - loc_ctr - 1);    // compute pcoffset9 of branch instruction: write getadd. Takes from pointer o1. 
         if (pcoffset9 > 255 || pcoffset9 < -256)
            error("pcoffset9 out of range");
         macword = macword | (pcoffset9 & 0x01ff);  // assemble inst
         fwrite(&macword, 2, 1, outfile);           // write out instruction
         loc_ctr++; //increases line 
      }
      else
      if (!mystrcmpi(mnemonic, "add" ))
      {
         if (!o3)
            error("Missing operand");
         dr = getreg(o1) << 9;   // get and position dest reg number
         sr1 = getreg(o2) << 6;  // get and position srce reg number

         if (isreg(o3)) // is 3rd operand a reg?
         {
            sr2 = getreg(o3);      // get third reg number
            macword = 0x1000 | dr | sr1 | sr2; // assemble inst 
         }
         else
         {
            if (sscanf(o3,"%d", &num) != 1)    // convert imm5 field
               error("Bad imm5");
            if (num > 15 || num < -16)
               error("imm5 out of range");
            macword = 0x1000 | dr | sr1 | 0x0020 | (num & 0x1f);
         }
         fwrite(&macword, 2, 1, outfile);      // write out instruction
         loc_ctr++;
      }

      else
      if (!mystrcmpi(mnemonic, "ld" ))
      {
         if(!o1 || !o2){
            error("Missing operand");
         }
         dr = getreg(o1) << 9;// get and position destination reg number

         pcoffset9 = getadd(o2) - loc_ctr - 1;

         if (pcoffset9 > 255 || pcoffset9 < -256)
            error("pcoffset9 out of range");

         macword = 0x2000 | dr | (pcoffset9 & 0x1ff);   // assemble inst

         fwrite(&macword, 2, 1, outfile); // write out instruction
         loc_ctr++;
      }

      else
      if (!mystrcmpi(mnemonic, "st"))
      {
         if(!o1 || !o2){
            error("Missing operand");
         }
         sr = getreg(o1) << 9; //get and position source register
         pcoffset9 = (getadd(o2) - loc_ctr-1);
         if (pcoffset9 > 255 || pcoffset9 < -256){
            error("pcoffset9 out of range");
         }
         macword = 0x3000 | sr | (pcoffset9 & 0x1ff); //assemble inst
         fwrite(&macword, 2, 1, outfile); // write out instruction
         loc_ctr++;
      }

      else
      if (!mystrcmpi(mnemonic, "bl"))
      {
      
         if(!o1){
            error("Missing operand");
         }
         pcoffset11 = (getadd(o1)-loc_ctr-1);
         if(pcoffset11 > 1023 || pcoffset11 < -1024){
            error("pcoffset11 out of range");
         }
         macword = 0x4000 | 0x0800 | (pcoffset11 & 0x7ff); 
         fwrite(&macword, 2, 1, outfile); // write out instruction
         loc_ctr++;         
      }

      else
      if (!mystrcmpi(mnemonic, "blr"))
      {
         if(!o1){
            error("Missing operand");
         }
         baser = getreg(o1)<<6; 
         
         if (mystrcmpi(o2, ";"))                         // offset6 specified?
         {
            if (sscanf(o2,"%d", &num) != 1)    // convert offset6 field
               error("Bad offset6");
            if (num > 31 || num < -32)
               error("offset6 out of range");
         }
         else
            num = 0;                           // offset6 defaults to 0
         
         macword = 0x4000 | baser | (num & 0x3f); 
         fwrite(&macword, 2, 1, outfile); // write out instruction
         loc_ctr++; 
      }

      else
      if (!mystrcmpi(mnemonic, "and"))
      {
         if(!o1 || !o2){
            error("Missing operand.");
         }
         dr = getreg(o1)<<9;  //get and position destination register 
         sr1 = getreg(o1) << 6; 

         if(isreg(o3)){
            sr2 = getreg(o3); 
            macword = 0x5000 | dr | sr1 | sr2; //assemble instruc
         }
         else{
            if(sscanf(o3, "%d", &num)!=1){
               error("Bad imm5");
            }
            if(num>15 || num< -16){
               error("imm5 out of range");
            }
            macword = 0x5000 | dr | sr1 | 0x0020 | (num & 0x1f);
         }
         fwrite(&macword, 2, 1, outfile); // write out instruction
         loc_ctr++; 
      }

      else
      if (!mystrcmpi(mnemonic, "ldr"))
      {
         if(!o1 || !o2){
            error("Missing operand");
         }
         dr = getreg(o1)<<9; 
         baser = getreg(o2)<<6; 
         if (o3)                         // offset6 specified?
         {
            if (sscanf(o3,"%d", &num) != 1)    // convert offset6 field
               error("Bad offset6");
            if (num > 31 || num < -32)
               error("offset6 out of range");
         }
         else
            num = 0;                           // offset6 defaults to 0
         macword = 0x6000 | dr | baser | (num & 0x3f); 
         fwrite(&macword, 2, 1, outfile); // write out instruction
         loc_ctr++;
      }

      else
      if (!mystrcmpi(mnemonic, "str"))
      {
         if(!o1 || !o2){
            error("Missing operand");
         }
         sr = getreg(o1) << 9; 
         baser = getreg(o2)<<6; 
         if (o3)                         // offset6 specified?
         {
            if (sscanf(o3,"%d", &num) != 1)    // convert offset6 field
               error("Bad offset6");
            if (num > 31 || num < -32)
               error("offset6 out of range");
         }
         else
            num = 0;                           // offset6 defaults to 0
 
         macword = 0x7000 | sr | baser | (num & 0x3f);
         fwrite(&macword, 2, 1, outfile); // write out instruction
         loc_ctr++;
      }

      else
      if (!mystrcmpi(mnemonic, "not"))
      {
         if(!o1 || !o2){
            error("Missing operand");
         }
         dr = getreg(o1) << 9; 
         sr1 = getreg(o2) << 6; 
         macword = 0x9000 | dr | sr1; 
         fwrite(&macword, 2, 1, outfile); // write out instruction
         loc_ctr++;
      }

      else
      if (!mystrcmpi(mnemonic, "jmp" ))     // also ret instruction
      {
         baser = getreg(o1) << 6;        // get reg number and position it
         if (o2)                         // offset6 specified?
         {
            if (sscanf(o2,"%d", &num) != 1)    // convert offset6 field
               error("Bad offset6");
            if (num > 31 || num < -32)
               error("offset6 out of range");
         }
         else
            num = 0;                           // offset6 defaults to 0
         // combine opcode, reg number, and offset6
         macword = 0xc000 | baser | (num & 0x3f);       
         fwrite(&macword, 2, 1, outfile);  // write out instruction
         loc_ctr++;
      }
      else
      if (!mystrcmpi(mnemonic, "ret" ))     
      {
         if(o1){
            if(sscanf(o1, "%d", &num)!=1){
               error("Bad offset6");
            }
            if (num > 31 || num < -32){
               error("offset6 out of range");
            }
         }
         else{
            num = 0; //offset6 defaults to 0 
         }
         macword = 0xc000 | 0x1c0 | (num & 0x3f);
         fwrite(&macword, 2, 1, outfile);  // write out instruction
         loc_ctr++;
      }
      else
      if (!mystrcmpi(mnemonic, "lea")) 
      {
         if(!o1 || !o2){
            error("Missing operand");
         }
         dr = getreg(o1)<<9; 
         pcoffset9 = (getadd(o2) - loc_ctr - 1);
         if (pcoffset9 > 255 || pcoffset9 < -256){
            error("pcoffset9 out of range");
         }
         macword = 0xe000 | dr | pcoffset9 & 0x1ff;

         fwrite(&macword, 2, 1, outfile);  // write out instruction
         loc_ctr++;   
      }

      else
      if (!mystrcmpi(mnemonic, ".word")) 
      {
         sscanf(o1, "%x", &num); 
         if(num>32765 || num<-32768){
            error(".word out of range");
         }
         
         macword = num; 
         fwrite(&macword, 2, 1, outfile);  // write out instruction
         loc_ctr++;  
      }

      else
      if (!mystrcmpi(mnemonic, "halt")){
         macword = 0xf000;
         fwrite(&macword, 2, 1, outfile);  // write out instruction
         loc_ctr++; 
      }

      else
      if (!mystrcmpi(mnemonic, "nl")){
         macword = 0xf001;
         fwrite(&macword, 2, 1, outfile);  // write out instruction
         loc_ctr++; 
      }
      else
      if (!mystrcmpi(mnemonic, "dout")){
         if(o1 == NULL || mystrcmpi(o1, ";")) {
            sr = 0;
         }
         else 
         if(!mystrcmpi(o1, ";")){
            sr = 0;
         }
         else {
            sr = getreg(o1)<<9; 
        }
         macword = 0xf000 | sr | 0x2;
         fwrite(&macword, 2, 1, outfile);  // write out instruction
         loc_ctr++;  
      }

      else
      if (!mystrcmpi(mnemonic, ".zero"))
      {
         macword = 0;
         //sscanf(o1, "%d", &num);             // get size of block
         //int size; 
         sscanf(o1, "%d", &num);             // get size of block
         //sscanf(o1, "%d", &size);
         loc_ctr = loc_ctr + num;
         while (num--)                       // write out a block of zeros
            fwrite(&macword, 2, 1, outfile);
         //loc_ctr = loc_ctr + num;            // adjust loc_ctr 
      }
      else
         error("Invalid mnemonic or directive");
   }
   
   fclose(infile);
   return 0; 
}