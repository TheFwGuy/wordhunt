/*
 *  Word Hunter
 *  This program is looking in a word hunter crossword for some words.
 *  Programmer : SB - Sept 2008
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLENWORD  100
#define NOCHARMASK  0xFFFF

#define RF_READROW	0	/* Read file status - Acquire number of rows */
#define RF_READCOL	1	/* Read file status - Acquire number of columns */
#define RF_READCRW	2	/* Read file status - Read crossword */
#define RF_FINISH	3	/* Read file status - Finish */

/*
 *  Global variables
 */
unsigned char *xword_matrix;		/* Pointer for the cross word matrix */
int Limit_X = 10;			/* Limit for the X position of the Matrix */
int Limit_Y = 10;			/* Limit for the Y position of the Matrix */
int Foundword[MAXLENWORD];		/* Array to store the sequence of address for the found word */
int Direction;				/* The variable contains the direction of the word found 
                                         *
                                         *    1 -> letter found - direction left diagonal up    (X-1 / Y-1)
                                         *    2 -> letter found - direction up                  (X-1 / Y  )
                                         *    3 -> letter found - direction right diagonal up   (X-1 / Y+1)
                                         *    4 -> letter found - direction left                (X   / Y-1)
                                         *    5 -> letter found - direction right               (X   / Y+1)
                                         *    6 -> letter found - direction left diagonal down  (X+1 / Y-1) 
                                         *    7 -> letter found - direction down                (X+1 / Y  )
                                         *    8 -> letter found - direction right diagonal down (X+1 / y+1)
                                         */
/*
 *  Function prototypes 
 */

void init_matrix();
void load_matrix();
void display_matrix(char *);
int search_matrix(char *);
char search_letter(char, int *, int *);
char search_direction(char *, char, int, int);
char check_coordinates(char, int, int);
char set_coordinates(char, int *, int *);
int prepare_address(int, int);
void initFoundword();
void reorderFoundword();

/*
 *  The program has some line input.
 *  ./wh f
 *
 *  If the 'f' parameter exists, then a file named xword.txt is searched and if found read it.
 *  The file is structured in this way :
 *  - # in the first column indicate a comment
 *  - The first valid row contains the number of rows
 *  - The second valid row contains the number of columns
 *  - the rest of the file will contains the crossword, each line is a row
 *
 */
