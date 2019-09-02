
/************************************************************************************

 CS 2604 program 4: hashing
 Program name: hash.c
 Greg Fudala
 xxx-xx-xxxx
 system: MAC under A/UX   

 Purpose: This program is an employee record data retrieval system. It allows search
	  by promary key on records stored externally using hashing, in addition
	  to retrieval using secondary keys. As input, the program can take three
	  commands. An enter command takes as its parameters the employee name,
	  address, ID num, management level, salary, and number of cats owned. This
	  procedure computes a hash fcn. and begins to create a hash file. The
	  secondary keys are stored in memory in binary trees allowing for log n
	  searching time. The delete command deletes an employee from the external
	  hash file and the secondary key trees. Either the name or the ID num
	  are the valid parameters. The third command is search. It takes as
	  parameters a key index, lower bound, and upper bound. It simply outputs
	  all employees identified by the key index(specifies sec. key) within
	  the bounds given.

	  Input is echoed to the screen and all output information save the hashing
	  table(file) is also printed to the screen.

   ***NOTE:  I regret I ran out of time and couldn't complete the DELETE routine.
	     However, a portion is coded.

 ************************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>

FILE *infile,*outfile,					/* input & output file ptrs */
     *fopen();						/* fopen routine */

struct emp {						/* employee struct */ 
   char chrstr[80];					/* name & address */
   long idnum;
   long manlevel;
   long salary;
   long numcats;
   char leftover[24];					/* left over space */
   char leftover2[4];
   long tombstone;					/* empty flag */
   };

struct emp buffer[8],				/* array of 8 employees(block) */ 
	   overflow[8]; 

struct skeylist {				/* list of employees */
   int blocknum;
   int index;
   struct skeylist *next;
   };


struct skey1 {					/* skey 1 node type */
   int idnum;
   int blocknum;
   int index;
   struct skey1 *left, *right;
   };

struct skey234 {				/* all other skeys node type */
   int trait;
   struct skeylist *list;
   struct skey234 *left, *right;
   };


char aline[80];					/* name & address */
char command[7];				/* command */
struct emp tempnode;				/* temporary employee var */

int i,j,					/* ctrs */
    slotnum,					/* hashing result */ 
    blocknum,					/* block num */
    index,					/* index into a block (0-7) */
    res,					/* computing variable */
    skey1flag,					/* found data flag */
    wall,					/* marker to verify lin. probe */
    numoverflow,				/* num. overflow blocks */
    currblock,					/* current block number */
    searchkey,					/* which secondary key variable */
    lbound,ubound,				/* search bounds parameters */
    delkey,					/* which deletion key flag */ 
    delidnum; 					/* id num to delete */

struct skey1 *idnumbt;				/* idnum bin tree */
struct skey234 *manlevelbt,			/* man. level bin tree */ 
	       *salarybt, 			/* salary bt */
	       *numcatsbt;			/* num cats bt */





compress(source,dest)

/**********************************************************************************

 function compress
 Purpose: This function removes all leading and trailing blanks from the name and
	  address and compresses all internal blanks to one blank only.

 **********************************************************************************/

char source[80], dest[80];
   {
   while (*source != NULL)
      {
      while (isspace(*source)) 
         ++source;
      while ((!isspace(*source)) && (*source != ':') && (*source != NULL))
	 {
	 *dest = *source;		/* copy characters */
	 ++source;
	 ++dest;
	 }
      					/* exit at space or : or eoln */
      if (*source == ':')
	 {
	 ++source;
	 *dest = NULL;
	 ++dest;
	 }
      if (isspace(*source))
	 if ((*(source+1) != ':') && (*(source+1) != NULL))
	    {
	    *dest = *source;
	    ++source;
	    ++dest;
	    }

      }
      *dest = NULL;
   }





int hash(str)

/********************************************************************************

 function hash
 Purpose: This function returns the hashing value (SUM is ASCII values mod 211)

 ********************************************************************************/

char str[80];
   {
   int sum,i,len;
   sum = 0;
   len = strlen(str);
   for (i=0; i<=len-1; ++i)
      sum = sum + str[i];				/* ASCII sum */
   return(sum % 211);
   }





struct skey1 *btinsertskey1(bt, idnum, blocknum, index)

/********************************************************************************

 function btinsertskey1
 Purpose: This function inserts an employee into the id num binary tree.

 ********************************************************************************/

struct skey1 *bt;
int idnum, blocknum, index;
   {
   if (bt == NULL)
      {
      bt = (struct skey1 *)malloc(sizeof(struct skey1));
      bt->idnum = idnum;
      bt->blocknum = blocknum;
      bt->index = index;
      bt->left = NULL;
      bt->right = NULL;
      }
   else
      if (idnum < bt->idnum)
	 bt->left = btinsertskey1(bt->left,idnum,blocknum,index);
      else
      if (idnum > bt->idnum)
	 bt->right = btinsertskey1(bt->right,idnum,blocknum,index);
   return(bt);
   }





