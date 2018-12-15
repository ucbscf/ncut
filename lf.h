struct ITEM
  {  struct ITEM *next;
     int type;
     int start;
     int finish; 
     int inc;
     char *sep; } ;

struct LINE
  {  struct LINE *next;      /* to next line request */
     struct ITEM *itemhead;  /* to this request */
     struct ITEM *head;      /* to col/field requests */
  }  ;
 
