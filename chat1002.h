/*
 * INF1002 (C Language) Group Project.
 *
 * This file contains the definitions and function prototypes for all of
 * features of the INF1002 chatbot.
 */

#ifndef _CHAT1002_H
#define _CHAT1002_H
#define CAPACITY 1001 // Size of the Hash Table
#include <stdio.h>

/* the maximum number of characters we expect in a line of input (including the terminating null)  */
#define MAX_INPUT    256

/* the maximum number of characters allowed in the name of an intent (including the terminating null)  */
#define MAX_INTENT   32

/* the maximum number of characters allowed in the name of an entity (including the terminating null)  */
#define MAX_ENTITY   64

/* the maximum number of characters allowed in a response (including the terminating null) */
#define MAX_RESPONSE 256

/* return codes for knowledge_get() and knowledge_put() */
#define KB_OK        0
#define KB_NOTFOUND -1
#define KB_INVALID  -2
#define KB_NOMEM    -3

/* Custom names for the chatbot and end user */
#define BOT_NAME "Chatbot"
#define USER_NAME "User"

/* Delimiters for splitting input to words */
const char *delimiters = " ?\t\n";

typedef struct node_struct Node; //Create a data structure Node to store key, intent, entity, responses from user.
struct node_struct {
char *key; //Stores key which is used to search for a particular node.
char *intent; //Stores intent
char *entity; //Stores entity name
char *responses; //Stores responses
};

typedef struct LinkedList LinkedList; //Create LinkedList data structure to handle collisions from hashtable.
struct LinkedList {
    Node* item; //Stores a Node pointer.
    LinkedList* next; //Stores address of next item in the linkedlist.
};

typedef struct HashTable HashTable; //Hashtable data structure. Hashtable is a array of pointers, makes it easy to search up nodes.
struct HashTable{
    Node** items; //Pointer to a Node Pointer.
    int size; //Size of the hash table.
    int count; //Number of items in hashtable
    LinkedList** obuckets; //Stores linkedlist in case of collision.
};

/* functions defined in main.c */
int compare_token(const char *token1, const char *token2);
void prompt_user(char *buf, int n, const char *format, ...);

/* functions defined in chatbot.c */
const char *chatbot_botname();
const char *chatbot_username();
int chatbot_main(int inc, char *inv[], char *response, int n);
int chatbot_is_exit(const char *intent);
int chatbot_do_exit(int inc, char *inv[], char *response, int n);
int chatbot_is_load(const char *intent);
int chatbot_do_load(int inc, char *inv[], char *response, int n);
int chatbot_is_question(const char *intent);
int chatbot_do_question(int inc, char *inv[], char *response, int n);
int chatbot_is_reset(const char *intent);
int chatbot_do_reset(int inc, char *inv[], char *response, int n);
int chatbot_is_save(const char *intent);
int chatbot_do_save(int inc, char *inv[], char *response, int n);

/* functions defined in knowledge.c */
int knowledge_get(const char *intent, const char *entity, char *response, int n);
int knowledge_put(const char *intent, const char *entity, const char *response);
void knowledge_reset();
int knowledge_read(FILE *f);
void knowledge_write(FILE *f);
const char* intent_convert(const char *intent);

/* functions defined in hashtable.c */
unsigned int hash_function(char *key, size_t len);
static LinkedList* allocate_memory_list ();
static LinkedList* linkedlist_insert(LinkedList* list, Node* item);
static void free_linkedlist(LinkedList* list);
static LinkedList** create_overflow_buckets(HashTable* table);
static void free_overflow_buckets(HashTable* table);
Node* create_item(char* key, const char* intent, const char* entity, const char* responses);
HashTable* create_table(int size);
void free_item(Node* item);
void free_table(HashTable* table);
void handle_collision(HashTable* table, unsigned long index, Node* item);
int ht_insert(HashTable* table, char* key, const char* intent, const char* entity, const char* response);
Node* ht_search(HashTable* table, char* key);
void ht_delete(HashTable* table, char* key);

#endif