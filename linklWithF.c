
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node{
  char *book;
  double price;
  struct Node *next;  
} Node;

typedef struct {
  Node **bucket;
  int capacity;
  int count;
} HashTable;

size_t hashKey(HashTable *ht, const char *key){
  int tbz = ht->capacity;
  int primeBase = 31;
  size_t hashkey = 0;
  while(*key){
    hashkey  = (hashkey * 31 + (*key++)) % tbz;
  }
  return hashkey;
}

HashTable* createHashTable(int inCp){
  HashTable *ht = malloc(sizeof(HashTable));
  if(ht == NULL){
    perror("Memory allocation failed!\n");
    exit(1);
  }
  ht->capacity = inCp;
  ht->count = 0;
  ht->bucket = calloc(ht->capacity, sizeof(Node*));
}

Node* search(HashTable *ht, char *key){
  size_t index = hashKey(ht, key);
  Node *curr = ht->bucket[index];
  while(curr){
    if(strcmp(curr->book, key) == 0)
      return curr;
    curr = curr->next;
  }
  return NULL;
}

void insert(HashTable *ht, char *key, double p);

void resizeHT(HashTable *ht){
  int newCapacity = ht->capacity * 2;
  Node **oldBuckets = ht->bucket;
  int oldsize = ht->capacity;

  ht->bucket = calloc(newCapacity, sizeof(Node*));
  ht->capacity = newCapacity;
  ht->count = 0;
  int i = 0;
  while(oldsize > i){
    Node *curr = oldBuckets[i];
    while(curr){
      insert(ht, curr->book, curr->price);
      Node *temp = curr;
      curr = curr->next;
      free(temp->book);
      free(temp);
    }
    i++;
  }
  free(oldBuckets);
}

void insert(HashTable *ht, char *key, double p){
  double loadFactor = (double) (ht->count + 1) / ht->capacity;
  if(loadFactor > 0.7){
    resizeHT(ht);
  }
  size_t index = hashKey(ht, key);
  Node *newNode = malloc(sizeof(Node));
  newNode->book = strdup(key);
  newNode->price = p;
  newNode->next = ht->bucket[index];
  
  ht->bucket[index] = newNode;
  ht->count++;
  printf("Done.\n");
}

void delete(HashTable *ht, const char *key){
  size_t index = hashKey(ht, key);
  Node *curr = ht->bucket[index];
  Node *prev = NULL;

  while(curr){
    if(strcmp(curr->book, key) == 0){
      if(prev == NULL){
	ht->bucket[index] = curr->next;
       
      }else{
	prev->next = curr->next;
      }
      printf("Delete done.\n");
      free(curr->book);
      free(curr);
      ht->count--;
      return;
    }
    prev = curr;
    curr = curr->next;
  }
  printf("Book \"%s\" not found!\n", key);
}

void saveTable(HashTable *ht, FILE *f){
  if(!ht || !f) return;
  fwrite(&ht->capacity, sizeof(int), 1, f);
  fwrite(&ht->count, sizeof(int), 1, f);

  for(int i = 0; i < ht->capacity; i++){
    Node *curr = ht->bucket[i];
    while(curr){
      size_t len = strlen(curr->book) + 1;
      fwrite(&len, sizeof(size_t), 1, f);
      fwrite(curr->book, sizeof(char), len, f);
      fwrite(&curr->price, sizeof(double), 1, f);
      curr = curr->next;
    }
  }
  printf("Fail has been saved!\n");
}

HashTable* loadFile(FILE *f){
  if(!f) return NULL;
  
  int capacity, count;
  fread(&capacity, sizeof(int), 1, f);
  fread(&count, sizeof(int), 1, f);

  HashTable *ht = createHashTable(capacity);

  printf("File is reading.\n");

  for(int i  = 0; i < count; i++){
    size_t len;
    fread(&len, sizeof(size_t), 1, f);

    char *book = malloc(len);
    fread(book, sizeof(char), len, f);

    double price;
    fread(&price, sizeof(double), 1, f);

    insert(ht, book, price);
    free(book);
  }
  return ht;
}

void freeHashTable(HashTable *ht) {
    for (int i = 0; i < ht->capacity; i++) {
        Node *curr = ht->bucket[i];
        while (curr) {
            Node *temp = curr;
            curr = curr->next;
            free(temp->book);
            free(temp);
        }
    }
    free(ht->bucket);
    free(ht);
}

int main(){

  FILE *fp = fopen("Linklist.dat", "wb");
  
  HashTable *ht = createHashTable(7);
  insert(ht, "CSE", 300.40);
  insert(ht, "Math", 120.79);

  saveTable(ht, fp);
  fclose(fp);
  freeHashTable(ht);

  fp = fopen("Linklist.dat", "rb");
  HashTable *loaded = loadFile(fp);
  fclose(fp);
  
  Node *n = search(loaded, "CSE");
  if(n) printf("Found: %s and price : %.2f\n", n->book, n->price);
 
  freeHashTable(loaded);
  
  return 0;
}
