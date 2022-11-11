/*
 * INF1002 (C Language) Group Project.
 *
 * This file implements the chatbot's knowledge base.
 *
 * knowledge_get() retrieves the response to a question.
 * knowledge_put() inserts a new response to a question.
 * knowledge_read() reads the knowledge base from a file.
 * knowledge_reset() erases all of the knowledge.
 * knowledge_write() saves the knowledge base in a file.
 *
 * You may add helper functions as necessary.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "chat1002.h" //uncomment this line if you have error.
#include "hashtable.c"
#define CAPACITY 1001 // Size of the Hash Table

Node *head = NULL;
Node *end = NULL;
HashTable* ht = NULL;

/*
 * Get the response to a question.
 *
 * Input:
 *   intent   - the question word
 *   entity   - the entity
 *   response - a buffer to receive the response
 *   n        - the maximum number of characters to write to the response buffer
 *
 * Returns:
 *   KB_OK, if a response was found for the intent and entity (the response is copied to the response buffer)
 *   KB_NOTFOUND, if no response could be found
 *   KB_INVALID, if 'intent' is not a recognised question word
 */

void hashtable_callup(){
	if (ht == NULL)	ht = create_table(CAPACITY); //If hashtable does not exist then create a hashtable.
}

int knowledge_get(const char *intent, const char *entity, char *response, int n) {;
	if (chatbot_is_question(intent) == KB_INVALID) { //If first word of user is not intent then return KB_invalid/not recognised.
		return KB_INVALID;
	}
	char* key = (char*)calloc(1, MAX_INTENT+1+MAX_ENTITY+1); //allocate memory space of MAX_INTENT+MAX_ENTITY for key.
	char *tempintent = (char*) calloc (1, MAX_INTENT);
	strcpy(tempintent, intent);
	for (int i = 0; i <strlen(tempintent); i++){
		tempintent[i]=tolower(tempintent[i]);
	}
	strcpy(key, tempintent); //Copy intent onto key
	strcat(key, entity); //Concatenate entity onto key. Eg. Who is Mike will become whoMike. This is used to create unique keys for hashtable.
	Node* knowledge = ht_search(ht, key); //Invoke ht_search which return knowledge node if found.
	free(key);
	free(tempintent);
	if (knowledge == NULL) { //If knowledge node is empty then return item not found.
			return KB_NOTFOUND;
		}
	else{ //Else if item is not empty then print out response to user.
			char * buf = NULL; 
    		buf = knowledge->responses;
			snprintf(response, n, "%s", buf);
			return KB_OK;
	}

}


/*
 * Insert a new response to a question. If a response already exists for the
 * given intent and entity, it will be overwritten. Otherwise, it will be added
 * to the knowledge base.
 *
 * Input:
 *   intent    - the question word
 *   entity    - the entity
 *   response  - the response for this question and entity
 *
 * Returns:
 *   KB_FOUND, if successful
 *   KB_NOMEM, if there was a memory allocation failure
 *   KB_INVALID, if the intent is not a valid question word
 */
int knowledge_put(const char *intent, const char *entity, const char *response) {
	if (chatbot_is_question(intent) == KB_INVALID) { //If first word of user is not intent then return KB_invalid/not recognised.
		return KB_INVALID;
	}
	char* key = (char*)calloc(1, MAX_INTENT+1+MAX_ENTITY+1); //allocate memory space of MAX_INTENT+MAX_ENTITY for key.
	char *tempintent = (char*) calloc (1, MAX_INTENT);
	strcpy(tempintent, intent);
	for (int i = 0; i <strlen(tempintent); i++){
		tempintent[i]=tolower(tempintent[i]);
	}
	strcpy(key, tempintent); //Copy intent onto key
	strcat(key, entity); //Concatenate entity onto key. Eg. Who is Mike will become whoMike. This is used to create unique keys for hashtable.
	int successful = ht_insert(ht, key, intent, entity, response); //Invoke ht_insert which return knowledge node if found.
    if (!successful){ //If unable to be inserted into hashtable then return memory allocation error.
            return KB_NOMEM;
        }
	free(tempintent);
	free(key);
	return KB_OK; //else return it is successful. 

}


