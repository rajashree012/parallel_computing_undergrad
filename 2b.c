
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <mpi.h>
#include <memory.h>
#define MAXCITIES 25
#define MSGSIZE 100
///////////////////////////////////////////////////////////////////////////
#define TRUE_ 1
#define FALSE_ 0
 
typedef struct { void * data; int pri; } q_elem_t;
typedef struct { q_elem_t *buf; int n, alloc; } pri_queue_t, *pri_queue;
 
#define priq_purge(q) (q)->n = 1
#define priq_size(q) ((q)->n - 1)
/* first element in array not used to simplify indices */
pri_queue priq_new(int size)
{
	if (size < 4) size = 4;
 
	pri_queue q = malloc(sizeof(pri_queue_t));
	q->buf = malloc(sizeof(q_elem_t) * size);
	q->alloc = size;
	q->n = 1;
 
	return q;
}
 
void priq_push(pri_queue q, void *data, int pri)
{
	q_elem_t *b;
	int n, m;
 
	if (q->n >= q->alloc) {
		q->alloc *= 2;
		b = q->buf = realloc(q->buf, sizeof(q_elem_t) * q->alloc);
	} else
		b = q->buf;
 
	n = q->n++;
	/* append at end, then up heap */
	while ((m = n / 2) && pri < b[m].pri) {
		b[n] = b[m];
		n = m;
	}
	b[n].data = data;
	b[n].pri = pri;
}
 
/* remove top item. returns 0 if empty. *pri can be null. */
void * priq_pop(pri_queue q, int *pri)
{
	void *out;
	if (q->n == 1) return 0;
 
	q_elem_t *b = q->buf;
 
	out = b[1].data;
	if (pri) *pri = b[1].pri;
 
	/* pull last item to top, then down heap. */
	--q->n;
 
	int n = 1, m;
	while ((m = n * 2) < q->n) {
		if (m + 1 < q->n && b[m].pri > b[m + 1].pri) m++;
 
		if (b[q->n].pri <= b[m].pri) break;
		b[n] = b[m];
		n = m;
	}
 
	b[n] = b[q->n];
	if (q->n < q->alloc / 2 && q->n >= 16)
		q->buf = realloc(q->buf, (q->alloc /= 2) * sizeof(b[0]));
 
	return out;
}
 
/* get the top element without removing it from queue */
void* priq_top(pri_queue q, int *pri)
{
	if (q->n == 1) return 0;
	if (pri) *pri = q->buf[1].pri;
	return q->buf[1].data;
}
 
/* this is O(n log n), but probably not the best */
void priq_combine(pri_queue q, pri_queue q2)
{
	int i;
	q_elem_t *e = q2->buf + 1;
 
	for (i = q2->n - 1; i >= 1; i--, e++)
		priq_push(q, e->data, e->pri);
	priq_purge(q2);
}

 

enum {PUT_PATH_TAG, BEST_PATH_TAG, GET_PATH_TAG, UPDATE_BEST_PATH_TAG, 
      DONE_TAG, REPLY_PATH_TAG};




struct path {
  int length;			// Current length of partial path
  int city[25];
  int visited;			// Number of cities in the partial path


};


 void Path(struct path* p);			// Initialize a Path with city 0 visited
 void Set (int len, int *cit, int vis,struct path* p);  // Sets visited, length and city
 void Print(struct path* p);

int myrank, NumProcs, NumCities;

int Dist[100]={0};

void Path (struct path* p)
{ int i;
  p->length=0; 
  p->visited=1;
  for (i=0; i<NumCities; i++) p->city[i]=i;
}

void Set (int len, int *cit, int vis,struct path* p)
{
  p->length = len;
  memcpy (p->city, cit, NumCities*sizeof(int));
 p->visited = vis;
}

void Print(struct path* p)
{int i;
int vis=p->visited;
  for (i=0; i<vis; i++) 
     printf("  %d", p->city[i]);
  printf("; length = %d\n", p->length);
}

