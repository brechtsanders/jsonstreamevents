#include "jsonstreamevents.h"
#include <string.h>
#include <stdio.h>
#if defined(USE_LIBYAJL)
#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>
#elif defined(LIBLAXJSON)
#include <laxjson.h>
#else
#error No JSON parsing library selected, make sure to define either USE_LIBYAJL or LIBLAXJSON
#endif

DLL_EXPORT_JSONSTREAMEVENTS const char* parser_type_get_name (enum parser_type_enum type)
{
  switch (type) {
    case type_null:    return "null";
    case type_text:    return "text";
    case type_number:  return "number";
    case type_boolean: return "boolean";
    default:           return "(invalid type)";
  }
}



struct jsonstreamevents_handle_struct {
  struct jsonstreamevents_status_struct parserdata;
  parser_value_callback_fn on_value;
  parser_beginend_callback_fn on_begin;
  parser_beginend_callback_fn on_end;
  void* userdata;
#if defined(USE_LIBYAJL)
  yajl_handle jsonparser;
  unsigned char* errmsg;
#elif defined(LIBLAXJSON)
  struct LaxJsonContext* jsonparser;
  char* errmsg;
#endif
};

typedef struct jsonstreamevents_handle_struct* jsonstreamevents_handle;



static inline void parser_data_set_property (struct jsonstreamevents_handle_struct* handle, const char* propertyname, size_t propertylen)
{
  if (handle->parserdata.property)
    free(handle->parserdata.property);
  if (!propertyname) {
    handle->parserdata.property = NULL;
  } else if ((handle->parserdata.property = (char*)malloc(propertylen + 1)) != NULL) {
    memcpy(handle->parserdata.property, propertyname, propertylen);
    handle->parserdata.property[propertylen] = 0;
  }
}

static inline void parser_data_clear_property (struct jsonstreamevents_handle_struct* handle)
{
  if (handle->parserdata.property)
    free(handle->parserdata.property);
  handle->parserdata.property = NULL;
}

static inline void parser_data_set_parent (struct jsonstreamevents_handle_struct* handle, int isarray)
{
  struct jsonstreamevents_node_struct* newparent;
  //set current node to parent node
  if ((newparent = (struct jsonstreamevents_node_struct*)malloc(sizeof(struct jsonstreamevents_node_struct))) != NULL) {
    newparent->isarray = isarray;
    newparent->name = (handle->parserdata.property ? strdup(handle->parserdata.property) : NULL);
    newparent->parent = handle->parserdata.parent;
    handle->parserdata.parent = newparent;
  }
  //clear current property name
  parser_data_clear_property(handle);
}

static inline void parser_data_remove_parent (struct jsonstreamevents_status_struct* parserdata)
{
  struct jsonstreamevents_node_struct* oldparent;
  //remove current parent node
  if (parserdata->parent) {
    oldparent = parserdata->parent;
    parserdata->parent = parserdata->parent->parent;
    free(oldparent->name);
    free(oldparent);
  }
}



int jsonstreamevents_parent_check_match (struct jsonstreamevents_node_struct* parent, unsigned int nodechainlen, const char* nodechain[], unsigned int pos)
{
  if (pos >= nodechainlen)
    return -1;
  if (nodechainlen == 0)
    return 0;
  if (pos > 0) {
    int result;
    if (!parent->parent)
      return -1;
    if ((result = jsonstreamevents_parent_check_match(parent->parent, nodechainlen, nodechain, pos - 1)) != 0)
      return result;
  }
  if (!parent || parent->name == NULL)
    return (nodechain[pos] == NULL ? 0 : 1);
  if (nodechain[pos] == NULL)
    return 1;
  return strcmp(parent->name, nodechain[pos]);
}

DLL_EXPORT_JSONSTREAMEVENTS int jsonstreamevents_parent_matches_chain (struct jsonstreamevents_status_struct* parserdata, unsigned int nodechainlen, const char* nodechain[])
{
  int result;
  if (nodechainlen < 1)
    return -1;
  result = jsonstreamevents_parent_check_match(parserdata->parent, nodechainlen, nodechain, nodechainlen - 1);
  return result;
}