int main(int argc, char * *argv)
{
   int i;
   int sx, sy = 0;
   char buffer[80];
   char ch, isSearch;
   FILE *xword_fp;
   char buf81[81];
   char stateReadFile = RF_READROW;

   /*
    *  Inputselection is a flag that indicate where the crossword is coming :
    *   0 = local input
    *   1 = file
    *   2 = leave init crossword
    */
   char inputSelection = 0;

   if(argc > 1)
   {
      if((*argv[1] == 'f') || (*argv[1] == 'F'))
         inputSelection = 1;

      if((*argv[1] == 'h') || (*argv[1] == 'H'))
      {
         printf("\n\nCWHS - Crossword Word Hunting Solver\n");
         printf("------------------------------------\n\n");
         printf("This program allow to search in a crossword, if a word exist. \n");
         printf("The crossword can be loaded manually or using a file named xword.txt in  \n");
         printf("the same directory of this program. \n");
         printf("To load the file, digit ./cwhs f    \n");
         printf("To exit from the program, just press Enter at the search word request.\n");
         printf("Pay attention to use for the search the same letter case present in the crossword. \n\n");

         exit(0);
      }
   }

   printf("\n\nCWHS - Crossword Word Hunting Solver\n");
   printf("------------------------------------\n");
   printf("(digit ./cwhs h  for help)\n\n");

   initFoundword();        	/* Initialize found word array */

   if(inputSelection == 0)
   {
      printf("Input how many rows : ");
      scanf("%d", &Limit_X);
      printf("Set %d num rows\n", Limit_X);

      printf("Input how many columns : ");
      scanf("%d", &Limit_Y);
      printf("Set %d num columns\n", Limit_Y);
 
      xword_matrix = malloc(Limit_X * Limit_Y * sizeof(char *));

      init_matrix();   
      display_matrix("->manual<-");
      load_matrix();
   } 
   else if(inputSelection == 1)
   {
      /* Read file "xword.txt" */
      printf("Reading file xword.txt in order to load the crossword\n");
      if((xword_fp = (fopen("xword.txt", "r"))) == (FILE *) NULL)
      {
         printf("Could not open xword.txt\n");
         exit(0);
      }

      while(fgets(buf81, 80, xword_fp))              /* Read chars from file    */
      {
         if(buf81[0] == '#')
            continue;

         switch(stateReadFile)
         {
            case RF_READROW:	/* Acquire number of rows */
               sscanf(buf81, "%d", &Limit_X);
               printf("Set %d num rows\n", Limit_X);
               stateReadFile++;
               break;

            case RF_READCOL:	/* Acquire number of columns */
               sscanf(buf81, "%d", &Limit_Y);
               printf("Set %d num columns\n", Limit_Y);
               stateReadFile++;
               break;
        
            case RF_READCRW:	/* Allocate crossword */
               xword_matrix = malloc(Limit_X * Limit_Y * sizeof(char *));

               init_matrix();   
/*             display_matrix();  */
               stateReadFile++;
               sx = 0;
               sy = 0;
               break;
        
            case RF_FINISH:	/* Load crossword, a row at a time */
               for(sy=0; sy< Limit_Y; sy++)
               {
                  xword_matrix[sx * Limit_Y + sy] = buf81[sy];
               }
               sx++;
               break;

            default:
               printf("ERROR reading file !!");
               exit();
               break;
         }
      }

      fclose(xword_fp);
      printf("Downloading Completed!\n");
      printf("Here the matrix loaded\n");
      display_matrix("-");
   }

   isSearch = 1;
   while(isSearch)
   {
      printf("\nStart search ! Input the word to find : ");
      /* Read in single line from "stdin": */
      for( i = 0; (i < 80) &&  ((ch = getchar()) != EOF) 
                           && (ch != '\n'); i++ )
         buffer[i] = (char)ch;
      buffer[i]=0;

      if(i==0 && ch=='\n')
         isSearch = 0;
      else
         if(search_matrix(buffer))
            display_matrix(buffer);
   }

   free(xword_matrix);
   printf("\nBye\n\n");
}

/*
 *  init_matrix
 *  This function create and initialize a matrix for the crossword letters
 *  The limits are loaded before
 */
void init_matrix()
{
   int x,y;
   char letter = 'A';

   for(x=0; x< Limit_X; x++)
   {
      for(y=0; y< Limit_Y; y++)
      {
         xword_matrix[x * Limit_Y + y] = letter;
      }

      letter++;
   }
}

/*
 *  load_matrix
 *  This function create and load a matrix with the crossword letters
 *  The limits are loaded before
 */
void load_matrix()
{
   int i,x,y,ch;
   unsigned char valid_input = 0;
   char buffer[81];

   printf("\n Input the word hunt crossword, one row at time\n");

   /*
    *  Flush STDIN
    */

   while((ch = getchar()) != EOF && ch != '\n')
      continue;

   for(x=0; x< Limit_X; x++)
   {
      valid_input = 0;

      do
      {
         printf("Input row n. %d : ", x);
         /* Read in single line from "stdin": */
         for( i = 0; (i < 80) &&  ((ch = getchar()) != EOF) 
                              && (ch != '\n'); i++ )
            buffer[i] = (char)ch;

         if(i == 0 && ch == '\n')		/* If no input, keep the matrix row */
         {
            for(y=0; y< Limit_Y; y++)
            {
               buffer[y] = xword_matrix[x * Limit_Y + y];
            }
            valid_input = 1;
            break;
         }

         if(i != Limit_Y)
            printf("\n\nATTENTION ! The number of characters introduced are different than %d !\n", Limit_Y);
         else
            valid_input = 1;

         if(i > Limit_Y)
         {
            printf("The extra character will be ignored !\n");
            valid_input = 1;
         }
         if(i < Limit_Y)
            printf("Not enough characters ! Do it again !\n");
      } while(!valid_input);

      for(y=0; y< Limit_Y; y++)
      {
         xword_matrix[x * Limit_Y + y] = buffer[y];
      }
      printf("\n");
   }      
}