//////////////////////////////////////////////////////////////////////////////////

void Fill_Dist()
{
int i;
int j;

  if (myrank == 0)    // I'll read the matrix
     // actualy, you must redirect the input to a file
     // I'll not print any message, first it reads the number of cities
     {// and then it reads the matrix
     
     scanf("%d", &NumCities);}

  // global operation, all processes must call it
  MPI_Bcast( &NumCities, 1, MPI_INT, 0, MPI_COMM_WORLD);


printf("enter");
  if (myrank == 0)
     for(  i = 0 ; i<NumCities ; i++ )
        for( j = 0 ; j<NumCities ; j++ )
           scanf("%d", &Dist[i*NumCities + j]);
  
  // global operation, all processes must call it
  MPI_Bcast( Dist,                   // the buffer
             100,    // number of elements
             MPI_INT,                // type of elements
             0,                      // the root for the broadcast
             MPI_COMM_WORLD);        // the most used communicator
  
  if (myrank == 0)        // print the matrix for debugging
  {
     printf("Number of cities: %d\n", NumCities);
     for( i = 0 ; i<NumCities ; i++ )
     {
        for( j=0 ; j<NumCities ; j++ )
           printf("%5d", Dist[i*NumCities+j] );
        printf("\n");
     }
  }
}
void Coordinator ()
{

  MPI_Status status;
 // Msg_t msg;
 struct path Shortest;
struct path *P_ptr,*P_ptr1;
 struct path P; 
  // To keep track of processes that are waiting for a Path
  int waiting[100];     // to save ranks
  int nwait = 0;	                // how many are waiting
  int bpath = 0;                        // just count the number of best
  int i;                                      // path received
  int temp;
  int msg[100];
pri_queue queue = priq_new(0);
 printf("Coord started ...\n"); fflush(stdout);

Path(&P);

priq_push(queue,&P, 0);

 
	       // initialize queue with the first task
                               // one zero-length path
  Shortest.length = INT_MAX;   // The initial Shortest path must be bad

 
  while (nwait < NumProcs-1) 
  {
    MPI_Recv (msg, MSGSIZE, MPI_INT, MPI_ANY_SOURCE, 
	      MPI_ANY_TAG, MPI_COMM_WORLD, &status); 

    switch (status.MPI_TAG) 
    {
      case BEST_PATH_TAG: 
	if (msg[0] < Shortest.length)
	{
           bpath ++;
           printf("Got best path %d, source = %d, length = %d\n", 
                bpath, status.MPI_SOURCE, msg[0]);
           fflush(stdout);

	   // update Shortest:
           Shortest.length=msg[0];
           Shortest.visited=NumCities;
           i=0;
           while(i<NumCities)
          {
          Shortest.city[i]=msg[i+2];
          i++;
          }
           temp=msg[0];
           for(  i = 1 ; i<NumProcs ; i++ )
              MPI_Send( &(temp), 1, MPI_INT, i, 
                UPDATE_BEST_PATH_TAG, MPI_COMM_WORLD );
        }
        break;

      case PUT_PATH_TAG:
	if (nwait>0) 
	{
	  printf(" Don't put path into queue; send it to one waiting process\n");
          i=waiting[--nwait];
	  MPI_Send (msg, MSGSIZE, MPI_INT, i,
		    REPLY_PATH_TAG, MPI_COMM_WORLD);
	} 
	else 
	{
	  P_ptr=(struct path*)malloc(sizeof(struct path));
Path(P_ptr);
	  P_ptr->length=msg[0];
          P_ptr->visited=msg[1];
         
            i=0;
	  while(i<NumCities)
          {
          P_ptr->city[i]=msg[i+2];
          i++;
          }
         priq_push(queue, P_ptr, msg[0]);
	//printf("%d--push--\n",P.length);
    
    
	}
	break;

      case GET_PATH_TAG:
	if ( (P_ptr=priq_pop(queue, &i))!=0) 
	{
	
         
	  
	  msg[0] = P_ptr->length;
   
          i=0;
	  while(i<NumCities)
          {
          msg[i+2]=P_ptr->city[i];
          i++;
          }
	  msg[1] = P_ptr->visited;
	  MPI_Send (msg, MSGSIZE, MPI_INT, status.MPI_SOURCE, 
		    REPLY_PATH_TAG, MPI_COMM_WORLD);
   
	}
	else 
	{
	   waiting[nwait++] = status.MPI_SOURCE;

	  if (nwait==NumProcs-1)
	   for ( i=1; i<NumProcs; i++)
	      MPI_Send (NULL, 0, MPI_INT, i, DONE_TAG, MPI_COMM_WORLD);
	}
	break;
    }
  }
  printf("Shortest path:\n");
  Print(&Shortest);
}