//usage example: if (jsonstreamevents_property_matches(parserdata, 3, (const char*[]){NULL, "owner", "login"}) == 0) printf("*owner login\n");
DLL_EXPORT_JSONSTREAMEVENTS int jsonstreamevents_property_matches_chain (struct jsonstreamevents_status_struct* parserdata, unsigned int nodechainlen, const char* nodechain[])
{
  int result;
  if (nodechainlen < 1)
    return -1;
  if (parserdata->property == NULL) {
    if (nodechain[nodechainlen - 1] != NULL)
      return 1;
  } else if (nodechain[nodechainlen - 1] == NULL) {
    return 1;
  } else {
    if ((result = strcmp(parserdata->property, nodechain[nodechainlen - 1])) != 0)
      return result;
  }
  if (nodechainlen > 1) {
    if (!parserdata->parent)
      return -1;
    result = jsonstreamevents_parent_check_match(parserdata->parent, nodechainlen, nodechain, nodechainlen - 2);
  }
  return result;
}



#if defined(USE_LIBYAJL)

static int on_json_null (void* userdata)
{
  int result = 0;
  struct jsonstreamevents_handle_struct* handle = (struct jsonstreamevents_handle_struct*)userdata;
  if (handle->on_value)
    result = (*handle->on_value)(&handle->parserdata, type_null, NULL, 0, handle->userdata);
  parser_data_clear_property(handle);
  return (result == 0 ? 1 : 0);
}

static int on_json_boolean(void * userdata, int value)
{
  int result = 0;
  struct jsonstreamevents_handle_struct* handle = (struct jsonstreamevents_handle_struct*)userdata;
  if (handle->on_value)
    result = (*handle->on_value)(&handle->parserdata, type_boolean, NULL, (value ? 1 : 0), handle->userdata);
  parser_data_clear_property(handle);
  return (result == 0 ? 1 : 0);
}

static int on_json_integer (void* userdata, long long value)
{
  int result = 0;
  struct jsonstreamevents_handle_struct* handle = (struct jsonstreamevents_handle_struct*)userdata;
  if (handle->on_value) {
    double doublevalue = (double)value;
    result = (*handle->on_value)(&handle->parserdata, type_number, &doublevalue, sizeof(doublevalue), handle->userdata);
  }
  parser_data_clear_property(handle);
  return (result == 0 ? 1 : 0);
}

static int on_json_double (void* userdata, double value)
{
  int result = 0;
  struct jsonstreamevents_handle_struct* handle = (struct jsonstreamevents_handle_struct*)userdata;
  if (handle->on_value)
    result = (*handle->on_value)(&handle->parserdata, type_number, &value, sizeof(value), handle->userdata);
  parser_data_clear_property(handle);
  return (result == 0 ? 1 : 0);
}

static int on_json_string (void* userdata, const unsigned char* value, size_t len)
{
  int result = 0;
  struct jsonstreamevents_handle_struct* handle = (struct jsonstreamevents_handle_struct*)userdata;
  if (handle->on_value)
    result = (*handle->on_value)(&handle->parserdata, type_text, value, len, handle->userdata);
  parser_data_clear_property(handle);
  return (result == 0 ? 1 : 0);
}

static int on_json_map_key (void* userdata, const unsigned char* value, size_t len)
{
  struct jsonstreamevents_handle_struct* handle = (struct jsonstreamevents_handle_struct*)userdata;
  parser_data_set_property(handle, (char*)value, len);
  return 1;
}

static int on_json_start_map (void* userdata)
{
  int result = 0;
  struct jsonstreamevents_handle_struct* handle = (struct jsonstreamevents_handle_struct*)userdata;
  if (handle->on_begin)
    result = (*handle->on_begin)(&handle->parserdata, 0, handle->userdata);
  parser_data_set_parent(handle, 0);
  return (result == 0 ? 1 : 0);
}


static int on_json_end_map (void* userdata)
{
  int result = 0;
  struct jsonstreamevents_handle_struct* handle = (struct jsonstreamevents_handle_struct*)userdata;
  if (handle->on_end && handle->parserdata.parent && handle->parserdata.parent->name) {
    handle->parserdata.property = handle->parserdata.parent->name;
    handle->parserdata.parent->name = NULL;
  }
  parser_data_remove_parent(&handle->parserdata);
  if (handle->on_end) {
    result = (*handle->on_end)(&handle->parserdata, 0, handle->userdata);
    parser_data_clear_property(handle);
  }
  parser_data_clear_property(handle);
  return (result == 0 ? 1 : 0);
}

static int on_json_start_array (void* userdata)
{
  int result = 0;
  struct jsonstreamevents_handle_struct* handle = (struct jsonstreamevents_handle_struct*)userdata;
  if (handle->on_begin)
    result = (*handle->on_begin)(&handle->parserdata, 1, handle->userdata);
  parser_data_set_parent(handle, 1);
  return (result == 0 ? 1 : 0);
}

