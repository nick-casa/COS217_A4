/*--------------------------------------------------------------------*/
/* checkerDT.c                                                        */
/* Author: Alina Chen and Nickolas Casalinuovo                        */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "dynarray.h"
#include "checkerDT.h"

/* see checkerDT.h for specification */
boolean CheckerDT_Node_isValid(Node_T n) {
   Node_T parent;
   const char* npath;
   const char* ppath;
   const char* rest;
   size_t i;


   /* Sample check: a ./NULL pointer is not a valid node */
   if(n == NULL) {
      fprintf(stderr, "A node is a NULL pointer\n");
      return FALSE;
   }

   parent = Node_getParent(n);
    if (Node_getPath(n) == NULL) {
        fprintf(stderr, "Node has no path\n");
        return FALSE;
    }

   if(parent != NULL) {
       npath = Node_getPath(n);
        /* Sample check that parent's path must be prefix of n's path */
       ppath = Node_getPath(parent);

       if (npath==NULL || ppath == NULL) {
           fprintf(stderr, "Node has no path\n");
           return FALSE;
       }

      i = strlen(ppath);
      if(strncmp(npath, ppath, i)) {
          fprintf(stderr, "P's path is not a prefix of C's path\n");
          return FALSE;
      }

      if (i > strlen(npath)) {
          fprintf(stderr, "P's path longer than C's path\n");
          return FALSE;
      }

      /* Sample check that n's path after parent's path + '/'
         must have no further '/' characters */
      rest = npath + i;
      rest++;
      if(strstr(rest, "/") != NULL) {
         fprintf(stderr, "C's path has grandchild of P's path\n");
         return FALSE;
      }



   }


   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at n.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise.

   Takes in a pointer to numNodes which it increments.

   You may want to change this function's return type or
   parameter list to facilitate constructing your checks.
   If you do, you should update this function comment.
*/
static boolean CheckerDT_treeCheck(Node_T n, size_t *numNodes) {
   size_t c;
   assert(numNodes != NULL);
   (*numNodes)++;
   if(n != NULL) {

      /* Sample check on each non-root node: node must be valid */
      /* If not, pass that failure back up immediately */
      if(!CheckerDT_Node_isValid(n)){
          fprintf(stderr, "One or more node(s) is not valid\n");
          return FALSE;
      }



      for(c = 0; c < Node_getNumChildren(n); c++){
          Node_T child = Node_getChild(n, c);


          if (Node_getNumChildren(n) >= 1 && Node_getChild(n, 0) == NULL){
                  fprintf(stderr, "Child at wrong location. \n");
                  return FALSE;
          }

          if(strcmp(Node_getPath(n),Node_getPath(Node_getParent(child)))){
             fprintf(stderr, "Parent of child is not current node.\n");
             return FALSE;
          }
          if(child == NULL){
              fprintf(stderr, "Null Child.\n");
              return FALSE;
          }
          if(c > 0){
              Node_T lastChild = Node_getChild(n, c-1);
              if(strcmp(Node_getPath(lastChild), Node_getPath(child)) >= 0){
                  fprintf(stderr, "Children are not in alphabetical order.\n");
                  return FALSE;
              }
          }
          /* if recurring down one subtree results in a failed check
             farther down, passes the failure back up immediately */
          if(!CheckerDT_treeCheck(child, numNodes)){
              fprintf(stderr, "Failed recurring down subtree\n");
              return FALSE;
          }
      }
   }
   return TRUE;
}

/* see checkerDT.h for specification */
boolean CheckerDT_isValid(boolean isInit, Node_T root, size_t count) {
    size_t numNodes;
    boolean treeIsValid;

    numNodes = 0;
    /* Sample check on a top-level data structure invariant:
       if the DT is not initialized, its count should be 0. */
    if (!isInit) {
        if (count != 0) {
            fprintf(stderr, "Not initialized, but count is not 0\n");
            return FALSE;
        }
        if (root != NULL) {
            fprintf(stderr, "Not initialized, but root is not NULL\n");
            return FALSE;
        }
    }

    /* Check on when DT is in an initialized state */
    if (isInit) {
        /* if the root does not equal null */
        if (root != NULL){
            /* but the count equals 0 */
            if (count == 0) {
                fprintf(stderr, "Initialized, has a node but count is 0\n");
                return FALSE;
            }
            /* or the parent of the root is not null */
            if(Node_getParent(root) != NULL){
                fprintf(stderr, "Parent of root is not null.\n");
                return FALSE;
            }
        }
        if (root == NULL && count != 0) {
            fprintf(stderr, "Initialized, has no nodes but count isn't 0\n");
            return FALSE;
        }
    }


    /* Now checks invariants recursively at each node from the root. */
    treeIsValid = CheckerDT_treeCheck(root, &numNodes);
    if(!treeIsValid) return FALSE;

    /* Check that the amount of nodes is equal to the count */
    if(count == 0 && numNodes == 1){
       return TRUE;
    }
    else if (numNodes != count) {
       fprintf(stderr, "The number of nodes is not equal to the count.\n");
       return FALSE;
    }

    return TRUE;
}
