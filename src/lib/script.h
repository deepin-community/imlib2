#ifndef _DYN_FUNCTION_H_
#define _DYN_FUNCTION_H_

#include <stdarg.h>
#include "common.h"
#include "image.h"

#define VAR_CHAR 1
#define VAR_PTR  2

#define ASSIGN_DATA8( var, v ) if( strcmp( ptr->key, var ) == 0 ) v = (DATA8)atoi( (char *)ptr->data )
#define ASSIGN_INT(k, v)                           \
       if (!strcmp((k), ptr->key)) {                 \
	  if (ptr->type == VAR_PTR) {                \
	     (v) = (*(int *)ptr->data);              \
	  } else if (ptr->type == VAR_CHAR) {        \
	     (v) = strtol(ptr->data, 0, 0);          \
	  }                                          \
       }

#define ASSIGN_IMAGE(k, v)                         \
   if (!strcmp((k), ptr->key)) {                   \
      if (ptr->type == VAR_PTR) {                  \
	 (v) = ((Imlib_Image)ptr->data);           \
      } else if (ptr->type == VAR_CHAR) {          \
	 if (!free_map)                            \
	   (v) = imlib_load_image(ptr->data);      \
	 free_map = 1;                             \
      }                                            \
   }

typedef struct _imlib_function_param IFunctionParam;
typedef struct _imlib_function_param *pIFunctionParam;
struct _imlib_function_param {
   char               *key;
   int                 type;
   void               *data;
   pIFunctionParam     next;
};

typedef struct _imlib_function IFunction;
typedef struct _imlib_function *pIFunction;
struct _imlib_function {
   char               *name;
   pIFunctionParam     params;
   pIFunction          next;
};

typedef struct _imlib_variable {
   void               *ptr;
   struct _imlib_variable *next;
} IVariable;

ImlibImage         *__imlib_script_parse(ImlibImage * im, const char *script,
                                         va_list);
IFunctionParam     *__imlib_script_parse_parameters(ImlibImage * im,
                                                    const char *parameters);
ImlibImage         *__imlib_script_parse_function(ImlibImage * im,
                                                  const char *function);
void                __imlib_script_tidyup(void);
void               *__imlib_script_get_next_var(void);
void                __imlib_script_add_var(void *ptr);

#endif /* _FUNCTION_H_ */
