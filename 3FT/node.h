/*--------------------------------------------------------------------*/
/* node.h                                                             */
/* Author: Alina Chen and Nickolas Casalinuovo                        */
/*--------------------------------------------------------------------*/

#ifndef NODE_INCLUDED
#define NODE_INCLUDED

#include <stddef.h>
#include "a4def.h"

/*
   a Node_T is an object that contains a path payload and references to
   the node's parent (if it exists) and children (if they exist).
*/
typedef struct node* Node_T;

typedef enum {ISDIRECTORY, ISFILE} nodeType;


/*
   Given a parent node, a string newNode, contents, a length, and type,
   returns a new Node_T or NULL if any allocation error occurs in
   creating the node or its fields.

   The new structure is initialized to have its path as the parent's
   path (if it exists) prefixed to the directory/file string parameter,
   separated by a slash. It is also initialized with its parent link
   as the parent parameter value, but the parent itself is not changed
   to link to the new node.  The children links are initialized but
   do not point to any children. It is also initialized with its
   contents as the contents parameter, if the type parameter is a file,
   and length with the length parameter. It is initialized with its type
   as the type parameter.
*/
Node_T Node_create(const char* newNode, Node_T parent, void* contents,
                   size_t length, nodeType type);

/*
  If the type is a file, destroys the file node n. If type is a directory,
  destroys the entire hierarchy of nodes rooted at n,
  including n itself.

  Returns the number of nodes destroyed.
*/
size_t Node_destroy(Node_T n, nodeType type);


/*
   Returns n's path.
*/
const char* Node_getPath(Node_T n);

/*
  Returns the number of child directories n has.
*/
size_t Node_getNumDirChildren(Node_T n);

/*
   Returns the number of child files n has.
*/
size_t Node_getNumFileChildren(Node_T n);

/*
   Returns the child directory node of n with identifier childID, if one exists,
   otherwise returns NULL.
*/
Node_T Node_getChildDirectory(Node_T n, size_t childID);

/*
   Returns the child file node of n with identifier childID, if one exists,
   otherwise returns NULL.
*/
Node_T Node_getChildFile(Node_T n, size_t childID);

/*
   Returns the parent node of n, if it exists, otherwise returns NULL
*/
Node_T Node_getParent(Node_T n);

/*
  Makes child a child of parent, if possible, and returns SUCCESS.
  This is not possible in the following cases:
  * child's path is not parent's path + / + directory,
    in which case returns PARENT_CHILD_ERROR
  * parent already has a child with child's path,
    in which case returns ALREADY_IN_TREE
  * parent is unable to allocate memory to store new child link,
    in which case returns MEMORY_ERROR
  * parent is FILE and child is DIRECTORY,
    in which case returns PARENT_CHILD_ERROR
 */
int Node_linkChild(Node_T parent, Node_T child);

/*
  Unlinks node parent from its child node child. child is unchanged.

  Returns PARENT_CHILD_ERROR if child is not a child of parent,
  and SUCCESS otherwise.
 */
int Node_unlinkChild(Node_T parent, Node_T child);

/*
  Creates a new node such that the new node's path is newNode appended to
  n's path, separated by a slash, and that the new node has no
  children of its own. The new node will have its own type based on the
  type parameter, as well as contents and length. The new node's parent
  is n, and the new node is added as a child of n.

  Returns SUCCESS upon completion, or:
  MEMORY_ERROR if the new node cannot be created,
  ALREADY_IN_TREE if parent already has a child with that path
*/
int Node_addChild(Node_T parent, const char* newNode, void* contents,
                  size_t length, nodeType type);

/*
  Returns a string representation for n, 
  or NULL if there is an allocation error.

  Allocates memory for the returned string,
  which is then owned by client!
*/
char* Node_toString(Node_T n);

/*
 Returns the contents of the node n, if n is a file.
 */
void* getFileContents(Node_T n);

/*
 Returns the file length of the node n, if n is a file.
 */
size_t getFileLength(Node_T n);

/* Given a node n, returns TRUE if n is a file node. Returns FALSE
 * if n is a directory node/
*/
boolean isFile(Node_T n);

/* Replaces the file contents */
void* replaceFileContents(Node_T n, void *newContents, size_t newLength);

nodeType getType(Node_T n);
#endif
