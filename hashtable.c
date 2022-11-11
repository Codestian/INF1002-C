#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chat1002.h"


unsigned int hash_function(char *key, size_t len){ 
    /*Jenkins one at a time hash function. Used to creates keys which are mapped to values. 
    Good hash functions have low collision rates and JOAT hash function is one of the simplest/low-collision chance
    function to implement. Keys are encrypted values which are mapped to values to identify them.*/
    unsigned int hash, i;
    for(hash = i = 0; i < len; ++i)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

/*Method of handling collision for this hash table is through separate chaining. This means that whenever there is a collision,
we add items that collide on the same index to the overflow bucket which is basically a linked list.*/

static LinkedList* allocate_memory_list () {
    // Allocates memory for a Linkedlist pointer
    LinkedList* list = (LinkedList*) malloc (sizeof(LinkedList)); //Allocate memory for Linkedlist.
    return list;
}

static LinkedList* linkedlist_insert(LinkedList* list, Node* item) {
    // Inserts the item onto the Linked List
    if (!list) { //Create a linkedlist at memory address if it doesn't exist.
        LinkedList* head = allocate_memory_list(); //Allocate memory for Linkedlist obj to be inserted at memory address with collision.
        head->item = item; //Set item of head to be the item/Node which is supposed to be stored at collision address.
        head->next = NULL; //Set memory address of next item in linkedlist obj to null.
        list = head; 
        return list;
    }

    else if (list->next == NULL) { //If there is only 1 linkedlist obj at memory address.
        LinkedList* secondnode = allocate_memory_list(); //Allocate memory for Linkedlist obj to be inserted at memory address with collision.
        secondnode->item = item; //Set item of fnode to be the item/Node which is supposed to be stored at collision address.
        secondnode->next = NULL; //Set memory address of next item in linkedlist to null.
        list->next = secondnode; //Set next item of first linkedlist obj to be current linkedlist obj.
        return list;
    }

    LinkedList* temp = list; 
    while (temp->next->next) {
        temp = temp->next; //Iterates through linkedlist to its last item.
    }

    LinkedList* node = allocate_memory_list(); //Allocate memory for Linkedlist obj to be inserted at memory address with collision.
    node->item = item; //Set item of nnode to be the item/Node which is supposed to be stored at collision address.
    node->next = NULL; //Set memory address of next item in linkedlist to null.
    temp->next = node; //Set next item of previous linkedlist obj to be current linkedlist obj.

    return list; 
}

static void free_linkedlist(LinkedList* list) {
    //Removes linkedlist object.
    LinkedList* temp = list; 
    /*While there are still items in list/while list is not null, free up all items and its attributes in linkedlist*/
    while (list) { 
        temp = list; 
        list = list->next;
        free(temp->item->key);
        free(temp->item->entity);
        free(temp->item->intent);
        free(temp->item->responses);
        free(temp->item);
        free(temp);
    }
}

static LinkedList** create_overflow_buckets(HashTable* table) {
    /*Create the overflow buckets; an array of linkedlists. 
    Each node/item in hashtable will have its own overflow bucket thus you will need to set aside
    tablesize*linkedlist of memory space. */
    LinkedList** buckets = (LinkedList**) calloc (table->size, sizeof(LinkedList*)); //Allocate memory for overflow bucket and set all values in memory to 0
    for (int i=0; i<table->size; i++) 
        buckets[i] = NULL; //for each bucket set its value to be NULL. Which means the bucket contains nothing.
    return buckets;
}

static void free_overflow_buckets(HashTable* table) {
    // Free all the overflow bucket lists
    LinkedList** buckets = table->obuckets;
    for (int i=0; i<table->size; i++){
        if (buckets[i] == NULL) continue;
        free_linkedlist(buckets[i]);
    }
    free(buckets);
}


Node* create_item(char* key, const char* intent, const char* entity, const char* responses){
    // Creates a pointer to a new hash table item
    Node* item = (Node*) malloc (sizeof(Node));
    item->key = (char*) malloc (strlen(key) + 1);
    item->intent = (char*) malloc (strlen(intent) + 1);
    item->entity = (char*) malloc (strlen(entity) + 1);
    item->responses = (char*) malloc (strlen(responses) + 1);
    strcpy(item->key, key);
    strcpy(item->intent, intent);
    strcpy(item->entity, entity);
    strcpy(item->responses, responses);
    return item;
}

HashTable* create_table(int size) {
    // Creates a new HashTable
    HashTable* table = (HashTable*) malloc (CAPACITY * sizeof(HashTable)); //Allocate memory for hashtable. Capacity and size is same.
    table->size = size; //Set size of hashtable to be capacity
    table->count = 0; //Set number of items in hashtable to be 0.
    table->items = (Node**) calloc (table->size, sizeof(Node*)); //Allocate memory space for items in hashtable.
    for (int i=0; i<table->size; i++){ //Set value of items in hashtable to be NULL.
        table->items[i] = NULL; 
    }
    table->obuckets = create_overflow_buckets(table); //Create overflow bucket of hashtable.

    return table;
}

void free_item(Node* item) {
    // Frees an item
    free(item->key);
    free(item->intent);
    free(item->entity);
    free(item->responses);
    free(item);
}

void free_table(HashTable* table) {
    if (table == NULL) return;
    // Frees the table which is used to reset the chatbot.
    for (int i=0; i<table->size; i++) { //For each item in the hashtable, free its memory if there is are values in the it.
        Node* item = table->items[i];
        if (item != NULL)
            free_item(item);
    }
    
    free_overflow_buckets(table); //free overflow bucket
    free(table->items); //free memory space allocated for table items.
    free(table); //free the table.
}

void handle_collision(HashTable* table, unsigned long index, Node* item) {
    LinkedList* head = table->obuckets[index]; //Set head to be overflow bucket at memory address with collision.

    if (head == NULL) { 
        /*if head is empty/null, allocate memory for a linkedlist and set value of its item to be node that is to be stored at address.*/
        head = allocate_memory_list(); 
        head->item = item;
        table->obuckets[index] = head;
        return;
    }
    else {
        // Insert to the list
        table->obuckets[index] = linkedlist_insert(head, item);
        return;
    }
 }

int ht_insert(HashTable* table, char* key, const char* intent, const char* entity, const char* response) {
    Node* item = create_item(key, intent, entity, response); //Create item to be inserted
    unsigned long index = hash_function(key, strlen(key)) % CAPACITY; //Calculate index/key of item
    Node* current_item = table->items[index]; //Assign item to a variable current item.

    if (current_item == NULL) { 
        /*If current item not found and table is full then return. Else insert item to into the table and increase count.*/
        if (table->count == table->size) {
            printf("Hash Table is full\n");
            free_item(item); //Remove item
            return 0;
        }
        table->items[index] = item; //Add item into hashtable.
        table->count++; //Increase count.
        return 1; 
    }

    else {
            if (strcmp(current_item->key, key) == 0) { 
                /*If current item is exist and key is the same, then replace values of existing item.*/
                strcpy(table->items[index]->intent, intent); //Replace intent of exisiting item
                strcpy(table->items[index]->entity, entity); //Replace intent of exisiting item
                strcpy(table->items[index]->responses, response); //Replace intent of exisiting item
                return 1;
            }

        else {
            // Scenario 2: Collision
            handle_collision(table, index, item); //If there is already item in memory address and key is different then handle collision.
            return 1;
        }
    }
}

Node* ht_search(HashTable* table, char* key) {
    /*Search for key in hashtable.*/
    unsigned long index = hash_function(key, strlen(key)) % CAPACITY; //Calculate index/key of item
    Node* item = table->items[index]; //Set Node to be item at index of hashtable.
    LinkedList* head = table->obuckets[index]; //Set head of overflow bucket at item at index.

    while (item != NULL) { 
        /*While there are items in the hashtable. Compare item key with search key and if its the same then retrieve the item. 
        */ 
        if (strcmp(item->key, key) == 0) //If key at index is equals to search key then return item.
            return item;
        if (head == NULL){ //Else if key is not the same and there is no overflow bucket/linkedlist at index then return null.
            return NULL;
        }
        /*if there is overflow bucket then set item to be next item in linkedlist*/
        item = head->item; 
        head = head->next;
    }
    return NULL;
}

void ht_delete(HashTable* table, char* key) {

    unsigned long index = hash_function(key, strlen(key)) % CAPACITY; //Calculate index/key of item
    Node* item = table->items[index]; //Set Node to be item at index of hashtable.
    LinkedList* head = table->obuckets[index]; //Set head of overflow bucket at item at index.

    if (item == NULL) { //If item does not exist then return.
        return;
    }
    else {
        if (head == NULL && strcmp(item->key, key) == 0) {
            /*If not collison at index then set value of item to be null, free up its memory space, and decrease count*/
            table->items[index] = NULL;
            free_item(item);
            table->count--;
            return;
        }
        else if (head != NULL) {
            if (strcmp(item->key, key) == 0) {
            /*If there is collision at index and key of firstitem in the collisionn chain is the same then remove it.
            Set next item in collision chain/linkedlist to be the head*/
                free_item(item);
                LinkedList* node = head; 
                head = head->next;
                node->next = NULL;
                table->items[index] = create_item(node->item->key, node->item->intent, node->item->entity, node->item->responses);
                free_linkedlist(node);
                table->obuckets[index] = head;
                return;
            }

            LinkedList* curr = head;
            LinkedList* prev = NULL;

            while (curr) {
                /*If the item to be deleted lies somewhere in the linkedlist then iterate through it to find the item and remove it.*/
                if (strcmp(curr->item->key, key) == 0) { 
                    if (prev == NULL) { 
                        /*If key in linkedlist is same as search key and item is first in linkedlist/chain then remove item and set
                        value of linkedlist to be null to remove it. basically removing overflow bucket.*/
                        free_linkedlist(head); 
                        table->obuckets[index] = NULL;
                        return;
                    }
                    else {
                        /*Else If key in linkedlist is same as search key and item is not first in linkedlist/chain and set next address of previous
                        item to be the address of the next item of current address. Then remove the item in current address.*/
                        prev->next = curr->next;
                        curr->next = NULL;
                        free_linkedlist(curr);
                        table->obuckets[index] = head;
                        return;
                    }
                }
                curr = curr->next;
                prev = curr;
            }

        }
    }
}