struct skeylist *listinsert(list, blocknum, index)

/*********************************************************************************

 function listinsert
 Purpose: This function inserts an employee into the linked list pointed to by
	  one of the nodes of a secondary key binary tree (2,3,or 4).

 *********************************************************************************/

struct skeylist *list;
int blocknum, index;
   {
   if (list == NULL)
      {
      list  = (struct skeylist *)malloc(sizeof(struct skeylist));
      list->blocknum = blocknum;
      list->index = index;
      list->next = NULL;
      } 
   else
      list->next = listinsert(list->next, blocknum, index);
   return(list);
   }  





struct skey234 *btinsertskey234(bt, trait, blocknum, index)

/**********************************************************************************

 function btinsertskey234
 Purpose: This function inserts an employee into the secondary key binary tree
	  (2,3 and 4) and calls listinsert to insert data into the linked
	  list.

 **********************************************************************************/

struct skey234 *bt;
int trait, blocknum, index;
   {
   if (bt == NULL)
      {
      bt = (struct skey234 *)malloc(sizeof(struct skey234));
      bt->trait = trait;
      bt->list = NULL;
      bt->list = listinsert(bt->list,blocknum,index);
      bt->left = NULL;
      bt->right = NULL;
      }
   else
      if (bt->trait == trait)
         bt->list = listinsert(bt->list,blocknum,index);
      else

      if (trait < bt->trait)
	 bt->left = btinsertskey234(bt->left,trait,blocknum,index);
      else
      if (trait > bt->trait)
	 bt->right = btinsertskey234(bt->right,trait,blocknum,index);
   return(bt);
   }
	 



     
enteremp(buf,index,tempnode,over)

/*********************************************************************************

 function enteremp
 Purpose: This function enters an employee into the hash table(file), and all 
	  secondary keys by calling separate routines.

 *********************************************************************************/

struct emp buf[8];
int index;
struct emp tempnode;
int over;
   {
   int bnum;
   bnum = 1024 * blocknum+(1024*over);
   tempnode.tombstone = 0;
   buf[index] = tempnode;
   fseek(outfile,bnum,0); 
   fwrite(buf,128,8,outfile);

   /* insert into secondary binary trees */

   idnumbt = btinsertskey1(idnumbt,tempnode.idnum,bnum/1024,index);
   manlevelbt = btinsertskey234(manlevelbt,tempnode.manlevel, bnum/1024, index);
   salarybt = btinsertskey234(salarybt,  tempnode.salary, bnum/1024, index);
   numcatsbt = btinsertskey234(numcatsbt, tempnode.numcats, bnum/1024, index);
   }




btsearch(bt,idnum)

/**********************************************************************************

 function btsearch
 Purpose: This function returns a number 1 through parameter if the current idnum is
	  found in binary tree idnum.

 **********************************************************************************/

struct skey1 *bt;
int idnum;
   {
   if (bt != NULL)
      {
      btsearch(bt->left,idnum);
      if (bt->idnum == idnum)
	 skey1flag = 1;				/* GLOBAL */
      btsearch(bt->right,idnum);
      }
   }




changeindex() 

/**********************************************************************************

 function changeindex 
 Purpose: This function simply alters index to allow for linear probing.

 **********************************************************************************/

   {
   if (index == 0) index = 7;
      else --index;
   }




probeblock()

/**********************************************************************************

 function probeblock 
 Purpose: This function simply linear probes through a block and determines when
	  or if index shound be changed and updated.

 **********************************************************************************/

   {
   changeindex(); 
   while ((buffer[index].tombstone == 0) && (strcmp(buffer[index].chrstr,                                                             tempnode.chrstr) != 0) && (wall != index))
      changeindex();
   }





computeblockandindex(slotnum)

/*********************************************************************************

 function computeblockandindex
 Purpose: This function computes the block number and index into a block given
	  a slotnum.

 *********************************************************************************/

int slotnum;
   {
   blocknum = slotnum / 8;
   res = 8 * blocknum;
   index = slotnum - res;
   }




createoverflow() 

/*********************************************************************************

 function createoverflow
 Purpose: This function creates a new empty overflow block.         

 *********************************************************************************/

   {
   fseek(outfile,1024*blocknum+(1024*numoverflow),0); 
   fwrite(overflow,128,8,outfile);
   }




outemp(anemp)

/**********************************************************************************

 function outemp
 Purpose: This function outputs an employee following a search command.

 **********************************************************************************/

