/*
 * INF1002 (C Language) Group Project.
 *
 * This file implements the behaviour of the chatbot. The main entry point to
 * this module is the chatbot_main() function, which identifies the intent
 * using the chatbot_is_*() functions then invokes the matching chatbot_do_*()
 * function to carry out the intent.
 *
 * chatbot_main() and chatbot_do_*() have the same method signature, which
 * works as described here.
 *
 * Input parameters:
 *   inc      - the number of words in the question
 *   inv      - an array of pointers to each word in the question
 *   response - a buffer to receive the response
 *   n        - the size of the response buffer
 *
 * The first word indicates the intent. If the intent is not recognised, the
 * chatbot should respond with "I do not understand [intent]." or similar, and
 * ignore the rest of the input.
 *
 * If the second word may be a part of speech that makes sense for the intent.
 *    - for WHAT, WHERE and WHO, it may be "is" or "are".
 *    - for SAVE, it may be "as" or "to".
 *    - for LOAD, it may be "from".
 * The word is otherwise ignored and may be omitted.
 *
 * The remainder of the input (including the second word, if it is not one of the
 * above) is the entity.
 *
 * The chatbot's answer should be stored in the output buffer, and be no longer
 * than n characters long (you can use snprintf() to do this). The contents of
 * this buffer will be printed by the main loop.
 *
 * The behaviour of the other functions is described individually in a comment
 * immediately before the function declaration.
 *
 * You can rename the chatbot and the user by changing chatbot_botname() and
 * chatbot_username(), respectively. The main loop will print the strings
 * returned by these functions at the start of each line.
 */

#include <stdio.h>
#include <string.h>
#include "chat1002.h"
#include "knowledge.c" //uncomment this line if you have error.

/*
 * Get the name of the chatbot from chat1002.h
 *
 * Returns: the name of the chatbot as a null-terminated string
 */
const char *chatbot_botname() {

	return BOT_NAME;

}


/*
 * Get the name of the user from chat1002.h
 *
 * Returns: the name of the user as a null-terminated string
 */
const char *chatbot_username() {

	return USER_NAME;

}


/*
 * Get a response to user input.
 *
 * See the comment at the top of the file for a description of how this
 * function is used.
 *
 * Returns:
 *   0, if the chatbot should continue chatting
 *   1, if the chatbot should stop (i.e. it detected the EXIT intent)
 */
int chatbot_main(int inc, char *inv[], char *response, int n) {

	/* check for empty input */
	if (inc < 1) {
		snprintf(response, n, "");
		return 0;
	}

	/* look for an intent and invoke the corresponding do_* function */
	hashtable_callup();
	if (chatbot_is_exit(inv[0]))
		return chatbot_do_exit(inc, inv, response, n);
	else if (chatbot_is_load(inv[0]))
		return chatbot_do_load(inc, inv, response, n);
	else if (chatbot_is_question(inv[0]))
		return chatbot_do_question(inc, inv, response, n);
	else if (chatbot_is_reset(inv[0]))
		return chatbot_do_reset(inc, inv, response, n);
	else if (chatbot_is_save(inv[0]))
		return chatbot_do_save(inc, inv, response, n);
	else {
		snprintf(response, n, "I don't understand \"%s\".", inv[0]);
		return 0;
	}

}


/*
 * Determine whether an intent is EXIT.
 *
 * Input:
 *  intent - the intent
 *
 * Returns:
 *  1, if the intent is "exit" or "quit"
 *  0, otherwise
 */
int chatbot_is_exit(const char *intent) {

	return compare_token(intent, "exit") == 0 || compare_token(intent, "quit") == 0;

}


/*
 * Perform the EXIT intent.
 *
 * See the comment at the top of the file for a description of how this
 * function is used.
 *
 * Returns:
 *   0 (the chatbot always continues chatting after a question)
 */
int chatbot_do_exit(int inc, char *inv[], char *response, int n) {

	snprintf(response, n, "Goodbye!");
	return 1;

}


/*
 * Determine whether an intent is LOAD.
 *
 * Input:
 *  intent - the intent
 *
 * Returns:
 *  1, if the intent is "load"
 *  0, otherwise
 */
int chatbot_is_load(const char *intent) {
	return compare_token(intent, "load") == 0;
}


/*
 * Load a chatbot's knowledge base from a file.
 *
 * See the comment at the top of the file for a description of how this
 * function is used.
 *
 * Returns:
 *   0 (the chatbot always continues chatting after loading knowledge)
 */
int chatbot_do_load(int inc, char *inv[], char *response, int n) {
	if (inc < 2) {
		snprintf(response, n, "%s", "Please enter a valid filename!");
		return 0;
	}

	int startindex = 1;

	FILE * fp;
	char * filename = inv[startindex];
	int inifound = 0;
	char * checkini = strchr(filename, '.');
	if (checkini != NULL){
		if(!compare_token(checkini, ".ini")){
			inifound = 1;
		}
	}
	if (!inifound){
		snprintf(response, n, "%s", "Please enter a valid filename!");
		return 0;
	}
	fp = fopen(filename, "r");
	if (fp != NULL) {
		int result = knowledge_read(fp);
		fclose(fp);
		if (result == KB_NOMEM){
			snprintf(response, n, "Out of Memory");
		} else {
			snprintf(response, n, "Read %d responses from %s", result, filename);
		}
	} else {
		snprintf(response, n, "File %s not found", filename);
	}
	return 0;

}


