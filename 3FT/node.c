/*--------------------------------------------------------------------*/
/* node.c                                                             */
/* Author:                                                            */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "dynarray.h"
#include "node.h"
/* #include "checkerDT.h" */

/*
   A node structure represents a file or a directory in the tree
*/
struct node {
   /* the full path of this node */
   char* path;

   /* the parent of this node
      NULL for the root of the tree */
   Node_T parent;

   /* the children nodes of this node
      stored in sorted order by name */
   DynArray_T children;

   /* if the node is a file, contains
      contents. Otherwise, NULL  */
   void* pvContents;

   /* if the node is a file, contains
      length. Otherwise, NULL */
   size_t uLength;

   /* contains information on if the node
      is a file or a directory. */
   nodeType type;
};


/*
  returns a path with contents
  n->path/dir
  or NULL if there is an allocation error.

  Allocates memory for the returned string,
  which is then owned by the caller!
*/
static char* Node_buildPath(Node_T n, const char* nodeName) {
   char* path;

   assert(nodeName != NULL);

   if(n == NULL)
      path = malloc(strlen(nodeName)+1);
   else
      path = malloc(strlen(n->path) + 1 + strlen(nodeName) + 1);

   if(path == NULL)
      return NULL;
   *path = '\0';

   if(n != NULL) {
      strcpy(path, n->path);
      strcat(path, "/");
   }
   strcat(path, nodeName);

   return path;
}

/* see node.h for specification */
Node_T Node_create(const char* nodeName, Node_T parent, void* contents, size_t length, nodeType type){
   Node_T new;

   assert(parent == NULL /*|| CheckerDT_Node_isValid(parent)*/);
   assert(nodeName != NULL);

   new = malloc(sizeof(struct node));
   if(new == NULL) {
      assert(parent == NULL /*|| CheckerDT_Node_isValid(parent)*/);
      return NULL;
   }

   new->path = Node_buildPath(parent, nodeName);

   if(new->path == NULL) {
      free(new);
      assert(parent == NULL /*|| CheckerDT_Node_isValid(parent)*/);
      return NULL;
   }

   new->parent = parent;

   if(type == ISFILE){
       new->children = NULL;
       new->uLength= length;
       new->pvContents = contents;
       new->type = type;
   }
   else{
       new->children = DynArray_new(0);
       if(new->children == NULL) {
          free(new->path);
          free(new);
          assert(parent == NULL /* || CheckerDT_Node_isValid(parent)*/);
          return NULL;
       }
   }

   assert(parent == NULL /*|| CheckerDT_Node_isValid(parent)*/);
   /* assert(CheckerDT_Node_isValid(new)); */
   return new;
}

/* see node.h for specification */
size_t Node_destroy(Node_T n) {
   size_t i;
   size_t count = 0;
   Node_T c;

   assert(n != NULL);

   if (type == ISDIRECTORY) {
       for (i = 0; i < DynArray_getLength(n->children); i++) {
           c = DynArray_get(n->children, i);
           count += Node_destroy(c);
       }
       DynArray_free(n->children);
   }
   /* MIGHT NEED TO FIX THIS LATER!!!!!!!!! */
   free(n->path);
   free(n);
   count++;

   return count;
}

/* see node.h for specification */
const char* Node_getPath(Node_T n) {
   assert(n != NULL);

   return n->path;
}

/*   see node.h for specification
     node1 (b is a directory): a/b/
     node2 (b is a file): a/b/
     path names could be same but should return not equal
 */
 int Node_compare(Node_T node1, Node_T node2) {
   assert(node1 != NULL);
   assert(node2 != NULL);

   return strcmp(node1->path, node2->path);
}

/* see node.h for specification */
size_t Node_getNumChildren(Node_T n) {
   assert(n != NULL);
   if(n->type == ISFILE) return 0;
   else return DynArray_getLength(n->children);
}

/* see node.h for specification */
int Node_hasChild(Node_T n, const char* path, size_t* childID) {
   size_t index;
   int result;
   Node_T checker;

   assert(n != NULL);
   assert(path != NULL);

   if (n->type == ISFILE) return 0;

   checker = Node_create(path, NULL);
   if(checker == NULL) {
      return -1;
   }
   result = DynArray_bsearch(n->children, checker, &index,
                    (int (*)(const void*, const void*)) Node_compare);
   (void) Node_destroy(checker);

   if(childID != NULL)
      *childID = index;

   return result;
}