/*
 *  display_matrix
 *  This function print out the matrix with the crossword letters
 *  The limits are loaded before
 */
void display_matrix(char *searchname)
{
   int x,y;
   int address;
   int found_index = 0;
   int display_found = 0;

   printf("\n\n------Display Matrix [%s]------\n\n", searchname);

   reorderFoundword();

   /*
    *  Print the Colum title
    */
   printf("Column  ");
   for(y=0; y< Limit_Y; y++)
      printf("%02d ", y);
   printf("\n");

   printf("----------");
   for(y=0; y< Limit_Y; y++)
      printf("---", y);
   printf("\n");

   /*
    *  Print the matrix and the row title
    */

   for(x=0; x< Limit_X; x++)
   {
      printf("Row %02d -", x);

      for(y=0; y< Limit_Y; y++)
      {
         address = prepare_address(x,y);  /* Calculate the address of the letter to display */

         if(Foundword[found_index] != NOCHARMASK &&
            address == Foundword[found_index])
         {
            printf("[%c]", xword_matrix[address]);
            if(found_index < MAXLENWORD)
               found_index++;
         }
         else
            printf(" %c ", xword_matrix[address]);

      }
      printf(" \n");
   }
}

/*
 *  The function search if the input word is present in the matrix
 *  Return 0 if the word is not found, 1 if the word is found
 */
int search_matrix(char *word)
{
   int retfunz = 0;
   int mtx_x, mtx_y;
   int fnd_x, fnd_y;
   int fnd_index = 0;
   int statesearch = 0;
   char ch;
   char dirflag = 0xff;

   /*
    *  Search the first letter of the word in the matrix
    *  Initialize the coordinates
    */
   mtx_x = 0;	/* Coordinates for the matrix search */
   mtx_y = 0;
   fnd_x = 0;	/* Coordinates for the word search */
   fnd_y = 0;

   initFoundword();		/* Initialize found word array */
   
   do
   {
      switch(statesearch)
      {
         case 0:  /* Search the first letter */
            ch = word[0];
/*          printf("Looking for first letter of [%s] [%c] starting from %d,%d\n", word, ch, mtx_x, mtx_y); */
            if(search_letter(ch, &mtx_x, &mtx_y))
            {
/*             printf("Found first letter of [%s] at %d,%d\n", word, mtx_x, mtx_y); */
               dirflag = 0xFF;		/* Enable all the possible direction */
               Foundword[0] = prepare_address(mtx_x, mtx_y);
               statesearch = 1;
            }
            else
            {
               statesearch = 99;		/* Force endsearch ! */
               initFoundword();         	/* Erase found word array */
               printf("The word [%s] is not in the crossword\n", word);
            }
            break;

         case 1:  /* Search the second letter and direction */
            ch = word[1];
/*          printf("Looking for second letter of [%s][%c] around %d,%d\n", word, ch, mtx_x, mtx_y); */

            fnd_x = mtx_x;
            fnd_y = mtx_y;

            Direction =  search_direction(&dirflag, ch, fnd_x, fnd_y);
            fnd_index = 1;	/* Start from the second letter for the search if the test is successful */
            statesearch = 2;	/* Assume second letter is found. If not, default force back */

            switch(Direction)
            {
                case 1: /* left diag up */
/*                 printf("Letter %c found at left diag up\n", ch); */  /* Diagnostic print */
                   dirflag &= ~0x01;
                   break;
                case 2: /* up  */
/*                 printf("Letter %c found at up\n", ch); */  /* Diagnostic print */
                   dirflag &= ~0x02;
                   break;
                case 3: /* right diag up */
/*                 printf("Letter %c found at right diag up\n", ch);  */  /* Diagnostic print */
                   dirflag &= ~0x04;
                   break;
                case 4: /* left */
/*                 printf("Letter %c found at left\n", ch); */  /* Diagnostic print */
                   dirflag &= ~0x08;
                   break;
                case 5: /* right */
/*                 printf("Letter %c found at right\n", ch); */  /* Diagnostic print */
                   dirflag &= ~0x10;
                   break;
                case 6: /* left diag down */
/*                 printf("Letter %c found at left diag down\n", ch); */  /* Diagnostic print */
                   dirflag &= ~0x20;
                   break;
                case 7: /* down */
/*                 printf("Letter %c found at down\n", ch); */  /* Diagnostic print */
                   dirflag &= ~0x40;
                   break;
               case 8: /* right diag down */
/*                 printf("Letter %c found at right diag down\n", ch); */  /* Diagnostic print */
                   dirflag &= ~0x80;
                   break;
               default:
/*                 printf("Letter %c not found around %d, %d\n", ch, fnd_x, fnd_y); */  /* Diagnostic print */
                   /*
                    *  Second letter not found !
                    *  Return to the state 0 and continue to search for the first letter starting
                    *  from the last coordinates
                    */
                   statesearch = 0;

                   mtx_y++;		/* Update to the next character */
                   if(mtx_y > Limit_Y-1)
                   {
                      mtx_x++;
                      mtx_y = 0;
                      if(mtx_x > Limit_X-1)
                         statesearch = 99;
                   } 
                   break;
            }
            break;

         case 2:	/* Check for other letters following the direction */
            /*
             *  Update the found coordinates to the second letter using the found direction
             *  and re-check
             */
            if(set_coordinates(Direction, &fnd_x, &fnd_y))
            {
               if(check_coordinates(word[fnd_index], fnd_x, fnd_y))
               {
                  /*
                   *  Letter found - pass to the next following the same direction
                   */
                  Foundword[fnd_index] = prepare_address(fnd_x, fnd_y);
                  fnd_index++;
                  if(fnd_index == strlen(word))
                  {
                     printf("Word [%s] found in the crossword ! \n", word);
                     retfunz=1;
                     statesearch = 99;
                  }
               }
               else
               {
                  /*
                   *  Letter not found ! Abort the word search and go back to direction search
                   *  Return to the state 1 and continue to search for the second letter starting
                   *  from the last coordinates
                   */
                  statesearch = 1;
               }
            }
            else
            {
               /*
                *  Coordinates over the limits ! Abort the word search and go back to matrix search
                *  Return to the state 0 and continue to search for the first letter starting
                *  from the last coordinates
                */
               statesearch = 1;
            }
            break;
      }
   } while(statesearch != 99);
   return(retfunz);
}

