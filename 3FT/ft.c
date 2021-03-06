/*--------------------------------------------------------------------*/
/* ft.c                                                               */
/* Author: Alina Chen and Nickolas Casalinuovo                        */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <string.h>
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
static Node_T FT_traversePathFrom(char* path, Node_T curr) {
    Node_T found;
    size_t i;

    assert(path != NULL);

    if(curr == NULL)
        return NULL;

    if(!strcmp(path,Node_getPath(curr)))
        return curr;

    if(!strncmp(path, Node_getPath(curr), strlen(Node_getPath(curr)))) {
        /* Loop through all directory children */
        for(i = 0; i < Node_getNumDirChildren(curr); i++) {
            /* Recur down directory children */
            found = FT_traversePathFrom(path,
                                        Node_getChildDirectory(curr, i));
            /* If found, stop recursion */
            if(found != NULL) return found;
        }
        /* Loop through all file children */
        for(i = 0; i < Node_getNumFileChildren(curr); i++) {
            /* Recur down file children  */
            found = FT_traversePathFrom(path,
                                        Node_getChildFile(curr, i));
            /* If found, stop recursion */
            if(found != NULL) return found;
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
static int FT_linkParentToChild(Node_T parent, Node_T child) {

    assert(parent != NULL);

    if(Node_linkChild(parent, child) != SUCCESS) {
        (void) Node_destroy(child, getType(child));
        return PARENT_CHILD_ERROR;
    }

    return SUCCESS;
}

/*
   Inserts a new path into the tree rooted at parent, or, if
   parent is NULL, as the root of the data structure.

   Finds rest of path depending on nodeType type
   the arguments contents and length are carried down to create the end node

   If a node representing path already exists, returns ALREADY_IN_TREE

   If a proper prefix of a path exists as a file, return NOT_A_DIRECTORY.

   If there is an allocation error in creating any of the new nodes or
   their fields, returns MEMORY_ERROR

   If there is an error linking any of the new nodes,
   returns PARENT_CHILD_ERROR

   Otherwise, returns SUCCESS
*/
static int FT_insertRestOfPath(char* path, Node_T parent, nodeType type, void* contents, size_t length) {
    Node_T curr = parent;
    Node_T firstNew = NULL;
    Node_T new;
    char* copyPath;
    char* restPath = path;
    char* dirToken;
    int result;
    size_t newCount = 0;

    assert(path != NULL);

    /* if current node is null */
    if(curr == NULL){
        if(root != NULL) return CONFLICTING_PATH;

    }
    /* if we have a valid curr */
    else {
        /* check if already a path */
        if(!strcmp(path, Node_getPath(curr))) return ALREADY_IN_TREE;
        /* if path doesnt already exist find rest of path */
        restPath += (strlen(Node_getPath(curr)) + 1);
    }
    if(isFile(parent)) return NOT_A_DIRECTORY;

    /* allocate memory for rest of path */
    copyPath = malloc(strlen(restPath)+1);
    if(copyPath == NULL)
        return MEMORY_ERROR;
    strcpy(copyPath, restPath);
    dirToken = strtok(copyPath, "/");

    /* while there are directories to visit */
    while(dirToken != NULL) {
        /* track next token */
        char* nextToken = strtok(NULL, "/");
        /* insert last file node */
        if (type == ISFILE && nextToken == NULL){
            new = Node_create(dirToken, curr, contents, length, ISFILE);
        }
        /* insert directory nodes */
        else{
            new = Node_create(dirToken, curr, NULL, 0, ISDIRECTORY);
        }
        /* add to new count */
        newCount++;

        if(firstNew == NULL)
            firstNew = new;
        else {
            /* if not the first new child, link */
            result = FT_linkParentToChild(curr, new);
            if(result != SUCCESS) {
                (void) Node_destroy(new, getType(new));
                (void) Node_destroy(firstNew, getType(firstNew));
                free(copyPath);
                return result;
            }
        }
        if(new == NULL) {
            /* if new was not created */
            (void) Node_destroy(firstNew, getType(firstNew));
            free(copyPath);
            return MEMORY_ERROR;
        }

        curr = new;
        dirToken = nextToken;
    }

    free(copyPath);

    if(parent == NULL) {
        root = firstNew;
        count = newCount;
        return SUCCESS;
    }
    /* link rest to parent */
    result = FT_linkParentToChild(parent, firstNew);
    if(result == SUCCESS)
        count += newCount;

    return result;
}

/*
  Returns TRUE if the tree contains the full path parameter as the type
  given by the type parameter and FALSE otherwise.
*/
static boolean FT_contains(char *path, nodeType type){
    Node_T curr;
    boolean result = FALSE;

    assert(path != NULL);

    if(!isInitialized)
        return FALSE;

    curr = FT_traversePathFrom(path, root);
    if(curr == NULL)
        result = FALSE;
    else if(strcmp(path, Node_getPath(curr)))
        /*if(type==ISFILE && Node)*/
        result = FALSE;
    else{
        if(getType(curr) == type) result = TRUE;
        else result = FALSE;
    }

    return result;
}


boolean FT_containsFile(char *path){
    assert(path != NULL);
    return FT_contains(path, ISFILE);
}


boolean FT_containsDir(char *path) {
    assert(path != NULL);
    return FT_contains(path, ISDIRECTORY);
}


int FT_destroy(void){
    if(!isInitialized)
        return INITIALIZATION_ERROR;
    if(root != NULL) {
        if(isFile(root)) count -= Node_destroy(root, ISFILE);
        else count -= Node_destroy(root, ISDIRECTORY);
    }
    root = NULL;
    isInitialized = 0;
    return SUCCESS;
}

int FT_init(void){
    if(isInitialized)
        return INITIALIZATION_ERROR;
    isInitialized = 1;
    root = NULL;
    count = 0;
    return SUCCESS;
}

int FT_insertDir(char *path) {
    Node_T curr;
    int result;

    assert(path != NULL);

    if(!isInitialized)
        return INITIALIZATION_ERROR;
    curr = FT_traversePathFrom(path, root);
    result = FT_insertRestOfPath(path, curr, ISDIRECTORY, NULL, 0);
    return result;
}

/*
  Removes the hierarchy rooted at path starting from
  curr. If curr is the data structure's root, root becomes NULL.

  Returns NO_SUCH_PATH if curr is not the node for path,
  and SUCCESS otherwise.
 */
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

        count -= Node_destroy(curr, getType(curr));
        return SUCCESS;
    }
    else
        return NO_SUCH_PATH;
}

int FT_rmDir(char *path){
    Node_T curr;
    int result;

    assert(path != NULL);

    if(!isInitialized)
        return INITIALIZATION_ERROR;

    curr = FT_traversePathFrom(path, root);
    if(curr == NULL || strcmp(Node_getPath(curr), path))
        result =  NO_SUCH_PATH;
    else if(isFile(curr)) result = NOT_A_DIRECTORY;
    else
        result = FT_rmPathAt(path, curr);

    return result;
}

int FT_insertFile(char *path, void *contents, size_t length){
    Node_T curr;
    int result;

    assert(path != NULL);
    if(!isInitialized) return INITIALIZATION_ERROR;
    if (root == NULL) return CONFLICTING_PATH;

    curr = FT_traversePathFrom(path, root);

    result = FT_insertRestOfPath(path, curr, ISFILE, contents, length);
    return result;
}

int FT_rmFile(char *path){
    Node_T curr;
    int result;

    assert(path != NULL);

    if(!isInitialized)
        return INITIALIZATION_ERROR;

    curr = FT_traversePathFrom(path, root);
    if(curr == NULL || strcmp(Node_getPath(curr),path))
        result =  NO_SUCH_PATH;
    else if(!isFile(curr)) result = NOT_A_FILE;
    else
        result = FT_rmPathAt(path, curr);

    return result;

}

void *FT_getFileContents(char *path){
    Node_T curr;
    void* result;

    assert(path != NULL);

    curr = FT_traversePathFrom(path, root);
    if(!isFile(curr) || curr == NULL) result = NULL;
    else result = getFileContents(curr);

    return result;
}

void *FT_replaceFileContents(char *path, void *newContents, size_t newLength) {
    Node_T curr;
    void* result;

    assert(path != NULL);

    curr = FT_traversePathFrom(path, root);
    if(!isFile(curr) || curr == NULL) result = NULL;
    else result = replaceFileContents(curr,newContents,newLength);

    return result;
}

int FT_stat(char *path, boolean *type, size_t *length){
    Node_T curr;

    if(!isInitialized) return INITIALIZATION_ERROR;
    assert(type!=NULL);
    assert(length!=NULL);
    assert(path != NULL);

    if(FT_containsDir(path)) {
        *type = FALSE;
        curr = FT_traversePathFrom(path, root);
    }
    else if(FT_containsFile(path)){
        *type = TRUE;
        curr = FT_traversePathFrom(path, root);
        *length = getFileLength(curr);
    }
    else
        curr = NULL;

    if (curr == NULL) return NO_SUCH_PATH;
    else return SUCCESS;

}

/*
   Performs a pre-order traversal of the tree rooted at n,
   inserting each payload to DynArray_T d beginning at index i.
   Returns the next unused index in d after the insertion(s).
*/
static size_t FT_preOrderTraversal(Node_T n, DynArray_T d, size_t i) {
    size_t c;
    size_t cFile;
    Node_T file;

    assert(d != NULL);

    if(n != NULL) {
        (void) DynArray_set(d, i, Node_getPath(n));
        i++;
        for(cFile = 0; cFile < Node_getNumFileChildren(n); cFile++){
            file = Node_getChildFile(n, cFile);
            (void) DynArray_set(d, i, Node_getPath(file));
            i++;
        }
        for(c = 0; c < Node_getNumDirChildren(n); c++) {
            i = FT_preOrderTraversal(Node_getChildDirectory(n, c), d, i);
        }
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
    assert(str != NULL);

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
    assert(str != NULL);

    if(str != NULL)
        strcat(acc, str); strcat(acc, "\n");
}

char *FT_toString(void){
    DynArray_T nodes;
    size_t totalStrlen = 1;
    char* result = NULL;


    if(!isInitialized) return NULL;

    nodes = DynArray_new(count);
    (void) FT_preOrderTraversal(root, nodes, 0);

    DynArray_map(nodes, (void (*)(void *, void*)) FT_strlenAccumulate, (void*) &totalStrlen);

    result = malloc(totalStrlen);
    if(result == NULL) {
        DynArray_free(nodes);
        return NULL;
    }
    *result = '\0';

    DynArray_map(nodes, (void (*)(void *, void*)) FT_strcatAccumulate, (void *) result);

    DynArray_free(nodes);
    return result;
}