static int on_json_end_array (void* userdata)
{
  int result = 0;
  struct jsonstreamevents_handle_struct* handle = (struct jsonstreamevents_handle_struct*)userdata;
  if (handle->on_end && handle->parserdata.parent && handle->parserdata.parent->name) {
    handle->parserdata.property = handle->parserdata.parent->name;
    handle->parserdata.parent->name = NULL;
  }
  parser_data_remove_parent(&handle->parserdata);
  if (handle->on_end)
    result = (*handle->on_end)(&handle->parserdata, 1, handle->userdata);
  parser_data_clear_property(handle);
  return (result == 0 ? 1 : 0);
}

static yajl_callbacks json_callbacks = {
    on_json_null,
    on_json_boolean,
    on_json_integer,
    on_json_double,
    NULL,
    on_json_string,
    on_json_start_map,
    on_json_map_key,
    on_json_end_map,
    on_json_start_array,
    on_json_end_array
};

#elif defined(LIBLAXJSON)

static int on_json_string(struct LaxJsonContext* context, enum LaxJsonType type, const char *value, int len)
{
  int result = 0;
  struct jsonstreamevents_handle_struct* handle = (struct jsonstreamevents_handle_struct*)context->userdata;
  if (type == LaxJsonTypeProperty) {
    parser_data_set_property(handle, value, len);
  } else {
    if (handle->on_value)
      result = (*handle->on_value)(&handle->parserdata, type_text, value, len, handle->userdata);
    parser_data_clear_property(handle);
  }
  return result;
}

static int on_json_number(struct LaxJsonContext* context, double value)
{
  int result = 0;
  struct jsonstreamevents_handle_struct* handle = (struct jsonstreamevents_handle_struct*)context->userdata;
  if (handle->on_value)
    result = (*handle->on_value)(&handle->parserdata, type_number, &value, sizeof(value), handle->userdata);
  parser_data_clear_property(handle);
  return result;
}

static int on_json_primitive(struct LaxJsonContext* context, enum LaxJsonType type)
{
  int result = 0;
  struct jsonstreamevents_handle_struct* handle = (struct jsonstreamevents_handle_struct*)context->userdata;
  if (handle->on_value) {
    if (type == LaxJsonTypeFalse)
      result = (*handle->on_value)(&handle->parserdata, type_boolean, NULL, 0, handle->userdata);
    else if (type == LaxJsonTypeTrue)
      result = (*handle->on_value)(&handle->parserdata, type_boolean, NULL, 1, handle->userdata);
    else
      result = (*handle->on_value)(&handle->parserdata, type_null, NULL, 0, handle->userdata);
  }
  parser_data_clear_property(handle);
  return result;
}

static int on_json_begin(struct LaxJsonContext* context, enum LaxJsonType type)
{
  int result = 0;
  struct jsonstreamevents_handle_struct* handle = (struct jsonstreamevents_handle_struct*)context->userdata;
  if (handle->on_begin)
    result = (*handle->on_begin)(&handle->parserdata, (type == LaxJsonTypeArray ? 1 : 0), handle->userdata);
  parser_data_set_parent(handle, (type == LaxJsonTypeArray ? 1 : 0));
  return result;
}

static int on_json_end(struct LaxJsonContext* context, enum LaxJsonType type)
{
  int result = 0;
  struct jsonstreamevents_handle_struct* handle = (struct jsonstreamevents_handle_struct*)context->userdata;
  if (handle->on_end && handle->parserdata.parent && handle->parserdata.parent->name) {
    handle->parserdata.property = handle->parserdata.parent->name;
    handle->parserdata.parent->name = NULL;
  }
  parser_data_remove_parent(&handle->parserdata);
  if (handle->on_end)
    result = (*handle->on_end)(&handle->parserdata, (type == LaxJsonTypeArray ? 1 : 0), handle->userdata);
  parser_data_clear_property(handle);
  return result;
}

#endif



DLL_EXPORT_JSONSTREAMEVENTS jsonstreamevents_handle jsonstreamevents_create_parser (parser_value_callback_fn on_value, parser_beginend_callback_fn on_begin, parser_beginend_callback_fn on_end, void* userdata)
{
  struct jsonstreamevents_handle_struct* handle;
  if ((handle = (struct jsonstreamevents_handle_struct*)malloc(sizeof(struct jsonstreamevents_handle_struct))) != NULL) {
    handle->parserdata.property = NULL;
    handle->parserdata.parent = NULL;
    handle->on_value = on_value;
    handle->on_begin = on_begin;
    handle->on_end = on_end;
    handle->userdata = userdata;
#if defined(USE_LIBYAJL)
    handle->jsonparser = yajl_alloc(&json_callbacks, NULL, &handle->parserdata);
    handle->errmsg = NULL;
#elif defined(LIBLAXJSON)
    handle->jsonparser = lax_json_create();
    handle->jsonparser->userdata = &handle->parserdata;
    handle->jsonparser->string = on_json_string;
    handle->jsonparser->number = on_json_number;
    handle->jsonparser->primitive = on_json_primitive;
    handle->jsonparser->begin = on_json_begin;
    handle->jsonparser->end = on_json_end;
    handle->errmsg = NULL;
#endif
  }
  return handle;
}