struct emp anemp;
   {
   printf("%s %s\n", "Name: ", anemp.chrstr);
   printf("%s %s\n", "Address: ", anemp.chrstr+strlen(anemp.chrstr)+1);
   printf("%s %d\n", "ID number: ", anemp.idnum);
   printf("%s %d\n", "Management level: ", anemp.manlevel);
   printf("%s %d\n", "Salary: ", anemp.salary);
   printf("%s %d\n", "Number of cats: ", anemp.numcats);
   printf("\n");
   }




searchskey1(bt, lbound, ubound)

/***********************************************************************************

 function searchskey1
 Purpose: This function checks and outputs all employees within the bounds from the
	  idnum bin tree following the search command.

 ***********************************************************************************/

struct skey1 *bt;
int lbound, ubound;
   {
   if (bt != NULL)
      {
      searchskey1(bt->left, lbound, ubound);
      if ((bt->idnum >= lbound) && (bt->idnum <= ubound))
         {
         fseek(outfile,1024*bt->blocknum,0); 
         fread(buffer,128,8,outfile);
         if (buffer[bt->index].idnum == bt->idnum)
	    outemp(buffer[bt->index]);
         }
      searchskey1(bt->right, lbound, ubound);
      }
   }




searchlist(list)

/**********************************************************************************

 function searchlist
 Purpose: This function outputs the employees in a linked list of a node from the
	  secondary key bin tree(2,3 or 4).

 **********************************************************************************/

struct skeylist *list;
   {
   struct skeylist *tempptr;
   tempptr = list;
   while (tempptr != NULL)
      {
      fseek(outfile,1024*tempptr->blocknum,0);
      fread(buffer,128,8,outfile);
      outemp(buffer[tempptr->index]);
      tempptr = tempptr->next;
      }
   }




searchskey234(bt, lbound, ubound)

/***********************************************************************************

 function searchskey234
 Purpose: This function searchess and outputs employees with the bounds given
	  from the secondary bin trees(2,3, or 4).

 ***********************************************************************************/

struct skey234 *bt;
int lbound, ubound;
   {
   if (bt != NULL)
      {
      searchskey234(bt->left, lbound, ubound);
      if ((bt->trait >= lbound) && (bt->trait <= ubound))
         searchlist(bt->list);      
      searchskey234(bt->right, lbound, ubound);
      } 
   }




getblocknum(bt,idnum)

/*********************************************************************************

 function getblocknum
 Purpose: This function searches the ID num. bin tree for the idnum sent in by
	  parameter, and sets blocknum and index(both global) if a match occurs.
	  (used for delete routine)

 *********************************************************************************/

struct skey1 *bt;
int idnum;
   {
   if (bt != NULL)
      {
      getblocknum(bt->left,idnum);
      if (bt->idnum == idnum)
	 {
	 blocknum = bt->blocknum;
	 index = bt->index;
	 }
      getblocknum(bt->right,idnum);
      } 
   }




delfromtable()

/***********************************************************************************

 function delfromtable
 Purpose: This function deletes an employee from the hash table(file) by using
	  a tombstone method.

 ***********************************************************************************/

   {
   fseek(outfile,1024*blocknum,0); 
   fread(buffer,128,8,outfile); 
   buffer[index].tombstone = 1;			/* employee slot # now empty */
   fseek(outfile,1024*blocknum,0); 
   fwrite(buffer,128,8,outfile);
   }




deletemin1(bt, idnum, blocknum, index) 

/***********************************************************************************

 function deletemin1
 Purpose: This routine, in conjunction with delete below, deletes a node in the 
	  ID num bin tree.

 ***********************************************************************************/

struct skey1 *bt;
int idnum;
   {
   struct skey1 *tempptr;
   if (bt->left == NULL)
      {
      tempptr = bt;
      bt->idnum = idnum;
      bt->blocknum = blocknum;
      bt->index = index;
      bt = bt->right;
      free(bt);
      }
   else
      deletemin1(bt->left,idnum,blocknum,index);
   }
 



delete1(bt, idnum)

/***********************************************************************************

 function delete1
 Purpose: This function is responsible for searching the ID num bin tree and
	  ultimately deleting the node with idnum(parameter) if it exists.

 ***********************************************************************************/

struct skey1 *bt;
int idnum;
   {
   if (bt != NULL)
      if (idnum < bt->idnum)
	 delete1(bt->left,idnum);
      else
      if (idnum > bt->idnum)
	 delete1(bt->right,idnum);
      else
      if (bt->left == NULL)
	 bt = bt->right;
      else
      if (bt->right == NULL)
	 bt = bt->left;
      else
	 deletemin1(bt->right, bt->idnum, bt->blocknum, bt->index);
   }




