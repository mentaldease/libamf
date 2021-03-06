#ifndef _AMF3_H
#   define _AMF3_H

#include <stdint.h>
#include "endian.h"
#include "list.h"

#define AMF3_UNDEFINED	(0x00)
#define AMF3_NULL	(0x01)
#define AMF3_FALSE	(0x02)
#define AMF3_TRUE	(0x03)
#define AMF3_INTEGER	(0x04)
#define AMF3_DOUBLE	(0x05)
#define AMF3_STRING	(0x06)
#define AMF3_XMLDOC	(0x07)
#define AMF3_DATE	(0x08)
#define AMF3_ARRAY	(0x09)
#define AMF3_OBJECT	(0x0A)
#define AMF3_XML	(0x0B)
#define AMF3_BYTEARRAY	(0x0C)

/* types for internal use */
#define AMF3_TRAITS	(0x70)


struct amf3_value;
struct amf3_vlist;
struct amf3_kvlist;

struct amf3_date {
    double value;
};

struct amf3_array {
    List assoc_list;
    List dense_list;
};

struct amf3_traits {
    struct amf3_value *type;
    char externalizable;
    char dynamic;
    int nmemb;
    struct amf3_value **members;
};

struct amf3_object {
    struct amf3_value *traits;
    union {
	struct {
	    struct amf3_value **member_values;
	    List dynmemb_list;
	} i;
	void *external_ctx;
    } m;
};

struct amf3_binary {
    int length;
    char *data;
};

struct amf3_value {
    int retain_count;
    char type;
    union {
	int			integer;
	double			real;
	struct amf3_date	date;
	struct amf3_array	array;
	struct amf3_object	object;
	struct amf3_binary	binary;

	/* for internal use only */
	struct amf3_traits	traits;
    } v;
};

struct amf3_kv {
    struct amf3_value *key;
    struct amf3_value *value;
};

struct amf3_valfind {
    struct amf3_value *value;
    int idx;
};

#define AMF3_REF_TABLE_PREALLOC (128)
struct amf3_ref_table {
    struct amf3_value **refs;
    int nref;
    int nalloc;
};

struct amf3_parse_context {
    const char *data;
    int length;
    const char *p;
    int left;
    struct amf3_ref_table *object_refs;
    struct amf3_ref_table *string_refs;
    struct amf3_ref_table *traits_refs;
};

struct amf3_serialize_context {
    char *buffer;
    int allocated;
    int length;
    struct amf3_ref_table *object_refs;
    struct amf3_ref_table *string_refs;
    struct amf3_ref_table *traits_refs;
};

typedef struct amf3_value *AMF3Value;
typedef struct amf3_parse_context *AMF3ParseContext;
typedef struct amf3_serialize_context *AMF3SerializeContext;
/* returns 0 if success; otherwise, failed. */
typedef int (* AMF3PluginParserParseFunc) (
	AMF3ParseContext c, AMF3Value classname, void **external_ctx);
typedef void (* AMF3PluginExternalObjectFreeFunc) (void *external_ctx);
typedef void (* AMF3PluginExternalObjectDumpFunc) (void *external_ctx, int depth);
typedef int  (* AMF3PluginExternalObjectWriteFunc) (
	AMF3SerializeContext c, const void *data, int len);
typedef int  (* AMF3PluginExternalObjectSerializeFunc) (
	AMF3SerializeContext c, AMF3Value classname, void *external_ctx);

struct amf3_plugin_parser {
    char *classname;
    AMF3PluginParserParseFunc handler;
    AMF3PluginExternalObjectFreeFunc freefunc;
    AMF3PluginExternalObjectDumpFunc dumpfunc;
    AMF3PluginExternalObjectSerializeFunc serializefunc;
};

AMF3Value amf3_retain(AMF3Value v);
void amf3_release(AMF3Value v);
AMF3Value amf3_new_undefined();
AMF3Value amf3_new_null();
AMF3Value amf3_new_false();
AMF3Value amf3_new_true();
AMF3Value amf3_new_integer(int value);
AMF3Value amf3_new_double(double value);
AMF3Value amf3_new_string(const char *string, int length);
AMF3Value amf3_new_string_utf8(const char *utf8);
AMF3Value amf3_new_xmldoc(const char *doc, int length);
AMF3Value amf3_new_xml(const char *doc, int length);
AMF3Value amf3_new_bytearray(const char *bytes, int length);
AMF3Value amf3_new_date(double date);
AMF3Value amf3_new_array();
AMF3Value amf3_new_object(AMF3Value type, char dynamic,
	AMF3Value *member_names, int nmemb);
AMF3Value amf3_new_object_external(AMF3Value type, void *external_ctx);

int amf3_string_cmp(AMF3Value a, AMF3Value b);
int amf3_string_len(AMF3Value v);
const char *amf3_string_cstr(AMF3Value v);

int amf3_binary_len(AMF3Value v);
const char *amf3_binary_data(AMF3Value v);

void amf3_array_push(AMF3Value a, AMF3Value v);
void amf3_array_assoc_set(AMF3Value a, AMF3Value key, AMF3Value value);
AMF3Value amf3_array_assoc_get(AMF3Value a, AMF3Value key);

AMF3Value amf3_object_traits_get(AMF3Value o);
void *amf3_object_external_get(AMF3Value o);
AMF3Value amf3_object_prop_get(AMF3Value o, AMF3Value key);
void amf3_object_prop_set(AMF3Value o, AMF3Value key, AMF3Value value);

AMF3Value amf3_traits_type_get(AMF3Value o);
int amf3_traits_is_externalizable(AMF3Value o);
int amf3_traits_is_dynamic(AMF3Value o);
int amf3_traits_num_members(AMF3Value o);
AMF3Value amf3_traits_member_name_get(AMF3Value o, int idx);

struct amf3_ref_table *amf3_ref_table_new();
void amf3_ref_table_free(struct amf3_ref_table *r);
AMF3Value amf3_ref_table_push(struct amf3_ref_table *r, AMF3Value v);
AMF3Value amf3_ref_table_get(struct amf3_ref_table *r, int idx);

int amf3_parse_u29(struct amf3_parse_context *c);
AMF3Value amf3_parse_string(struct amf3_parse_context *c);
AMF3Value amf3_parse_array(struct amf3_parse_context *c);
AMF3Value amf3_parse_object(struct amf3_parse_context *c);
AMF3Value amf3_parse_value(struct amf3_parse_context *c);

AMF3ParseContext amf3_parse_context_new(const char *data, int length);
void amf3_parse_context_free(AMF3ParseContext c);

void amf3_dump_value(AMF3Value v, int depth);
void amf3__print_indent(int indent);

AMF3SerializeContext amf3_serialize_context_new();
void amf3_serialize_context_free(AMF3SerializeContext c);
const char *amf3_serialize_context_get_buffer(AMF3SerializeContext c, int *len);
int amf3_serialize_write_func(AMF3SerializeContext c, const void *data, int len);
int amf3_serialize_u29(AMF3SerializeContext c, int integer);
int amf3_serialize_value(AMF3SerializeContext c, AMF3Value v);

#endif
