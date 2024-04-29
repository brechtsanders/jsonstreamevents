/**
 * @file jsonstreamevents.h
 * @brief jsonstreamevents library header file with main functions
 * @author Brecht Sanders
 *
 * This header file defines the functions needed for the basic library example
 */

#ifndef INCLUDED_JSONSTREAMEVENTS_H
#define INCLUDED_JSONSTREAMEVENTS_H

#include <stdlib.h>

/*! \cond PRIVATE */
#if !defined(DLL_EXPORT_JSONSTREAMEVENTS)
# if defined(_WIN32) && (defined(BUILD_JSONSTREAMEVENTS_DLL) || defined(JSONSTREAMEVENTS_EXPORTS))
#  define DLL_EXPORT_JSONSTREAMEVENTS __declspec(dllexport)
# elif /*defined(_WIN32)*/defined(__MINGW32__) && !defined(STATIC) && !defined(BUILD_JSONSTREAMEVENTS_STATIC) && !defined(BUILD_JSONSTREAMEVENTS)
#  define DLL_EXPORT_JSONSTREAMEVENTS __declspec(dllimport)
# else
#  define DLL_EXPORT_JSONSTREAMEVENTS
# endif
#endif
/*! \endcond */



#ifdef __cplusplus
extern "C" {
#endif



/*! \brief types of values
 * \sa     parser_type_get_name
 * \sa     parser_value_callback_fn
 */
enum parser_type_enum {
  type_null,
  type_text,
  type_number,
  type_boolean
};

/*! \brief get a human readable name for the specified type
 * \param  type          type
 * \return name of the type specified by \a type
 * \sa     enum parser_type_enum
 */
DLL_EXPORT_JSONSTREAMEVENTS const char* parser_type_get_name (enum parser_type_enum type);



/*! \cond PRIVATE */
struct jsonstreamevents_status_struct;
/*! \endcond */

/*! \brief function type used for callback when a value is found by the parser
 * \param  parserdata    pointer to parser data structure
 * \param  type          type of value found
 * \param  value         pointer to value (type depends on \a type)
 * \param  valuelen      length of value in case it is of \a type \a type_text, the boolean value (0 = false) in case \a type is \a type_boolean
 * \param  userdata      user data passed to \a jsonstreamevents_create_parser()
 * \return version string
 * \sa     parser_beginend_callback_fn
 * \sa     enum parser_type_enum
 * \sa     jsonstreamevents_create_parser()
 * \sa     jsonstreamevents_parse_data()
 * \sa     jsonstreamevents_parse_done()
 */
typedef int (*parser_value_callback_fn) (struct jsonstreamevents_status_struct* parserdata, enum parser_type_enum type, const void* value, size_t valuelen, void* userdata);

/*! \brief function type used for callback when the beginning or end of an object or array is found by the parser
 * \param  parserdata    pointer to parser data structure
 * \param  is_array      zero in case of object, non-zero in case of array
 * \param  userdata      user data passed to \a jsonstreamevents_create_parser()
 * \sa     parser_value_callback_fn
 * \sa     jsonstreamevents_create_parser()
 * \sa     jsonstreamevents_parse_data()
 * \sa     jsonstreamevents_parse_done()
 */
typedef int (*parser_beginend_callback_fn) (struct jsonstreamevents_status_struct* parserdata, int isarray, void* userdata);



/*! \brief structure for keeping current information while parsing
 * \sa     struct jsonstreamevents_node_struct
 */
struct jsonstreamevents_status_struct {
  char* property;
  struct jsonstreamevents_node_struct* parent;
};

/*! \brief structure for keeping parent information while parsing
 * \sa     struct jsonstreamevents_status_struct
 */
struct jsonstreamevents_node_struct {
  int isarray;
  char* name;
  struct jsonstreamevents_node_struct* parent;
};



/*! \cond PRIVATE */
#define JSONSTREAMEVENTS_COUNT_ARGS(...) (unsigned int)(sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*))
/*! \endcond */

/*! \brief match current parent chain
 * \param  parserdata    data structure with current parsing status
 * \param  nodechainlen  number of node names in \q nodechain
 * \param  nodechain     node names of parents (highest node first), written as (const char*[]){...})
 * \return zero on match
 * \sa     struct jsonstreamevents_status_struct
 * \sa     struct jsonstreamevents_node_struct
 * \sa     jsonstreamevents_parent_matches()
 * \sa     jsonstreamevents_property_matches()
 * \sa     jsonstreamevents_property_matches_chain()
 * \sa     jsonstreamevents_destroy_parser()
 * \sa     jsonstreamevents_parse_data()
 * \sa     jsonstreamevents_parse_done()
 */
DLL_EXPORT_JSONSTREAMEVENTS int jsonstreamevents_parent_matches_chain (struct jsonstreamevents_status_struct* parserdata, unsigned int nodechainlen, const char* nodechain[]);

/*! \brief match current parent chain
 * \param  parserdata    data structure with current parsing status
 * \param  ...           node names of parents (highest node first)
 * \return zero on match
 * \sa     struct jsonstreamevents_status_struct
 * \sa     struct jsonstreamevents_node_struct
 * \sa     jsonstreamevents_property_matches_chain()
 * \sa     jsonstreamevents_destroy_parser()
 * \sa     jsonstreamevents_parse_data()
 * \sa     jsonstreamevents_parse_done()
 */
#define jsonstreamevents_parent_matches(jsonstreamevents_property_matches,...) jsonstreamevents_parent_matches_chain(parserdata, JSONSTREAMEVENTS_COUNT_ARGS(__VA_ARGS__), (const char*[]){__VA_ARGS__})

/*! \brief match current property and parent chain
 * \param  parserdata    data structure with current parsing status
 * \param  nodechainlen  number of node names in \q nodechain
 * \param  nodechain     node names of parents (highest node first) followed by property name, written as (const char*[]){...})
 * \return zero on match
 * \sa     struct jsonstreamevents_status_struct
 * \sa     struct jsonstreamevents_node_struct
 * \sa     jsonstreamevents_property_matches()
 * \sa     jsonstreamevents_parent_matches()
 * \sa     jsonstreamevents_parent_matches_chain()
 * \sa     jsonstreamevents_destroy_parser()
 * \sa     jsonstreamevents_parse_data()
 * \sa     jsonstreamevents_parse_done()
 */
DLL_EXPORT_JSONSTREAMEVENTS int jsonstreamevents_property_matches_chain (struct jsonstreamevents_status_struct* parserdata, unsigned int nodechainlen, const char* nodechain[]);

/*! \brief match current property and parent chain
 * \param  parserdata    data structure with current parsing status
 * \param  ...           node names of parents (highest node first) followed by property name
 * \return zero on match
 * \sa     struct jsonstreamevents_status_struct
 * \sa     struct jsonstreamevents_node_struct
 * \sa     jsonstreamevents_parent_matches()
 * \sa     jsonstreamevents_property_matches_chain()
 * \sa     jsonstreamevents_parent_matches_chain()
 * \sa     jsonstreamevents_destroy_parser()
 * \sa     jsonstreamevents_parse_data()
 * \sa     jsonstreamevents_parse_done()
 */
#define jsonstreamevents_property_matches(jsonstreamevents_property_matches,...) jsonstreamevents_property_matches_chain(parserdata, JSONSTREAMEVENTS_COUNT_ARGS(__VA_ARGS__), (const char*[]){__VA_ARGS__})



/*! \brief type for handle used by jsonstreamevents_* functions
 * \sa     jsonstreamevents_create_parser()
 * \sa     jsonstreamevents_destroy_parser()
 * \sa     jsonstreamevents_parse_data()
 * \sa     jsonstreamevents_parse_done()
 */
typedef struct jsonstreamevents_handle_struct* jsonstreamevents_handle;

/*! \brief initialize json stream parser
 * \param  on_value      callback function to be called when a value is found by the parser
 * \param  on_begin      callback function to be when the beginning of an object or array is found by the parser
 * \param  on_end        callback function to be when the end of an object or array is found by the parser
 * \param  userdata      user data to be passed to callback functions
 * \return a handle to the created json stream parser or NULL on error
 * \sa     jsonstreamevents_handle
 * \sa     parser_value_callback_fn
 * \sa     parser_beginend_callback_fn
 * \sa     jsonstreamevents_destroy_parser()
 * \sa     jsonstreamevents_parse_data()
 * \sa     jsonstreamevents_parse_done()
 */
DLL_EXPORT_JSONSTREAMEVENTS jsonstreamevents_handle jsonstreamevents_create_parser (parser_value_callback_fn on_value, parser_beginend_callback_fn on_begin, parser_beginend_callback_fn on_end, void* userdata);

/*! \brief clean up json stream parser
 * \param  handle        handle to json stream parser
 * \sa     jsonstreamevents_handle
 * \sa     jsonstreamevents_create_parser()
 * \sa     jsonstreamevents_parse_data()
 * \sa     jsonstreamevents_parse_done()
 */
DLL_EXPORT_JSONSTREAMEVENTS void jsonstreamevents_destroy_parser (jsonstreamevents_handle handle);

/*! \brief parse part of json stream
 * \param  handle        handle to json stream parser
 * \param  buf           buffer to parse
 * \param  buflen        length of buffer to parse
 * \return NULL on success or an error message
 * \sa     jsonstreamevents_handle
 * \sa     parser_value_callback_fn
 * \sa     parser_beginend_callback_fn
 * \sa     jsonstreamevents_create_parser()
 * \sa     jsonstreamevents_destroy_parser()
 * \sa     jsonstreamevents_parse_done()
 */
DLL_EXPORT_JSONSTREAMEVENTS const char* jsonstreamevents_parse_data (jsonstreamevents_handle handle, char* buf, size_t buflen);

/*! \brief finish parsing json stream (must be called after last data was parsed)
 * \param  handle        handle to json stream parser
 * \return NULL on success or an error message
 * \sa     jsonstreamevents_handle
 * \sa     parser_value_callback_fn
 * \sa     parser_beginend_callback_fn
 * \sa     jsonstreamevents_create_parser()
 * \sa     jsonstreamevents_destroy_parser()
 * \sa     jsonstreamevents_parse_data()
 */
DLL_EXPORT_JSONSTREAMEVENTS const char* jsonstreamevents_parse_done (jsonstreamevents_handle handle);



/*! \brief version number constants
 * \sa     jsonstreamevents_get_version()
 * \sa     jsonstreamevents_get_version_string()
 * \name   JSONSTREAMEVENTS_VERSION_*
 * \{
 */
/*! \brief major version number */
#define JSONSTREAMEVENTS_VERSION_MAJOR 0
/*! \brief minor version number */
#define JSONSTREAMEVENTS_VERSION_MINOR 1
/*! \brief micro version number */
#define JSONSTREAMEVENTS_VERSION_MICRO 0
/*! @} */

/*! \brief packed version number */
#define JSONSTREAMEVENTS_VERSION (JSONSTREAMEVENTS_VERSION_MAJOR * 0x01000000 + JSONSTREAMEVENTS_VERSION_MINOR * 0x00010000 + JSONSTREAMEVENTS_VERSION_MICRO * 0x00000100)

/*! \cond PRIVATE */
#define JSONSTREAMEVENTS_VERSION_STRINGIZE_(major, minor, micro) #major"."#minor"."#micro
#define JSONSTREAMEVENTS_VERSION_STRINGIZE(major, minor, micro) JSONSTREAMEVENTS_VERSION_STRINGIZE_(major, minor, micro)
/*! \endcond */

/*! \brief string with dotted version number \hideinitializer */
#define JSONSTREAMEVENTS_VERSION_STRING JSONSTREAMEVENTS_VERSION_STRINGIZE(JSONSTREAMEVENTS_VERSION_MAJOR, JSONSTREAMEVENTS_VERSION_MINOR, JSONSTREAMEVENTS_VERSION_MICRO)

/*! \brief string with name of jsonstreamevents library */
#define JSONSTREAMEVENTS_NAME "jsonstreamevents"

/*! \brief string with name and version of jsonstreamevents library \hideinitializer */
#define JSONSTREAMEVENTS_FULLNAME JSONSTREAMEVENTS_NAME " " JSONSTREAMEVENTS_VERSION_STRING



/*! \brief get jsonstreamevents library version string
 * \param  pmajor        pointer to integer that will receive major version number
 * \param  pminor        pointer to integer that will receive minor version number
 * \param  pmicro        pointer to integer that will receive micro version number
 * \sa     jsonstreamevents_get_version_string()
 */
DLL_EXPORT_JSONSTREAMEVENTS void jsonstreamevents_get_version (int* pmajor, int* pminor, int* pmicro);

/*! \brief get jsonstreamevents library version string
 * \return version string
 * \sa     jsonstreamevents_get_version()
 */
DLL_EXPORT_JSONSTREAMEVENTS const char* jsonstreamevents_get_version_string ();

#ifdef __cplusplus
}
#endif

#endif