main()
{
infile = fopen("pgm4.dat","r");				/* input file of employees */ 
outfile = fopen("hash.out", "r+");			/* output hashing table */

for (j=0; j<=7; ++j)					/* set empty flags */
   {
   buffer[j].tombstone = 1;
   overflow[j].tombstone = 1;
   }
for (i=1; i<=27; ++i)					/* create empty table */
   fwrite(buffer,128,8,outfile);
currblock = -1;

while (!feof(infile)) 
{

fscanf(infile,"%s", command);
printf("%s %c",command, 32);

if (strcmp(command,"enter") == 0)			/* ENTER */ 
   {
   fgets(aline,80,infile);				/* read name & address */
   compress(aline,tempnode.chrstr); 			/* compress */
   printf("%s %c", tempnode.chrstr, 32);
   printf("%s\n",tempnode.chrstr+strlen(tempnode.chrstr)+2);
   fscanf(infile, "%d %d %d %d\n", &tempnode.idnum, &tempnode.manlevel, &tempnode.salary,                                   &tempnode.numcats);
   printf("%d %d %d %d\n", tempnode.idnum,tempnode.manlevel,tempnode.salary,                                        tempnode.numcats);
   printf("\n");
   slotnum = hash(tempnode.chrstr);			/* compute slot num */
   computeblockandindex(slotnum);			/* compute block, index */

   if (currblock != blocknum)				/* block not already in mem */
      {
      fseek(outfile,1024*blocknum,0);
      fread(buffer,128,8,outfile);
      currblock = blocknum; 
      }
   skey1flag = 0;
   btsearch(idnumbt, tempnode.idnum);		/* does emp. exist in idnum bt? */  
   if (skey1flag != 0)
      printf("Error: employee already exists with same ID Number\n");
   else
      {
      if ((buffer[index].tombstone) == 1)       /* name is new */
         enteremp(buffer,index,tempnode,0);
      else		                	/* must linear probe */
         {
         wall = index;
         probeblock();
         if (wall == index)			/* check overflow block(s) */
	    {
	    blocknum = 26;
	    if (numoverflow == 0)
	       {
	       ++numoverflow;
	       createoverflow();
	       }
	    for (i=1; i<=numoverflow; ++i)	/* check all overflow blocks */
	       {
	       fseek(outfile,1024*blocknum+(1024*numoverflow),0); 
	       /* fseek(outfile,1024*blocknum+((i-1)*1024),0); */
               fread(buffer,128,8,outfile); 
	       if ((buffer[index].tombstone) == 1)
		  enteremp(buffer,index,tempnode,numoverflow);
               else
	       {
               probeblock();
	       if (wall != index) break;	/* break if found open slot */
	       else 
		  {
		  ++numoverflow;
		  createoverflow();
		  }
	       }
	       }
            if (wall != index)			/* found open slot */
	       enteremp(buffer,index,tempnode,numoverflow);
            }
         else					/* found space at index */
	    if (strcmp(buffer[index].chrstr,tempnode.chrstr) != 0)
               enteremp(buffer,index,tempnode);
            else
	       printf("Error- duplicate primary key\n");
	   
         } 
      }
   }
if (strcmp(command,"search") == 0)			/* SEARCH */ 
   {
   fscanf(infile,"%d %d %d\n", &searchkey, &lbound, &ubound);
   printf("%c %d %d %d\n", 32, searchkey, lbound, ubound);
   printf("\n");
   if ((searchkey < 1) || (searchkey > 4))
      printf("Error- SEARCH: invalid secondary key index\n");
   else
   if (lbound > ubound)
      printf("Error- bounds are reversed\n");
   else
      {
      if (searchkey == 1)
         searchskey1(idnumbt, lbound, ubound);
      if (searchkey == 2)
	 searchskey234(manlevelbt, lbound, ubound);
      if (searchkey == 3)
	 searchskey234(salarybt, lbound, ubound);
      if (searchkey == 4)
	 searchskey234(numcatsbt, lbound, ubound);
      }
   }

if (strcmp(command,"delete") == 0)    			/* DELETE */ 
   {
   if (fscanf(infile,"%d %d\n", &delkey, &delidnum) == 2)  
      {
      if (delkey != 1)
         printf("Error- only ID num. key index allowed for first parameter\n");
      else
         {
         printf("%c %d %d\n", 32, delkey, delidnum);
         blocknum = -1;
         getblocknum(idnumbt, delidnum);
         if (blocknum == -1)
            printf("%s %d %s\n", "Error- DELETE: ID number: ", delidnum,                                                  " never encountered.");
         else
            { 
            delfromtable();
            delete1(idnumbt, delidnum);
            /* REMAINING BIN TREE DELETION NOT COMPLETED */
	    /* DELETE <name> not completed */
	    }
         }
      }
   }


}  /* feof */
}