/*
 *  Search letter
 *  Starting from a specific coordinates (X and Y) the function is looking for
 *  the input letter in the matrix
 *  The function return the found letter or 0 if the letter is not present
 *  The coordinates are updated as weel to the found letter
 */
char search_letter(char letter, int *inp_x, int *inp_y)
{
   int x,y;

   y=*(inp_y);

   for(x=*(inp_x); x< Limit_X; x++)
   {
      for(;y< Limit_Y; y++)
      {
         if(xword_matrix[x * Limit_Y + y] == letter)
         {
            *(inp_x) = x;
            *(inp_y) = y;
            return(letter);
         }
      }
      y=0;
   }
   return(0);
}

/*
 *  Search direction
 *  Starting from a specific coordinates (X and Y) the function is looking for
 *  the input letter in the box around the coordinates.
 *  The function return :
 *    0 -> no letter found around the coordinates
 *    1 -> letter found - direction left diagonal up    (X-1 / Y-1)
 *    2 -> letter found - direction up                  (X-1 / Y  )
 *    3 -> letter found - direction right diagonal up   (X-1 / Y+1)
 *    4 -> letter found - direction left                (X   / Y-1)
 *    5 -> letter found - direction right               (X   / Y+1)
 *    6 -> letter found - direction left diagonal down  (X+1 / Y-1) 
 *    7 -> letter found - direction down                (X+1 / Y  )
 *    8 -> letter found - direction right diagonal down (X+1 / y+1)
 *
 *  The function uses a byte (bit setting) to allow a direction.
 *  This is needed for multiple research
 *  The flagdir has this format :
 *    xxxxxxxx
 *    ||||||||__ direction 1  (0 ignore - 1 allow)
 *    |||||||__ direction 2  (0 ignore - 1 allow)
 *    ||||||__ direction 3  (0 ignore - 1 allow)
 *    |||||__ direction 4  (0 ignore - 1 allow)
 *    ||||__ direction 5  (0 ignore - 1 allow)
 *    |||__ direction 6  (0 ignore - 1 allow)
 *    ||__ direction 7  (0 ignore - 1 allow)
 *    |__ direction 8  (0 ignore - 1 allow)
 */