/*
 * Read a knowledge base from a file.
 *
 * Input:
 *   f - the file
 *
 * Returns: the number of entity/response pairs successful read from the file
 */
int knowledge_read(FILE *f) {
	int erpair = 0; //count number of er pair successfully read from file.
	int length = MAX_ENTITY + 1 + MAX_RESPONSE + 1; //initialise maximum length of each line/
	int linelength = 0; //store length of each line.
	char * buf = calloc(1, length); //allocate memory to buffer to store each line read from file.
	if (buf == NULL) { //if unable to allocate memory then return memory allocation failure
		return KB_NOMEM;
	}
	int isquestion = 0; //initalise variable isquestion to determine whether intent is valid.
	char * headertext; //initalise variable to store intent
	headertext = (char *)calloc(1, 7); //allocate memory to store each intent temporarily.
	 while ((fgets(buf, length, (FILE*)f)) != NULL) { //while not end of file
		if (buf == NULL){ //if empty line then continue to next iteration.
			continue;
		}
		if (buf[0] == '['){ //if first character of line is [ then extract string between delimiters []
			buf = strtok(buf, "]"); //removes ] from string
			buf = strtok(buf, "["); //return [ from string
			isquestion = chatbot_is_question(buf); //check whether string is a valid intent/question. 
			if (!isquestion) continue; //if it is not a valid intent then continue
			strcpy(headertext, buf); //else copy the intent from buf to headertext and continue.
			continue;
		}

		char *entity = (char *) calloc(1, MAX_ENTITY); //allocate memory for entity and set all values in memory to be 0.
		snprintf(entity, MAX_ENTITY, "%s", strtok(buf, "=")); //store string before delimiter = into variable entity

		char *response = (char *) calloc(1, MAX_RESPONSE); //allocate memory for response and set all values in memory to be 0.
		snprintf(response, MAX_RESPONSE, "%s", strtok(NULL, "\r\n")); //store string before delimiter "\r\n" which is a newline character into variable entity

		int result = knowledge_put(headertext, entity, response); //invoke knowledge_put for data retrieved from line,
		if (result != KB_OK) {
			return result;
		}
		erpair++; //add 1 to erpair
		free(entity); //free up memory allocated for entity
		free(response); //free up memory allocated for response
	}
	free(headertext); //free up memory allocated for response
	return erpair;
}


/*
 * Reset the knowledge base, removing all know entitities from all intents.
 */
void knowledge_reset() {
	if (ht == NULL) return; //if hash table is null/does not exist then return.
	free_table(ht); //else invoke free_table function
	HashTable* ht = create_table(CAPACITY); //create new empty hash table.
}


/*
 * Write the knowledge base to a file.
 *
 * Input:
 *   f - the file
 */
void knowledge_write(FILE *f) {
	fputs("[what]\n", f); //insert intent onto file
	if (ht != NULL){ //if hashtable is not empty then iterate through it and get all items with what intent
		for (int i = 0; i<ht->size; i++){
			if (ht->items[i] != NULL){
			if (compare_token(ht->items[i]->intent, "what") == 0 ){
				fprintf(f, "%s=%s\n", ht->items[i]->entity, ht->items[i]->responses);
			}
			}
		}
	}

	fputs("[where]\n", f); //insert intent onto file
	if (ht != NULL){ //if hashtable is not empty then iterate through it and get all items with where intent
		for (int i = 0; i<ht->size; i++){
			if (ht->items[i]!= NULL){
			if (compare_token(ht->items[i]->intent, "where")== 0 ){
				fprintf(f, "%s=%s\n", ht->items[i]->entity, ht->items[i]->responses);
			}
			}
		}
	}
	
	fputs("[who]\n", f); //insert intent onto file
	if (ht != NULL){ //if hashtable is not empty then iterate through it and get all items with who intent
		for (int i = 0; i<ht->size; i++){
			if (ht->items[i] != NULL){
			if (compare_token(ht->items[i]->intent, "who")== 0 ){
				fprintf(f, "%s=%s\n", ht->items[i]->entity, ht->items[i]->responses);
			}
			}
		}
	}