DLL_EXPORT_JSONSTREAMEVENTS void jsonstreamevents_destroy_parser (jsonstreamevents_handle handle)
{
  if (handle) {
    if (handle->parserdata.property)
      free(handle->parserdata.property);
#if defined(USE_LIBYAJL)
    yajl_free(handle->jsonparser);
    if (handle->errmsg)
      yajl_free_error(handle->jsonparser, handle->errmsg);
#elif defined(LIBLAXJSON)
    lax_json_destroy(handle->jsonparser);
    if (handle->errmsg)
      free(handle->errmsg);
#endif
    free(handle);
  }
}

DLL_EXPORT_JSONSTREAMEVENTS const char* jsonstreamevents_parse_data (jsonstreamevents_handle handle, char* buf, size_t buflen)
{
  char* result = NULL;
#if defined(USE_LIBYAJL)
  yajl_status status;
  if ((status = yajl_parse(handle->jsonparser, (unsigned char*)buf, buflen)) != yajl_status_ok) {
    if (handle->errmsg) {
      yajl_free_error(handle->jsonparser, handle->errmsg);
      handle->errmsg = NULL;
    }
    handle->errmsg = yajl_get_error(handle->jsonparser, 1, (unsigned char*)buf, buflen);
    result = (char*)handle->errmsg;
  }
#elif defined(LIBLAXJSON)
  enum LaxJsonError status;
  if ((status = lax_json_feed(handle->jsonparser, buflen, buf)) != 0) {
    const char* errmsg = lax_json_str_err(status);
    size_t errmsglen = snprintf(NULL, 0, "Line %d, column %d: %s\n", handle->jsonparser->line, handle->jsonparser->column, errmsg);
    if (handle->errmsg) {
      free(handle->errmsg);
      handle->errmsg = NULL;
    }
    if ((handle->errmsg = (char*)malloc(errmsglen + 1)) != NULL) {
      snprintf(handle->errmsg, errmsglen + 1, "Line %d, column %d: %s\n", handle->jsonparser->line, handle->jsonparser->column, errmsg);
      result = handle->errmsg;
    }
  }
#endif
  return result;
}

DLL_EXPORT_JSONSTREAMEVENTS const char* jsonstreamevents_parse_done (jsonstreamevents_handle handle)
{
  char* result = NULL;
#if defined(USE_LIBYAJL)
  yajl_status status;
  if ((status = yajl_complete_parse(handle->jsonparser)) != yajl_status_ok) {
    if (handle->errmsg) {
      yajl_free_error(handle->jsonparser, handle->errmsg);
      handle->errmsg = NULL;
    }
    handle->errmsg = yajl_get_error(handle->jsonparser, 1, NULL, 0);
  }
#elif defined(LIBLAXJSON)
  enum LaxJsonError status;
  if ((status = lax_json_eof(handle->jsonparser)) != 0) {
    const char* errmsg = lax_json_str_err(status);
    size_t errmsglen = snprintf(NULL, 0, "Line %d, column %d: %s\n", handle->jsonparser->line, handle->jsonparser->column, errmsg);
    if (handle->errmsg) {
      free(handle->errmsg);
      handle->errmsg = NULL;
    }
    if ((handle->errmsg = (char*)malloc(errmsglen + 1)) != NULL) {
      snprintf(handle->errmsg, errmsglen + 1, "Line %d, column %d: %s\n", handle->jsonparser->line, handle->jsonparser->column, errmsg);
      result = handle->errmsg;
    }
  }
#endif
  return result;
}



DLL_EXPORT_JSONSTREAMEVENTS void jsonstreamevents_get_version (int* pmajor, int* pminor, int* pmicro)
{
  if (pmajor)
    *pmajor = JSONSTREAMEVENTS_VERSION_MAJOR;
  if (pminor)
    *pminor = JSONSTREAMEVENTS_VERSION_MINOR;
  if (pmicro)
    *pmicro = JSONSTREAMEVENTS_VERSION_MICRO;
}

DLL_EXPORT_JSONSTREAMEVENTS const char* jsonstreamevents_get_version_string ()
{
  return JSONSTREAMEVENTS_VERSION_STRING;
}