/* see node.h for specification */
Node_T Node_getChild(Node_T n, size_t childID) {
   assert(n != NULL);
   if (n->type == ISFILE) return 0;

   if(DynArray_getLength(n->children) > childID) {
      return DynArray_get(n->children, childID);
   }
   else {
      return NULL;
   }
}

/* see node.h for specification */
Node_T Node_getParent(Node_T n) {
   assert(n != NULL);

   return n->parent;
}


static void assertNodes(Node_T nodeOne, Node_T nodeTwo){
    /* assert(CheckerDT_Node_isValid(parent));
    assert(CheckerDT_Node_isValid(child)); */
}
/* see node.h for specification */
int Node_linkChild(Node_T parent, Node_T child) {
   size_t i;
   char* rest;

   assert(parent != NULL);
   assert(child != NULL);
   assertNodes(parent,child);

   if (parent->type == ISFILE) {
       assertNodes(parent,child);
       return PARENT_CHILD_ERROR;
   }

   if(Node_hasChild(parent, child->path, NULL)) {
      assertNodes(parent,child);
      return ALREADY_IN_TREE;
   }
   i = strlen(parent->path);
   if(strncmp(child->path, parent->path, i)) {
      assertNodes(parent,child);
      return PARENT_CHILD_ERROR;
   }
   rest = child->path + i;
   if(strlen(child->path) >= i && rest[0] != '/') {
      assertNodes(parent,child);
      return PARENT_CHILD_ERROR;
   }
   rest++;
   if(strstr(rest, "/") != NULL) {
      assertNodes(parent,child);
      return PARENT_CHILD_ERROR;
   }
   child->parent = parent;
   if(DynArray_bsearch(parent->children, child, &i,
         (int (*)(const void*, const void*)) Node_compare) == 1) {
      assertNodes(parent,child);
      return ALREADY_IN_TREE;
   }
   if(DynArray_addAt(parent->children, i, child) == TRUE) {
      assertNodes(parent,child);
      return SUCCESS;
   }
   else {
      assertNodes(parent,child);
      return PARENT_CHILD_ERROR;
   }
}

/* see node.h for specification */
int  Node_unlinkChild(Node_T parent, Node_T child) {
   size_t i;

   assert(parent != NULL);
   assert(child != NULL);
   assertNodes(parent,child);

   if(parent->type == ISFILE) return PARENT_CHILD_ERROR;

   if(DynArray_bsearch(parent->children, child, &i,
         (int (*)(const void*, const void*)) Node_compare) == 0) {
      assertNodes(parent,child);
      return PARENT_CHILD_ERROR;
   }

   (void) DynArray_removeAt(parent->children, i);
   assertNodes(parent,child);
   return SUCCESS;
}


/* see node.h for specification */
int Node_addChild(Node_T parent, const char* newNode, void* contents,
                  size_t length, nodeType type) {
   Node_T new;
   int result;

   assert(parent != NULL);
   assert(newNode != NULL);
   /* assert(CheckerDT_Node_isValid(parent)); */

   /* precautionary */
   if (type == ISFILE)
       new = Node_create(newNode, parent, contents, length, type);
   else if(type == ISDIRECTORY)
       new = Node_create(newNode, parent, NULL, 0,  type);

   if(new == NULL) {
      /* assert(CheckerDT_Node_isValid(parent)); */
      return PARENT_CHILD_ERROR;
   }
   result = Node_linkChild(parent, new);
   if(result != SUCCESS)
      (void) Node_destroy(new);
    /* else
        assert(CheckerDT_Node_isValid(new)); */

   /* assert(CheckerDT_Node_isValid(parent)); */
   return result;
}


/* see node.h for specification */
char* Node_toString(Node_T n) {
   char* copyPath;

   assert(n != NULL);

   copyPath = malloc(strlen(n->path)+1);
   if(copyPath == NULL) {
      return NULL;
   }
   else {
      return strcpy(copyPath, n->path);
   }
}

void* getFileContents(Node_T n) {
    assert(n != NULL);
    return (n->pvContents);
}

size_t getFileLength(Node_T n) {
    assert(n != NULL);
    return (n->uLength);
}

/* see node.h for specification */
boolean isFile(Node_t n){
    assert(n != NULL);
    return (n->type == ISFILE);
}

void* replaceFileContents(Node_T n, void *newContents, size_t newLength) {
    void* oldContents;
    assert(n != NULL);
    oldContents = pvContents;
    pvContents = newContents;
    uLength = newLength;
    return oldContents;
}