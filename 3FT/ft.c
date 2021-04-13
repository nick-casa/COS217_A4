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
                                            Node_getChild(curr, i), type);
            else found = curr;
            if(found != NULL)
                return found;
        }
        return curr;
    }
    return NULL;
}

static int FT_linkParentToChild(Node_T parent, Node_T child) {

    assert(parent != NULL);

    if(Node_linkChild(parent, child) != SUCCESS) {
        (void) Node_destroy(child);
        return PARENT_CHILD_ERROR;
    }

    return SUCCESS;
}

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
        if(!strcmp(path, Node_getPath(curr))) {
            return ALREADY_IN_TREE;
        }
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

int FT_insertDir(char *path) {
    Node_T curr;
    int result;

    /* assert(CheckerDT_isValid(isInitialized,root,count)); */
    assert(path != NULL);

    if(!isInitialized)
        return INITIALIZATION_ERROR;
    curr = FT_traversePathFrom(path, root, ISDIRECTORY);

    result = FT_insertRestOfPath(path, curr, ISDIRECTORY);
    /* assert(CheckerDT_isValid(isInitialized,root,count)); */
    return result;
}

boolean FT_containsFile(char *path){
    return contains(path, ISFILE);
}

boolean FT_containsDir(char *path) {
    return contains(path, ISDIRECTORY);
}

static int FT_rmPathAt(char* path, Node_T curr) {
    Node_T parent;

    assert(path != NULL);
    assert(curr != NULL);

    parent = Node_getParent(curr);

    if(!strcmp(path,Node_getPath(curr))) {
        if(parent == NULL)
            root = NULL;
        else
            Node_unlinkChild(parent, curr);

        if(curr != NULL) count -= Node_destroy(curr);
        return SUCCESS;
    }
    else
        return NO_SUCH_PATH;
}

int FT_rmDir(char *path){
    Node_T curr;
    int result;

    /*assert(CheckerDT_isValid(isInitialized,root,count));*/
    assert(path != NULL);

    if(!isInitialized)
        return INITIALIZATION_ERROR;

    curr = FT_traversePathFrom(path, root, ISDIRECTORY);
    if(curr == NULL)
        result =  NO_SUCH_PATH;
    else
        result = FT_rmPathAt(path, cur);

    /*assert(CheckerDT_isValid(isInitialized,root,count));*/
    return result;
}

int FT_insertFile(char *path, void *contents, size_t length){
    /* can't insert file if root is NULL */
    Node_T curr, new;
    int result;

    /* assert(CheckerDT_isValid(isInitialized,root,count)); */
    assert(path != NULL);
    if(!isInitialized) return INITIALIZATION_ERROR;
    if (root == NULL) return CONFLICTING_PATH;

    curr = FT_traversePathFrom(path, root, ISFILE);
    if(curr == NULL) {
        if (root != NULL) return CONFLICTING_PATH;
    }
    else{
        if(!strcmp(path, Node_getPath(curr)))
            return ALREADY_IN_TREE;
        restPath += (strlen(Node_getPath(curr)) + 1);
    }

    /* check this*/
    if (isFile(curr)) return NOT_A_DIRECTORY;

    copyPath = malloc(strlen(restPath)+1);
    if(copyPath == NULL)
        return MEMORY_ERROR;
    strcpy(copyPath, restPath);

    /* check this */
    /* maybe NOT_A_DIRECTORY */
    if (strstr(copyPath,'/') != NULL) return CONFLICTING_PATH;

    new = Node_create(copyPath, curr, contents, length, ISFILE);
    result = FT_linkParentToChild(curr, new);

    if(result == SUCCESS) count++;
    else (void) Node_destroy(new);

    /* assert(CheckerDT_isValid(isInitialized,root,count)); */
    return result;
}

