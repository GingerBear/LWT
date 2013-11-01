#include "lwt.h"  
#include "queue.h"  
#include "fatal.h"  
  
typedef struct QNode  
{  
  ElementType *  Element;  
  struct QNode *Next;  
}QNode, *QNodePtr;  
  
struct Node {  
  QNodePtr Front;  
  QNodePtr Rear;  
};  
  
int  
IsEmpty( Queue Q )  
{  
  return Q->Front == Q->Rear;  
}  
  
Queue  
CreateQueue( void )  
{  
  Queue Q;  
  Q = malloc( sizeof( struct Node ) );  
  Q->Front = Q->Rear = malloc( sizeof( struct QNode ) );  
  if (!Q->Front)  
    FatalError( "Out of space!!!");  
  Q->Front->Next = NULL;  
  return Q;  
}  
  
void  
MakeEmpty( Queue Q )  
{  
  if( Q == NULL )  
    Error( "Must use CreateQueue first" );  
  else  
    while( !IsEmpty( Q ) )  
      Dequeue( Q );  
}  
  
void  
DisposeQueue( Queue Q )  
{  
  while( Q->Front ) {  
    Q->Rear = Q->Front->Next;  
    free( Q->Front );  
    Q->Front = Q->Rear;  
  }  
  printf( "\nDispose queue completed!!!" );  
}  
void  
Enqueue( ElementType *  X, Queue Q )  
{  
  QNodePtr p;  
  p = malloc( sizeof( QNode ) );  
  if (!p)  
    FatalError( "Out of space!!!" );  
  p->Element = X;  
  p->Next = NULL;  
  Q->Rear->Next = p;  
  Q->Rear = p;  
}  
  
ElementType *   
Front( Queue Q )  
{  
  if ( !IsEmpty( Q ) )  
    return Q->Front->Next->Element;  
  return 0; /* Return value used to avoid warning */  
}  
  
void  
Dequeue( Queue Q )  
{  
  if ( !IsEmpty( Q ) )  
  {  
    QNodePtr p;  
    p = malloc( sizeof( QNode ) );  
    if (!p)  
      FatalError( "Out of space!!!" );  
    p = Q->Front->Next;  
    Q->Front->Next = p->Next;  
    if ( Q->Rear == p )  
      Q->Rear = Q->Front;  
    free( p );  
  }  
}  
  
ElementType *   
FrontAndDequeue( Queue Q )  
{  
  if ( !IsEmpty( Q ) )  
  {  
    QNodePtr p;  
    p = malloc( sizeof( QNode ) );  
    if (!p)  
      FatalError( "Out of space!!!" );  
    p = Q->Front->Next;  
    ElementType *  temp = NULL;  
    temp = p->Element;  
    Q->Front->Next = p->Next;  
    if ( Q->Rear == p )  
      Q->Rear = Q->Front;  
    free( p );  
    return temp;  
  }  
  Error( "Empty queue!!!" );  
  return 0; /* Return value used to avoid warning */  
} 