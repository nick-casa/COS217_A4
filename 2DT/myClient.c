#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dt.h"

int main(void) {
   char* temp;

   assert(DT_init() == SUCCESS);
   assert(DT_insertPath("a/b/c/d") == SUCCESS);
   assert(DT_rmPath("a/b/c") == SUCCESS);
   assert(DT_insertPath("a/e") == SUCCESS);
   assert(DT_insertPath("a/f") == SUCCESS);
   assert((temp = DT_toString()) != NULL);
   fprintf(stderr, "%s\n", temp);
   free(temp);
   assert(DT_destroy() == SUCCESS);

   return 0;
}