int FT_rmFile(char *path){
    Node_T curr;
    int result;

    /*assert(CheckerDT_isValid(isInitialized,root,count));*/
    assert(path != NULL);

    if(!isInitialized)
        return INITIALIZATION_ERROR;

    curr = FT_traversePathFrom(path, root, ISFILE);
    if(curr == NULL)
        result =  NO_SUCH_PATH;
    else
        result = FT_rmPathAt(path, curr);

    /*assert(CheckerDT_isValid(isInitialized,root,count));*/
    return result;

}

void *FT_getFileContents(char *path){
    Node_T curr;
    void* result;

    /* assert(CheckerDT_isValid(isInitialized,root,count)); */
    assert(path != NULL);

    curr = FT_traversePathFrom(path, root, ISFILE);
    if(!isFile(curr) || curr == NULL) result = NULL;
    else result = getFileContents(curr);

    /*assert(CheckerDT_isValid(isInitialized,root,count));*/
    return result;
}

void *FT_replaceFileContents(char *path, void *newContents, size_t newLength) {
    Node_T curr;
    void* result;

    /* assert(CheckerDT_isValid(isInitialized,root,count)); */
    assert(path != NULL);

    curr = FT_traversePathFrom(path, root, ISFILE);
    if(!isFile(curr) || curr == NULL) result = NULL;
    else result = replaceFileContents(curr,newContents,newLength);

    /*assert(CheckerDT_isValid(isInitialized,root,count));*/
    return result;
}

int FT_stat(char *path, boolean *type, size_t *length){
    Node_T curr;
    void* result;

    if(!isInitialized) return INITIALIZATION_ERROR;

    /* assert(CheckerDT_isValid(isInitialized,root,count)); */
    assert(path != NULL);

    if(FT_containsDir(path)) {
        *type = FALSE;
        curr = FT_traversePathFrom(path, root, ISDIRECTORY);
    }
    else if(FT_containsFile(path)){
        *type = TRUE;
        curr = FT_traversePathFrom(path, root, ISFILE);
        *length = getFileLength(curr);
    }
    else
        curr = NULL;

    if (curr == NULL) return NO_SUCH_PATH;
    else return SUCCESS;

}

static size_t FT_preOrderTraversal(Node_T n, DynArray_T d, size_t i) {
    size_t c;

    assert(d != NULL);

    if(n != NULL) {
        (void) DynArray_set(d, i, Node_getPath(n));
        i++;
        for(c = 0; c < Node_getNumChildren(n); c++)
            i = FT_preOrderTraversal(Node_getChild(n, c), d, i);
    }
    return i;
}

/*
   Alternate version of strlen that uses pAcc as an in-out parameter
   to accumulate a string length, rather than returning the length of
   str, and also always adds one more in addition to str's length.
*/
static void FT_strlenAccumulate(char* str, size_t* pAcc) {
    assert(pAcc != NULL);

    if(str != NULL)
        *pAcc += (strlen(str) + 1);
}

/*
   Alternate version of strcat that inverts the typical argument
   order, appending str onto acc, and also always adds a newline at
   the end of the concatenated string.
*/
static void FT_strcatAccumulate(char* str, char* acc) {
    assert(acc != NULL);

    if(str != NULL)
        strcat(acc, str); strcat(acc, "\n");
}


char *FT_toString(void){
    DynArray_T nodes;
    size_t totalStrlen = 1;
    char* result = NULL;

    /* assert(CheckerDT_isValid(isInitialized,root,count)); */

    if(!isInitialized) return NULL;

    nodes = DynArray_new(count);
    (void) FT_preOrderTraversal(root, nodes, 0);

    DynArray_map(nodes, (void (*)(void *, void*)) FT_strlenAccumulate, (void*) &totalStrlen);

    result = malloc(totalStrlen);
    if(result == NULL) {
        DynArray_free(nodes);
        /* assert(CheckerDT_isValid(isInitialized,root,count)); */
        return NULL;
    }
    *result = '\0';

    DynArray_map(nodes, (void (*)(void *, void*)) FT_strcatAccumulate, (void *) result);

    DynArray_free(nodes);
    /* assert(CheckerDT_isValid(isInitialized,root,count)); */
    return result;
}