/*
 * Determine whether an intent is a question.
 *
 * Input:
 *  intent - the intent
 *
 * Returns:
 *  1, if the intent is "what", "where", or "who"
 *  0, otherwise
 */
int chatbot_is_question(const char *intent) {
	char reg_intent[3][6] = {"WHAT", "WHERE", "WHO"};
	for (int i = 0; i < 3; i++) {
		if (compare_token(reg_intent[i], intent) == 0) {
			return 1;
		}
	}
	return 0;
}


/*
 * Answer a question.
 *
 * inv[0] contains the the question word.
 * inv[1] may contain "is" or "are"; if so, it is skipped.
 * The remainder of the words form the entity.
 *
 * See the comment at the top of the file for a description of how this
 * function is used.
 *
 * Returns:
 *   0 (the chatbot always continues chatting after a question)
 */
int chatbot_do_question(int inc, char *inv[], char *response, int n) {

	int result = 100;
	int startindex = 1;
	char * entity;
	char * fillerword = calloc(1, 4);
	entity = calloc(1, MAX_ENTITY);
	if (entity == NULL){
		snprintf(response, n, "Out of memory\n");
		return 1;
	}


	if (inc > 1) {
		if (compare_token(inv[1], "is") == 0 || compare_token(inv[1], "are") == 0){
            startindex = 2;
			if (startindex == 2 && inc < 3){
				snprintf(response, n, "Please ask a question with an entity.");
				free(entity);
				free(fillerword);
				return 0;
			}
			strcpy(fillerword, inv[1]);
		} 
        for(int i = startindex; i < inc; i++){
                strcat(entity, inv[i]);
			if (i != sizeof(inv - 1)){
				strcat(entity, " ");
                }
			}
			entity[strlen(entity)-1] = '\0';
			result = knowledge_get(inv[0], entity, response, n);
		}
	 else {
		snprintf(response, n, "Please ask a question with an entity.");
	}

	if (result == KB_NOTFOUND) {
		// Rebuild question to be displayed
		char *entityquestion;
		entityquestion = calloc(1, MAX_ENTITY);
		if (entityquestion == NULL){
			snprintf(response, n, "Out of memory\n");
		}
		strcpy(entityquestion, entity);
		char ans[MAX_RESPONSE];
		if (strlen(fillerword)){
            prompt_user(ans, MAX_RESPONSE, "I don't know. %s %s %s?", inv[0], fillerword, entityquestion);
		} else {
		    prompt_user(ans, MAX_RESPONSE, "I don't know. %s %s?", inv[0], entityquestion);
		}
		result = knowledge_put(inv[0], entity , ans);
		if (result == KB_OK){
			snprintf(response, n, "Thank you.");
		} else if (result == KB_NOMEM) {
			snprintf(response, n, "Out of Memory");
		} else if (result == KB_INVALID) {
			snprintf(response, n, "Out of Memory");
		}
		free(entityquestion);
		free(fillerword);
	}
	free(entity);
	return 0;

}


/*
 * Determine whether an intent is RESET.
 *
 * Input:
 *  intent - the intent
 *
 * Returns:
 *  1, if the intent is "reset"
 *  0, otherwise
 */
int chatbot_is_reset(const char *intent) {
	if (!compare_token("RESET", intent)) {
		return 1;
	}
	return 0;
}


/*
 * Reset the chatbot.
 *
 * See the comment at the top of the file for a description of how this
 * function is used.
 *
 * Returns:
 *   0 (the chatbot always continues chatting after beign reset)
 */
int chatbot_do_reset(int inc, char *inv[], char *response, int n) {
	knowledge_reset();
	snprintf(response, MAX_RESPONSE, "Chatbot reset");
	return 0;
}


/*
 * Determine whether an intent is SAVE.
 *
 * Input:
 *  intent - the intent
 *
 * Returns:
 *  1, if the intent is "what", "where", or "who"
 *  0, otherwise
 */
int chatbot_is_save(const char *intent) {
    return compare_token(intent, "save") == 0;
}


/*
 * Save the chatbot's knowledge to a file.
 *
 * See the comment at the top of the file for a description of how this
 * function is used.
 *
 * Returns:
 *   0 (the chatbot always continues chatting after saving knowledge)
 */
int chatbot_do_save(int inc, char *inv[], char *response, int n) {
	if (inc < 2) {
		snprintf(response, n, "%s", "Please enter a valid filename!");
		return 0;
	}
	int startindex = 1;
	if (compare_token(inv[1], "as") != 0){
		snprintf(response, n, "%s", "Please enter a valid filename!");
		return 0;
	} else {
		startindex = 2;
	}

	if (startindex == 2 && inc < 3) {
		snprintf(response, n, "%s", "Please enter a valid filename!");
		return 0;
	}

	char * filename = inv[startindex];
	if (filename[0] == 0) {
		snprintf(response, n, "%s", "Please enter a valid filename!");
		return 0;
	}

	int inifound = 0;
	char * checkini = strchr(filename, '.');
	if (checkini != NULL){
		if(!compare_token(checkini, ".ini")){
			inifound = 1;
		}
	}

	if (!inifound){
		snprintf(response, n, "%s", "Please enter a valid filename!");
		return 0;
	}

	FILE * file;
	file = fopen(filename, "w");
	knowledge_write(file);
	fclose(file);

	snprintf(response, n, "My knowledge has been saved to %s", filename);

	return 0;

}