void Worker ()
{ 
  MPI_Status status;
  int msg[100];
  int shortestLength = INT_MAX;
int d1,d2;
int length;
int i,tmp,d;
  printf("Worker started ...\n"); 

 MPI_Send (NULL, 0, MPI_INT, 0, GET_PATH_TAG, MPI_COMM_WORLD);

  while (1) 
  {
    MPI_Recv (msg, MSGSIZE, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    if (status.MPI_TAG==DONE_TAG) 
    {
       printf("Worker %d received DONE_TAG ..\n", myrank);

       break; // we're done
    }
    
    if (status.MPI_TAG==UPDATE_BEST_PATH_TAG)
    {  printf("updating");
       shortestLength = msg[0];    // we sent just one INT
                                       // length is the first field in msg
       continue;
    }


    // we must have a REPLY_PATH_TAG
    msg[1]++;
    if (msg[1]==NumCities) 
    {
      d1 = Dist[ (msg[NumCities-2+2])*NumCities + msg[NumCities-1+2] ];
      d2 = Dist[(msg[NumCities-1+2]) * NumCities ];
      if (d1 * d2)    // both edges exist
      { 
         msg[0] += d1 + d2;
      
        
         if (msg[0] < shortestLength)
	    MPI_Send (msg, MSGSIZE, MPI_INT, 0, BEST_PATH_TAG, MPI_COMM_WORLD);
      }
      // not a valid path, ask for another partial path
    }
    else 
    {
      // For each city not yet visited, extend the path:
      // (I'll use the same msg space to compute every extended the path)
       length = msg[0];
      for ( i=msg[1]-1; i<NumCities; i++) 
      {

	// swap city[i] and city[visted-1]
	if (i > msg[1]-1)
	{
	   tmp = msg[msg[1]-1+2];
	   msg[msg[1]-1+2] = msg[i+2];
	   msg[i+2] = tmp;
	}
      
	// visit city[visited-1]
	if (d = Dist[(msg[msg[1]-2+2])*NumCities + msg[msg[1]-1+2] ])
	  {
	    msg[0] = length + d;
	    if (msg[0] < shortestLength)
	      MPI_Send (msg, MSGSIZE, MPI_INT, 0, PUT_PATH_TAG, MPI_COMM_WORLD);
	  }
      }
    }
    MPI_Send (NULL, 0, MPI_INT, 0, GET_PATH_TAG, MPI_COMM_WORLD);
  }
}



int main(int argc, char *argv[])
{
  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &NumProcs);
 
  if (NumProcs<2) {
    printf("At least 2 processes are required\n");
    exit(-1);
  }  


  // Initialize distance matrix. Ususally done by one process 
  // and bcast, or initialized from a file in a shared file system.
  Fill_Dist();  // process 0 read the data and broadcast it to the others

  if (myrank==0) 
  { Coordinator();}
 else
    Worker();

  MPI_Finalize();
  return 0;
}


