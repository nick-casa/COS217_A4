/*--------------------------------------------------------------------*/
/* dt.c                                                               */
/* Author:                                                            */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "dynarray.h"
#include "ft.h"
#include "node.h"

/* A File Tree is an AO with 3 state variables: */
/* a flag for if it is in an initialized state (TRUE) or not (FALSE) */
static boolean isInitialized;
/* a pointer to the root node in the hierarchy */
static Node_T root;
/* a counter of the number of nodes in the hierarchy */
static size_t count;

/*
   Starting at the parameter curr, traverses as far down
   the hierarchy as possible while still matching the path
   parameter.

   Returns a pointer to the farthest matching node down that path,
   or NULL if there is no node in curr's hierarchy that matches
   a prefix of the path
*/
static Node_T FT_traversePathFrom(char* path, Node_T curr, nodeType type) {
    Node_T found;
    size_t i;

    assert(path != NULL);

    if(curr == NULL)
        return NULL;

    else if(!strcmp(path,Node_getPath(curr)))
        return curr;

    else if(!strncmp(path, Node_getPath(curr), strlen(Node_getPath(curr)))) {
        for(i = 0; i < Node_getNumChildren(curr); i++) {
            if (!isFile(Node_getChild(curr, i)))
                found = FT_traversePathFrom(path,
                                            Node_getChild(curr, i));
            else found = curr;
            if(found != NULL)
                return found;
        }
        return curr;
    }
    return NULL;
}
/*
   Given a prospective parent and child node,
   adds child to parent's children list, if possible

   If not possible, destroys the hierarchy rooted at child
   and returns PARENT_CHILD_ERROR, otherwise, returns SUCCESS.
*/
static int FT_linkParentToChild(Node_T parent, Node_T child, nodeType type) {

    assert(parent != NULL);

    if(Node_linkChild(parent, child) != SUCCESS) {
        (void) Node_destroy(child);
        return PARENT_CHILD_ERROR;
    }

    return SUCCESS;
}
/*
   Inserts a new path into the tree rooted at parent, or, if
   parent is NULL, as the root of the data structure.

   If a node representing path already exists, returns ALREADY_IN_TREE

   If there is an allocation error in creating any of the new nodes or
   their fields, returns MEMORY_ERROR

   If there is an error linking any of the new nodes,
   returns PARENT_CHILD_ERROR

   Otherwise, returns SUCCESS
*/
static int FT_insertRestOfPath(char* path, Node_T parent, nodeType type) {

    Node_T curr = parent;
    Node_T firstNew = NULL;
    Node_T new;
    char* copyPath;
    char* restPath = path;
    char* dirToken;
    int result;
    size_t newCount = 0;

    assert(path != NULL);

    if(curr == NULL){
        if(root != NULL) {return CONFLICTING_PATH;
        }
    }

    else {
        if(!strcmp(path, Node_getPath(curr))) return ALREADY_IN_TREE;
        restPath += (strlen(Node_getPath(curr)) + 1);
    }

    copyPath = malloc(strlen(restPath)+1);
    if(copyPath == NULL)
        return MEMORY_ERROR;
    strcpy(copyPath, restPath);
    dirToken = strtok(copyPath, "/");

    while(dirToken != NULL) {
        new = Node_create(dirToken, curr, NULL, 0, ISDIRECTORY);
        newCount++;

        if(firstNew == NULL)
            firstNew = new;
        else {
            result = FT_linkParentToChild(curr, new);
            if(result != SUCCESS) {
                (void) Node_destroy(new);
                (void) Node_destroy(firstNew);
                free(copyPath);
                return result;
            }
        }

        if(new == NULL) {
            (void) Node_destroy(firstNew);
            free(copyPath);
            return MEMORY_ERROR;
        }

        curr = new;
        dirToken = strtok(NULL, "/");
    }

    free(copyPath);

    if(parent == NULL) {
        root = firstNew;
        count = newCount;
        return SUCCESS;
    }
    else {
        result = FT_linkParentToChild(parent, firstNew);
        if(result == SUCCESS)
            count += newCount;
        else
            (void) Node_destroy(firstNew);

        return result;
    }
}
/**/
static boolean contains(char *path, nodeType type){
    Node_T curr;
    boolean result;

    /* assert(CheckerDT_isValid(isInitialized,root,count)); */
    assert(path != NULL);

    if(!isInitialized)
        return FALSE;

    curr = FT_traversePathFrom(path, root, type);

    if(curr == NULL)
        result = FALSE;
    else if(strcmp(path, Node_getPath(curr)))
        result = FALSE;
    else
        result = TRUE;

    /*assert(CheckerDT_isValid(isInitialized,root,count)); */
    return result;
}
/**/
int FT_insertDir(char *path) {
    Node_T curr;
    int result;

    /* assert(CheckerDT_isValid(isInitialized,root,count)); */
    assert(path != NULL);

    if(!isInitialized)
        return INITIALIZATION_ERROR;
    curr = FT_traversePathFrom(path, root, NOT_A_DIRECTORY);
    result = FT_insertRestOfPath(path, curr, NOT_A_DIRECTORY);
    /* assert(CheckerDT_isValid(isInitialized,root,count)); */
    return result;
}
/**/
boolean FT_containsDir(char *path) {
    return contains(path, ISDIRECTORY);
}

/**/
int FT_rmDir(char *path){

}

/**/
boolean FT_containsFile(char *path){
    return contains(path, ISFILE);
}
/**/
int FT_insertFile(char *path, void *contents, size_t length){
    /* can't insert file if root is NULL */
}

int FT_rmFile(char *path);
void *FT_getFileContents(char *path);
void *FT_replaceFileContents(char *path, void *newContents, size_t newLength);
int FT_stat(char *path, boolean *type, size_t *length);
char *FT_toString(void);



int FT_destroy(void){
    /* assert(CheckerDT_isValid(isInitialized,root,count)); */
    if(!isInitialized)
        return INITIALIZATION_ERROR;
    if(curr != NULL) count -= Node_destroy(curr);
    root = NULL;
    isInitialized = 0;
    /* assert(CheckerDT_isValid(isInitialized,root,count)); */
    return SUCCESS;
}

int FT_init(void){
    /* assert(CheckerDT_isValid(isInitialized,root,count)); */
    if(isInitialized)
        return INITIALIZATION_ERROR;
    isInitialized = 1;
    root = NULL;
    count = 0;
    /* assert(CheckerDT_isValid(isInitialized,root,count)); */
    return SUCCESS;
}