char search_direction(char *dirflag, char letter, int x, int y)
{
   int address;
   char flagdir = *(dirflag);
 
   /* Check for left diagonal up */
   if((flagdir & 0x01) && (x > 0 && y > 0))
   {
      address = (x-1) * Limit_Y + (y-1);
      if(xword_matrix[address] == letter)
         return(1);	/* Found left diagonal up ! */
   }      

   /* Check for up */
   if((flagdir & 0x02) && (x > 0))
   {
      address = (x-1) * Limit_Y + y;
      if(xword_matrix[address] == letter)
         return(2);	/* Found up ! */
   }      

   /* Check for right diagonal up */
   if((flagdir & 0x04) && (x > 0 && y < Limit_Y))
   {
      address = (x-1) * Limit_Y + (y+1);
      if(xword_matrix[address] == letter)
         return(3);	/* Found right diagonal up ! */
   }      

   /* Check for left */
   if((flagdir & 0x08) && (y > 0))
   {
      address = x * Limit_Y + (y-1);
      if(xword_matrix[address] == letter)
         return(4);	/* Found left ! */
   }      

   /* Check for right */
   if((flagdir & 0x10) && (y < Limit_Y))
   {
      address = x * Limit_Y + (y+1);
      if(xword_matrix[address] == letter)
         return(5);	/* Found right ! */
   }      

   /* Check for left diagonal down */
   if((flagdir & 0x20) && (x < Limit_X && y > 0))
   {
      address = (x+1) * Limit_Y + (y-1);
      if(xword_matrix[address] == letter)
         return(6);	/* Found left diagonal down ! */
   }      

   /* Check for down */
   if((flagdir & 0x40) && (x < Limit_X))
   {
      address = (x+1) * Limit_Y + y;
      if(xword_matrix[address] == letter)
         return(7);	/* Found down ! */
   }      

   /* Check for right diagonal down */
   if((flagdir & 0x80) && (x < Limit_X && y < Limit_Y))
   {
      address = (x+1) * Limit_Y + (y+1);
      if(xword_matrix[address] == letter)
         return(8);	/* Found right diagonal down ! */
   }      

   return(0);
}

/*
 *  check_coordinates
 *  The function check if a letter exist at specific coordinates
 *  The function return :
 *    0 -> no letter founda at the coordinates
 *    1 -> letter found
 */
char check_coordinates(char letter, int x, int y)
{
   int address;
 
   address = prepare_address(x,y);

/*   printf("Check_coordinates [%d,%d] - input letter : [%c] - found letter : [%c]\n",
           x,y, letter, xword_matrix[address]);  */  /* Diagnostic print */

   if(xword_matrix[address] == letter)
      return(1);	/* Found letter */
   else
      return(0);
}

/*
 *  set_coordinates
 *  The function calculate a new set of coordinates giving a starting X,Y and a direction.
 *  The function return :
 *    0 -> impossible to calculate coordinates (es. reach limits)
 *    1 -> coordinates calculated
 */
char set_coordinates(char direction, int *inp_x, int *inp_y)
{
   int x,y;

   x=*(inp_x);
   y=*(inp_y);

/* printf("set_coordinates - Input [%d:%d] ", x,y); */  /* Diagnostic print */

   switch(direction)
   {
      case 1: /* left diag up - X-1 / Y-1 */
         x--;
         y--;
         break;
      case 2: /* up - X-1 / Y  */
         x--;
         break;
      case 3: /* right diag up - X-1 / Y+1 */
         x--;
         y++;
         break;
      case 4: /* left  - X / Y-1 */
         y--;
         break;
      case 5: /* right - X / Y+1 */
         y++;
         break;
      case 6: /* left diag down - X+1 / Y-1 */
         x++;
         y--;
         break;
      case 7: /* down - X+1 / Y */
         x++;
         break;
      case 8: /* right diag down - X+1 / Y+1 */
         x++;
         y++;
         break;
      default:
/*       printf("Direction not allowed !\n");  */  /* Diagnostic print */
         return(0);
         break;
   }

   /*
    *  Check for Limits !
    */
   if((x >=0 && x< Limit_X) && (y >=0 && y< Limit_Y))
   {
/*    printf(" Output [%d:%d]\n", x,y); */  /* Diagnostic print */

      *(inp_x) = x;
      *(inp_y) = y;
      return(1);
   }
   else
   {
      printf("Output out of limits\n");
      return(0);	/* Out of limits ! */
   }
}    

