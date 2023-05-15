#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static int nExtra = 5;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

struct Node {
  struct Node *previous;
  struct Node *next;
  int value;
};

struct List {
  struct Node *head;
  struct Node *tail;
  struct Node *left;
  struct Node *right;
  int size; //size of free space available
};


void insert_space(struct List *s){
    struct Node *previous_node = s->left;
    struct Node *end_node = s->left->next;
    for(int i = 0;i < nExtra;i++){
      struct Node *new_node = malloc(sizeof(struct Node));
      new_node->value = -1;
      new_node->previous = previous_node;
      previous_node->next = new_node;
      previous_node = new_node;
    }//for
    s->size = s->size + nExtra;
    previous_node->next = end_node;
    end_node->previous = previous_node;
}

void free_space(struct List *s){
    struct Node *cur_node = s->left->next;
    struct Node *next_node;
    for(int i = 0; i < nExtra; i++){
      next_node = cur_node->next;
      free(cur_node);
      cur_node = next_node;
    }//for
    s->left->next = cur_node;
    cur_node->previous = s->left;
    s->size = s->size - nExtra;
}

void push(struct List *s, int i){
  if(s->size <= 0){
    insert_space(s);
  }
  if (i%2 == 1) {
    s->left = s->left->next;
    s->left->value = i;
  }//if
  else{
    s->right = s->right->previous;
    s->right->value = i;
  }//else
  s->size--;
}

int pull(struct List *s, int isLeft){
  int ifLeft = s->left != s->head;
  int ifRight = s->right != s->tail;
  if ((isLeft || !ifRight) && ifLeft){
    s->left->value = -1;
    s->left = s->left->previous;
  }
  else if((!isLeft || !ifLeft) && ifRight){
    s->right->value = -1;
    s->right = s->right->next;
  }
  else{
    printf("warning: no nodes to delete!\n");
    return 1;
  }//iffe
  s->size++;
if(s->size > nExtra){
  free_space(s);
}
  return 0;
}

void free_list(struct List *s){
  struct Node *cur_node = s->head;
  struct Node *next_node;
  s->head = s->tail = s->left = s->right = 0;
  while(cur_node){
    next_node = cur_node->next;
    free(cur_node);
    cur_node = next_node;
  }//while
}

void print_list(struct List *s){
  printf("[");
  struct Node *cur_node = s->head;
  while (cur_node->next) {
    printf("%2d, ", cur_node->value);
    cur_node = cur_node->next;
  }//while
  printf("%2d]\n", cur_node->value);
}

void* pushIntegers(void *s){
  for(int i = 0; i < 15; i++){
    int v = 100 * ((float)rand()) / ((float)RAND_MAX);
    pthread_mutex_lock(&m);
    push(s, v);
    print_list(s);
    pthread_mutex_unlock(&m);
  }
  return NULL;
}
//by using the examples I noticed that the turns when the warning"warning: no nodes to delete!\n"
//were not counted, and thus decided to make my for loop not count those iterations, this does cause
//some really long streaks of failed pulls, this is intentional but since the task did not make this clear
// I did not want to take risks
void* pullIntegers(void *s){
  for(int i = 0; i < 15; i++){
    int v = 100 * ((float)rand()) / ((float)RAND_MAX);
    v = (v + 1) % 2;
    pthread_mutex_lock(&m);
    int x = pull(s, !v);
    i = i - x;
    print_list(s);
    pthread_mutex_unlock(&m);
  }
  return NULL;
}

int main(){
  srand(getchar());
  struct List s;
  s.head = malloc(sizeof(struct Node));
  s.tail = malloc(sizeof(struct Node));
  s.head->value = 0;
  s.tail->value = 0;
  s.head->next = s.tail;
  s.tail->previous = s.head;
  s.head->previous = NULL;
  s.tail->next = NULL;
  s.left  = s.head;
  s.right = s.tail;
  s.size = 0;
  pthread_t pushThread;
  pthread_t pullThread;
  pthread_create(&pushThread, NULL, pushIntegers, &s);
  pthread_create(&pullThread,NULL, pullIntegers, &s);
  pthread_join(pushThread, NULL);
  pthread_join(pullThread, NULL);
  pull(&s,0);
  free_list(&s);
}