/*
 *  prepare_address
 *  The function return the address of a letter giving the X and Y
 */
int prepare_address(int x, int y)
{
   return(x * Limit_Y + y);
}

/*
 *  Initialize Foundword array
 */
void initFoundword()
{
   int i;
   for(i=0; i<MAXLENWORD; i++)	        /* Erase found word array */
      Foundword[i]=NOCHARMASK;
   Direction = 0;	
}

/*
 *  reorder_foundword
 *  The function reorder the foundword array considering the direction
 *  The Funword array always contains the found wourd sequence in the alphabetical order.
 *  The display_matrix function, prints out the matrix starting from the top left corner
 *  to the lower right corner, row by row.
 *  So to correctly display the found word, the Foundword array needs to contain the 
 *  found word in the display order and not alphabetical order.
 */
void reorderFoundword()
{
   int temparray[MAXLENWORD];
   int foundword_len = 0;
   int i;

   for(i=0; i<MAXLENWORD; i++)	        /* Erase temp array and count length found word*/
   {
      temparray[i]=NOCHARMASK;
      if(Foundword[i] != NOCHARMASK)
         foundword_len++;
   }

   printf("Word %d character length\n", foundword_len);

/* printf("Show Foundword before reorder \n");
   for(i=0; i<MAXLENWORD; i++)
      if(Foundword[i] != NOCHARMASK)
         printf("%d ",Foundword[i]);
   printf("\n"); */  /* Diagnostic print */

   switch(Direction)
   {
      case 1: /* left diag up - X-1 / Y-1 */
         printf("Word in diagonal from right to left - up\n");
         break;
      case 2: /* up - X-1 / Y  */
         printf("Word in vertical, first letter down \n");
         break;
      case 3: /* right diag up - X-1 / Y+1 */
         printf("Word in diagonal from left to right - up\n");
         break;
      case 4: /* left  - X / Y-1 */
         printf("Word in horizontal from right to left\n");
         break;
      case 5: /* right - X / Y+1 */
         printf("Word in horizontal from left to right\n");
         break;
      case 6: /* left diag down - X+1 / Y-1 */
         printf("Word in diagonal from right to left - down \n");
         break;
      case 7: /* down - X+1 / Y */
         printf("Word in vertical - first letter up \n");
         break;
      case 8: /* right diag down - X+1 / Y+1 */
         printf("Word in diagonal from left to right - down \n");
         break;
   }

   printf("\n");

   /*
    *  If the direction is compatible with the display, get out
    */
   if(Direction == 5 || Direction == 6 || Direction == 7 || Direction == 8)
      return;

   foundword_len--;

   switch(Direction)
   {
      case 1: /* left diag up - X-1 / Y-1 */
      case 2: /* up - X-1 / Y  */
      case 3: /* right diag up - X-1 / Y+1 */
      case 4: /* left  - X / Y-1 */
         for(i=0; i<MAXLENWORD; i++)
         {
            temparray[i]=Foundword[foundword_len];
            foundword_len--;
            if(foundword_len < 0)
               break;
         }
         break;
   }

   for(i=0; i<100; i++)	        /* Restore reordered array */
   {
      Foundword[i] = temparray[i];
   }

/* printf("Show Found word after  reorder \n");
   for(i=0; i<MAXLENWORD; i++)
      if(Foundword[i] != NOCHARMASK)
         printf("%d ",Foundword[i]);
   printf("\n"); */  /* Diagnostic print */

   return;
}

