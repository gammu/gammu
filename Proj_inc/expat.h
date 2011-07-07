/* Copyright (c) 1998, 1999, 2000 Thai Open Source Software Center Ltd
   See the file COPYING for copying permission.
*/

#ifndef XmlParse_INCLUDED
#define XmlParse_INCLUDED 1

#ifdef __VMS
/*      0        1         2         3      0        1         2         3
        1234567890123456789012345678901     1234567890123456789012345678901 */
#define ANWXML_SetProcessingInstructionHandler ANWXML_SetProcessingInstrHandler
#define ANWXML_SetUnparsedEntityDeclHandler    ANWXML_SetUnparsedEntDeclHandler
#define ANWXML_SetStartNamespaceDeclHandler    ANWXML_SetStartNamespcDeclHandler
#define ANWXML_SetExternalEntityRefHandlerArg  ANWXML_SetExternalEntRefHandlerArg
#endif

#include <stdlib.h>
#include "expat_external.h"

struct ANWXML_ParserStruct;
typedef struct ANWXML_ParserStruct *ANWXML_Parser;

/* Should this be defined using stdbool.h when C99 is available? */
typedef unsigned char ANWXML_Bool;
#define ANWXML_TRUE   ((ANWXML_Bool) 1)
#define ANWXML_FALSE  ((ANWXML_Bool) 0)

/* The ANWXML_Status enum gives the possible return values for several
   API functions.  The preprocessor #defines are included so this
   stanza can be added to code that still needs to support older
   versions of Expat 1.95.x:

   #ifndef ANWXML_STATUS_OK
   #define ANWXML_STATUS_OK    1
   #define ANWXML_STATUS_ERROR 0
   #endif

   Otherwise, the #define hackery is quite ugly and would have been
   dropped.
*/
enum ANWXML_Status {
  ANWXML_STATUS_ERROR = 0,
#define ANWXML_STATUS_ERROR ANWXML_STATUS_ERROR
  ANWXML_STATUS_OK = 1,
#define ANWXML_STATUS_OK ANWXML_STATUS_OK
  ANWXML_STATUS_SUSPENDED = 2,
#define ANWXML_STATUS_SUSPENDED ANWXML_STATUS_SUSPENDED
};

enum ANWXML_Error {
  ANWXML_ERROR_NONE,
  ANWXML_ERROR_NO_MEMORY,
  ANWXML_ERROR_SYNTAX,
  ANWXML_ERROR_NO_ELEMENTS,
  ANWXML_ERROR_INVALID_TOKEN,
  ANWXML_ERROR_UNCLOSED_TOKEN,
  ANWXML_ERROR_PARTIAL_CHAR,
  ANWXML_ERROR_TAG_MISMATCH,
  ANWXML_ERROR_DUPLICATE_ATTRIBUTE,
  ANWXML_ERROR_JUNK_AFTER_DOC_ELEMENT,
  ANWXML_ERROR_PARAM_ENTITY_REF,
  ANWXML_ERROR_UNDEFINED_ENTITY,
  ANWXML_ERROR_RECURSIVE_ENTITY_REF,
  ANWXML_ERROR_ASYNC_ENTITY,
  ANWXML_ERROR_BAD_CHAR_REF,
  ANWXML_ERROR_BINARY_ENTITY_REF,
  ANWXML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF,
  ANWXML_ERROR_MISPLACED_ANWXML_PI,
  ANWXML_ERROR_UNKNOWN_ENCODING,
  ANWXML_ERROR_INCORRECT_ENCODING,
  ANWXML_ERROR_UNCLOSED_CDATA_SECTION,
  ANWXML_ERROR_EXTERNAL_ENTITY_HANDLING,
  ANWXML_ERROR_NOT_STANDALONE,
  ANWXML_ERROR_UNEXPECTED_STATE,
  ANWXML_ERROR_ENTITY_DECLARED_IN_PE,
  ANWXML_ERROR_FEATURE_REQUIRES_ANWXML_DTD,
  ANWXML_ERROR_CANT_CHANGE_FEATURE_ONCE_PARSING,
  /* Added in 1.95.7. */
  ANWXML_ERROR_UNBOUND_PREFIX,
  /* Added in 1.95.8. */
  ANWXML_ERROR_UNDECLARING_PREFIX,
  ANWXML_ERROR_INCOMPLETE_PE,
  ANWXML_ERROR_ANWXML_DECL,
  ANWXML_ERROR_TEXT_DECL,
  ANWXML_ERROR_PUBLICID,
  ANWXML_ERROR_SUSPENDED,
  ANWXML_ERROR_NOT_SUSPENDED,
  ANWXML_ERROR_ABORTED,
  ANWXML_ERROR_FINISHED,
  ANWXML_ERROR_SUSPEND_PE
};

enum ANWXML_Content_Type {
  ANWXML_CTYPE_EMPTY = 1,
  ANWXML_CTYPE_ANY,
  ANWXML_CTYPE_MIXED,
  ANWXML_CTYPE_NAME,
  ANWXML_CTYPE_CHOICE,
  ANWXML_CTYPE_SEQ
};

enum ANWXML_Content_Quant {
  ANWXML_CQUANT_NONE,
  ANWXML_CQUANT_OPT,
  ANWXML_CQUANT_REP,
  ANWXML_CQUANT_PLUS
};

/* If type == ANWXML_CTYPE_EMPTY or ANWXML_CTYPE_ANY, then quant will be
   ANWXML_CQUANT_NONE, and the other fields will be zero or NULL.
   If type == ANWXML_CTYPE_MIXED, then quant will be NONE or REP and
   numchildren will contain number of elements that may be mixed in
   and children point to an array of ANWXML_Content cells that will be
   all of ANWXML_CTYPE_NAME type with no quantification.

   If type == ANWXML_CTYPE_NAME, then the name points to the name, and
   the numchildren field will be zero and children will be NULL. The
   quant fields indicates any quantifiers placed on the name.

   CHOICE and SEQ will have name NULL, the number of children in
   numchildren and children will point, recursively, to an array
   of ANWXML_Content cells.

   The EMPTY, ANY, and MIXED types will only occur at top level.
*/

typedef struct ANWXML_cp ANWXML_Content;

struct ANWXML_cp {
  enum ANWXML_Content_Type         type;
  enum ANWXML_Content_Quant        quant;
  ANWXML_Char *                    name;
  unsigned int                  numchildren;
  ANWXML_Content *                 children;
};


/* This is called for an element declaration. See above for
   description of the model argument. It's the caller's responsibility
   to free model when finished with it.
*/
typedef void (XMLCALL *ANWXML_ElementDeclHandler) (void *userData,
                                                const ANWXML_Char *name,
                                                ANWXML_Content *model);

XMLPARSEAPI(void)
ANWXML_SetElementDeclHandler(ANWXML_Parser parser,
                          ANWXML_ElementDeclHandler eldecl);

/* The Attlist declaration handler is called for *each* attribute. So
   a single Attlist declaration with multiple attributes declared will
   generate multiple calls to this handler. The "default" parameter
   may be NULL in the case of the "#IMPLIED" or "#REQUIRED"
   keyword. The "isrequired" parameter will be true and the default
   value will be NULL in the case of "#REQUIRED". If "isrequired" is
   true and default is non-NULL, then this is a "#FIXED" default.
*/
typedef void (XMLCALL *ANWXML_AttlistDeclHandler) (
                                    void            *userData,
                                    const ANWXML_Char  *elname,
                                    const ANWXML_Char  *attname,
                                    const ANWXML_Char  *att_type,
                                    const ANWXML_Char  *dflt,
                                    int              isrequired);

XMLPARSEAPI(void)
ANWXML_SetAttlistDeclHandler(ANWXML_Parser parser,
                          ANWXML_AttlistDeclHandler attdecl);

/* The XML declaration handler is called for *both* XML declarations
   and text declarations. The way to distinguish is that the version
   parameter will be NULL for text declarations. The encoding
   parameter may be NULL for XML declarations. The standalone
   parameter will be -1, 0, or 1 indicating respectively that there
   was no standalone parameter in the declaration, that it was given
   as no, or that it was given as yes.
*/
typedef void (XMLCALL *ANWXML_XmlDeclHandler) (void           *userData,
                                            const ANWXML_Char *version,
                                            const ANWXML_Char *encoding,
                                            int             standalone);

XMLPARSEAPI(void)
ANWXML_SetXmlDeclHandler(ANWXML_Parser parser,
                      ANWXML_XmlDeclHandler xmldecl);


typedef struct {
  void *(*malloc_fcn)(size_t size);
  void *(*realloc_fcn)(void *ptr, size_t size);
  void (*free_fcn)(void *ptr);
} ANWXML_Memory_Handling_Suite;

/* Constructs a new parser; encoding is the encoding specified by the
   external protocol or NULL if there is none specified.
*/
XMLPARSEAPI(ANWXML_Parser)
ANWXML_ParserCreate(const ANWXML_Char *encoding);

/* Constructs a new parser and namespace processor.  Element type
   names and attribute names that belong to a namespace will be
   expanded; unprefixed attribute names are never expanded; unprefixed
   element type names are expanded only if there is a default
   namespace. The expanded name is the concatenation of the namespace
   URI, the namespace separator character, and the local part of the
   name.  If the namespace separator is '\0' then the namespace URI
   and the local part will be concatenated without any separator.
   When a namespace is not declared, the name and prefix will be
   passed through without expansion.
*/
XMLPARSEAPI(ANWXML_Parser)
ANWXML_ParserCreateNS(const ANWXML_Char *encoding, ANWXML_Char namespaceSeparator);


/* Constructs a new parser using the memory management suite referred to
   by memsuite. If memsuite is NULL, then use the standard library memory
   suite. If namespaceSeparator is non-NULL it creates a parser with
   namespace processing as described above. The character pointed at
   will serve as the namespace separator.

   All further memory operations used for the created parser will come from
   the given suite.
*/
XMLPARSEAPI(ANWXML_Parser)
ANWXML_ParserCreate_MM(const ANWXML_Char *encoding,
                    const ANWXML_Memory_Handling_Suite *memsuite,
                    const ANWXML_Char *namespaceSeparator);

/* Prepare a parser object to be re-used.  This is particularly
   valuable when memory allocation overhead is disproportionatly high,
   such as when a large number of small documnents need to be parsed.
   All handlers are cleared from the parser, except for the
   unknownEncodingHandler. The parser's external state is re-initialized
   except for the values of ns and ns_triplets.

   Added in Expat 1.95.3.
*/
XMLPARSEAPI(ANWXML_Bool)
ANWXML_ParserReset(ANWXML_Parser parser, const ANWXML_Char *encoding);

/* atts is array of name/value pairs, terminated by 0;
   names and values are 0 terminated.
*/
typedef void (XMLCALL *ANWXML_StartElementHandler) (void *userData,
                                                 const ANWXML_Char *name,
                                                 const ANWXML_Char **atts);

typedef void (XMLCALL *ANWXML_EndElementHandler) (void *userData,
                                               const ANWXML_Char *name);


/* s is not 0 terminated. */
typedef void (XMLCALL *ANWXML_CharacterDataHandler) (void *userData,
                                                  const ANWXML_Char *s,
                                                  int len);

/* target and data are 0 terminated */
typedef void (XMLCALL *ANWXML_ProcessingInstructionHandler) (
                                                void *userData,
                                                const ANWXML_Char *target,
                                                const ANWXML_Char *data);

/* data is 0 terminated */
typedef void (XMLCALL *ANWXML_CommentHandler) (void *userData,
                                            const ANWXML_Char *data);

typedef void (XMLCALL *ANWXML_StartCdataSectionHandler) (void *userData);
typedef void (XMLCALL *ANWXML_EndCdataSectionHandler) (void *userData);

/* This is called for any characters in the XML document for which
   there is no applicable handler.  This includes both characters that
   are part of markup which is of a kind that is not reported
   (comments, markup declarations), or characters that are part of a
   construct which could be reported but for which no handler has been
   supplied. The characters are passed exactly as they were in the XML
   document except that they will be encoded in UTF-8 or UTF-16.
   Line boundaries are not normalized. Note that a byte order mark
   character is not passed to the default handler. There are no
   guarantees about how characters are divided between calls to the
   default handler: for example, a comment might be split between
   multiple calls.
*/
typedef void (XMLCALL *ANWXML_DefaultHandler) (void *userData,
                                            const ANWXML_Char *s,
                                            int len);

/* This is called for the start of the DOCTYPE declaration, before
   any DTD or internal subset is parsed.
*/
typedef void (XMLCALL *ANWXML_StartDoctypeDeclHandler) (
                                            void *userData,
                                            const ANWXML_Char *doctypeName,
                                            const ANWXML_Char *sysid,
                                            const ANWXML_Char *pubid,
                                            int has_internal_subset);

/* This is called for the start of the DOCTYPE declaration when the
   closing > is encountered, but after processing any external
   subset.
*/
typedef void (XMLCALL *ANWXML_EndDoctypeDeclHandler)(void *userData);

/* This is called for entity declarations. The is_parameter_entity
   argument will be non-zero if the entity is a parameter entity, zero
   otherwise.

   For internal entities (<!ENTITY foo "bar">), value will
   be non-NULL and systemId, publicID, and notationName will be NULL.
   The value string is NOT nul-terminated; the length is provided in
   the value_length argument. Since it is legal to have zero-length
   values, do not use this argument to test for internal entities.

   For external entities, value will be NULL and systemId will be
   non-NULL. The publicId argument will be NULL unless a public
   identifier was provided. The notationName argument will have a
   non-NULL value only for unparsed entity declarations.

   Note that is_parameter_entity can't be changed to ANWXML_Bool, since
   that would break binary compatibility.
*/
typedef void (XMLCALL *ANWXML_EntityDeclHandler) (
                              void *userData,
                              const ANWXML_Char *entityName,
                              int is_parameter_entity,
                              const ANWXML_Char *value,
                              int value_length,
                              const ANWXML_Char *base,
                              const ANWXML_Char *systemId,
                              const ANWXML_Char *publicId,
                              const ANWXML_Char *notationName);

XMLPARSEAPI(void)
ANWXML_SetEntityDeclHandler(ANWXML_Parser parser,
                         ANWXML_EntityDeclHandler handler);

/* OBSOLETE -- OBSOLETE -- OBSOLETE
   This handler has been superceded by the EntityDeclHandler above.
   It is provided here for backward compatibility.

   This is called for a declaration of an unparsed (NDATA) entity.
   The base argument is whatever was set by ANWXML_SetBase. The
   entityName, systemId and notationName arguments will never be
   NULL. The other arguments may be.
*/
typedef void (XMLCALL *ANWXML_UnparsedEntityDeclHandler) (
                                    void *userData,
                                    const ANWXML_Char *entityName,
                                    const ANWXML_Char *base,
                                    const ANWXML_Char *systemId,
                                    const ANWXML_Char *publicId,
                                    const ANWXML_Char *notationName);

/* This is called for a declaration of notation.  The base argument is
   whatever was set by ANWXML_SetBase. The notationName will never be
   NULL.  The other arguments can be.
*/
typedef void (XMLCALL *ANWXML_NotationDeclHandler) (
                                    void *userData,
                                    const ANWXML_Char *notationName,
                                    const ANWXML_Char *base,
                                    const ANWXML_Char *systemId,
                                    const ANWXML_Char *publicId);

/* When namespace processing is enabled, these are called once for
   each namespace declaration. The call to the start and end element
   handlers occur between the calls to the start and end namespace
   declaration handlers. For an xmlns attribute, prefix will be
   NULL.  For an xmlns="" attribute, uri will be NULL.
*/
typedef void (XMLCALL *ANWXML_StartNamespaceDeclHandler) (
                                    void *userData,
                                    const ANWXML_Char *prefix,
                                    const ANWXML_Char *uri);

typedef void (XMLCALL *ANWXML_EndNamespaceDeclHandler) (
                                    void *userData,
                                    const ANWXML_Char *prefix);

/* This is called if the document is not standalone, that is, it has an
   external subset or a reference to a parameter entity, but does not
   have standalone="yes". If this handler returns ANWXML_STATUS_ERROR,
   then processing will not continue, and the parser will return a
   ANWXML_ERROR_NOT_STANDALONE error.
   If parameter entity parsing is enabled, then in addition to the
   conditions above this handler will only be called if the referenced
   entity was actually read.
*/
typedef int (XMLCALL *ANWXML_NotStandaloneHandler) (void *userData);

/* This is called for a reference to an external parsed general
   entity.  The referenced entity is not automatically parsed.  The
   application can parse it immediately or later using
   ANWXML_ExternalEntityParserCreate.

   The parser argument is the parser parsing the entity containing the
   reference; it can be passed as the parser argument to
   ANWXML_ExternalEntityParserCreate.  The systemId argument is the
   system identifier as specified in the entity declaration; it will
   not be NULL.

   The base argument is the system identifier that should be used as
   the base for resolving systemId if systemId was relative; this is
   set by ANWXML_SetBase; it may be NULL.

   The publicId argument is the public identifier as specified in the
   entity declaration, or NULL if none was specified; the whitespace
   in the public identifier will have been normalized as required by
   the XML spec.

   The context argument specifies the parsing context in the format
   expected by the context argument to ANWXML_ExternalEntityParserCreate;
   context is valid only until the handler returns, so if the
   referenced entity is to be parsed later, it must be copied.
   context is NULL only when the entity is a parameter entity.

   The handler should return ANWXML_STATUS_ERROR if processing should not
   continue because of a fatal error in the handling of the external
   entity.  In this case the calling parser will return an
   ANWXML_ERROR_EXTERNAL_ENTITY_HANDLING error.

   Note that unlike other handlers the first argument is the parser,
   not userData.
*/
typedef int (XMLCALL *ANWXML_ExternalEntityRefHandler) (
                                    ANWXML_Parser parser,
                                    const ANWXML_Char *context,
                                    const ANWXML_Char *base,
                                    const ANWXML_Char *systemId,
                                    const ANWXML_Char *publicId);

/* This is called in two situations:
   1) An entity reference is encountered for which no declaration
      has been read *and* this is not an error.
   2) An internal entity reference is read, but not expanded, because
      ANWXML_SetDefaultHandler has been called.
   Note: skipped parameter entities in declarations and skipped general
         entities in attribute values cannot be reported, because
         the event would be out of sync with the reporting of the
         declarations or attribute values
*/
typedef void (XMLCALL *ANWXML_SkippedEntityHandler) (
                                    void *userData,
                                    const ANWXML_Char *entityName,
                                    int is_parameter_entity);

/* This structure is filled in by the ANWXML_UnknownEncodingHandler to
   provide information to the parser about encodings that are unknown
   to the parser.

   The map[b] member gives information about byte sequences whose
   first byte is b.

   If map[b] is c where c is >= 0, then b by itself encodes the
   Unicode scalar value c.

   If map[b] is -1, then the byte sequence is malformed.

   If map[b] is -n, where n >= 2, then b is the first byte of an
   n-byte sequence that encodes a single Unicode scalar value.

   The data member will be passed as the first argument to the convert
   function.

   The convert function is used to convert multibyte sequences; s will
   point to a n-byte sequence where map[(unsigned char)*s] == -n.  The
   convert function must return the Unicode scalar value represented
   by this byte sequence or -1 if the byte sequence is malformed.

   The convert function may be NULL if the encoding is a single-byte
   encoding, that is if map[b] >= -1 for all bytes b.

   When the parser is finished with the encoding, then if release is
   not NULL, it will call release passing it the data member; once
   release has been called, the convert function will not be called
   again.

   Expat places certain restrictions on the encodings that are supported
   using this mechanism.

   1. Every ASCII character that can appear in a well-formed XML document,
      other than the characters

      $@\^`{}~

      must be represented by a single byte, and that byte must be the
      same byte that represents that character in ASCII.

   2. No character may require more than 4 bytes to encode.

   3. All characters encoded must have Unicode scalar values <=
      0xFFFF, (i.e., characters that would be encoded by surrogates in
      UTF-16 are  not allowed).  Note that this restriction doesn't
      apply to the built-in support for UTF-8 and UTF-16.

   4. No Unicode character may be encoded by more than one distinct
      sequence of bytes.
*/
typedef struct {
  int map[256];
  void *data;
  int (XMLCALL *convert)(void *data, const char *s);
  void (XMLCALL *release)(void *data);
} ANWXML_Encoding;

/* This is called for an encoding that is unknown to the parser.

   The encodingHandlerData argument is that which was passed as the
   second argument to ANWXML_SetUnknownEncodingHandler.

   The name argument gives the name of the encoding as specified in
   the encoding declaration.

   If the callback can provide information about the encoding, it must
   fill in the ANWXML_Encoding structure, and return ANWXML_STATUS_OK.
   Otherwise it must return ANWXML_STATUS_ERROR.

   If info does not describe a suitable encoding, then the parser will
   return an ANWXML_UNKNOWN_ENCODING error.
*/
typedef int (XMLCALL *ANWXML_UnknownEncodingHandler) (
                                    void *encodingHandlerData,
                                    const ANWXML_Char *name,
                                    ANWXML_Encoding *info);

XMLPARSEAPI(void)
ANWXML_SetElementHandler(ANWXML_Parser parser,
                      ANWXML_StartElementHandler start,
                      ANWXML_EndElementHandler end);

XMLPARSEAPI(void)
ANWXML_SetStartElementHandler(ANWXML_Parser parser,
                           ANWXML_StartElementHandler handler);

XMLPARSEAPI(void)
ANWXML_SetEndElementHandler(ANWXML_Parser parser,
                         ANWXML_EndElementHandler handler);

XMLPARSEAPI(void)
ANWXML_SetCharacterDataHandler(ANWXML_Parser parser,
                            ANWXML_CharacterDataHandler handler);

XMLPARSEAPI(void)
ANWXML_SetProcessingInstructionHandler(ANWXML_Parser parser,
                                    ANWXML_ProcessingInstructionHandler handler);
XMLPARSEAPI(void)
ANWXML_SetCommentHandler(ANWXML_Parser parser,
                      ANWXML_CommentHandler handler);

XMLPARSEAPI(void)
ANWXML_SetCdataSectionHandler(ANWXML_Parser parser,
                           ANWXML_StartCdataSectionHandler start,
                           ANWXML_EndCdataSectionHandler end);

XMLPARSEAPI(void)
ANWXML_SetStartCdataSectionHandler(ANWXML_Parser parser,
                                ANWXML_StartCdataSectionHandler start);

XMLPARSEAPI(void)
ANWXML_SetEndCdataSectionHandler(ANWXML_Parser parser,
                              ANWXML_EndCdataSectionHandler end);

/* This sets the default handler and also inhibits expansion of
   internal entities. These entity references will be passed to the
   default handler, or to the skipped entity handler, if one is set.
*/
XMLPARSEAPI(void)
ANWXML_SetDefaultHandler(ANWXML_Parser parser,
                      ANWXML_DefaultHandler handler);

/* This sets the default handler but does not inhibit expansion of
   internal entities.  The entity reference will not be passed to the
   default handler.
*/
XMLPARSEAPI(void)
ANWXML_SetDefaultHandlerExpand(ANWXML_Parser parser,
                            ANWXML_DefaultHandler handler);

XMLPARSEAPI(void)
ANWXML_SetDoctypeDeclHandler(ANWXML_Parser parser,
                          ANWXML_StartDoctypeDeclHandler start,
                          ANWXML_EndDoctypeDeclHandler end);

XMLPARSEAPI(void)
ANWXML_SetStartDoctypeDeclHandler(ANWXML_Parser parser,
                               ANWXML_StartDoctypeDeclHandler start);

XMLPARSEAPI(void)
ANWXML_SetEndDoctypeDeclHandler(ANWXML_Parser parser,
                             ANWXML_EndDoctypeDeclHandler end);

XMLPARSEAPI(void)
ANWXML_SetUnparsedEntityDeclHandler(ANWXML_Parser parser,
                                 ANWXML_UnparsedEntityDeclHandler handler);

XMLPARSEAPI(void)
ANWXML_SetNotationDeclHandler(ANWXML_Parser parser,
                           ANWXML_NotationDeclHandler handler);

XMLPARSEAPI(void)
ANWXML_SetNamespaceDeclHandler(ANWXML_Parser parser,
                            ANWXML_StartNamespaceDeclHandler start,
                            ANWXML_EndNamespaceDeclHandler end);

XMLPARSEAPI(void)
ANWXML_SetStartNamespaceDeclHandler(ANWXML_Parser parser,
                                 ANWXML_StartNamespaceDeclHandler start);

XMLPARSEAPI(void)
ANWXML_SetEndNamespaceDeclHandler(ANWXML_Parser parser,
                               ANWXML_EndNamespaceDeclHandler end);

XMLPARSEAPI(void)
ANWXML_SetNotStandaloneHandler(ANWXML_Parser parser,
                            ANWXML_NotStandaloneHandler handler);

XMLPARSEAPI(void)
ANWXML_SetExternalEntityRefHandler(ANWXML_Parser parser,
                                ANWXML_ExternalEntityRefHandler handler);

/* If a non-NULL value for arg is specified here, then it will be
   passed as the first argument to the external entity ref handler
   instead of the parser object.
*/
XMLPARSEAPI(void)
ANWXML_SetExternalEntityRefHandlerArg(ANWXML_Parser parser,
                                   void *arg);

XMLPARSEAPI(void)
ANWXML_SetSkippedEntityHandler(ANWXML_Parser parser,
                            ANWXML_SkippedEntityHandler handler);

XMLPARSEAPI(void)
ANWXML_SetUnknownEncodingHandler(ANWXML_Parser parser,
                              ANWXML_UnknownEncodingHandler handler,
                              void *encodingHandlerData);

/* This can be called within a handler for a start element, end
   element, processing instruction or character data.  It causes the
   corresponding markup to be passed to the default handler.
*/
XMLPARSEAPI(void)
ANWXML_DefaultCurrent(ANWXML_Parser parser);

/* If do_nst is non-zero, and namespace processing is in effect, and
   a name has a prefix (i.e. an explicit namespace qualifier) then
   that name is returned as a triplet in a single string separated by
   the separator character specified when the parser was created: URI
   + sep + local_name + sep + prefix.

   If do_nst is zero, then namespace information is returned in the
   default manner (URI + sep + local_name) whether or not the name
   has a prefix.

   Note: Calling ANWXML_SetReturnNSTriplet after ANWXML_Parse or
     ANWXML_ParseBuffer has no effect.
*/

XMLPARSEAPI(void)
ANWXML_SetReturnNSTriplet(ANWXML_Parser parser, int do_nst);

/* This value is passed as the userData argument to callbacks. */
XMLPARSEAPI(void)
ANWXML_SetUserData(ANWXML_Parser parser, void *userData);

/* Returns the last value set by ANWXML_SetUserData or NULL. */
#define ANWXML_GetUserData(parser) (*(void **)(parser))

/* This is equivalent to supplying an encoding argument to
   ANWXML_ParserCreate. On success ANWXML_SetEncoding returns non-zero,
   zero otherwise.
   Note: Calling ANWXML_SetEncoding after ANWXML_Parse or ANWXML_ParseBuffer
     has no effect and returns ANWXML_STATUS_ERROR.
*/
XMLPARSEAPI(enum ANWXML_Status)
ANWXML_SetEncoding(ANWXML_Parser parser, const ANWXML_Char *encoding);

/* If this function is called, then the parser will be passed as the
   first argument to callbacks instead of userData.  The userData will
   still be accessible using ANWXML_GetUserData.
*/
XMLPARSEAPI(void)
ANWXML_UseParserAsHandlerArg(ANWXML_Parser parser);

/* If useDTD == ANWXML_TRUE is passed to this function, then the parser
   will assume that there is an external subset, even if none is
   specified in the document. In such a case the parser will call the
   externalEntityRefHandler with a value of NULL for the systemId
   argument (the publicId and context arguments will be NULL as well).
   Note: For the purpose of checking WFC: Entity Declared, passing
     useDTD == ANWXML_TRUE will make the parser behave as if the document
     had a DTD with an external subset.
   Note: If this function is called, then this must be done before
     the first call to ANWXML_Parse or ANWXML_ParseBuffer, since it will
     have no effect after that.  Returns
     ANWXML_ERROR_CANT_CHANGE_FEATURE_ONCE_PARSING.
   Note: If the document does not have a DOCTYPE declaration at all,
     then startDoctypeDeclHandler and endDoctypeDeclHandler will not
     be called, despite an external subset being parsed.
   Note: If ANWXML_DTD is not defined when Expat is compiled, returns
     ANWXML_ERROR_FEATURE_REQUIRES_ANWXML_DTD.
*/
XMLPARSEAPI(enum ANWXML_Error)
ANWXML_UseForeignDTD(ANWXML_Parser parser, ANWXML_Bool useDTD);


/* Sets the base to be used for resolving relative URIs in system
   identifiers in declarations.  Resolving relative identifiers is
   left to the application: this value will be passed through as the
   base argument to the ANWXML_ExternalEntityRefHandler,
   ANWXML_NotationDeclHandler and ANWXML_UnparsedEntityDeclHandler. The base
   argument will be copied.  Returns ANWXML_STATUS_ERROR if out of memory,
   ANWXML_STATUS_OK otherwise.
*/
XMLPARSEAPI(enum ANWXML_Status)
ANWXML_SetBase(ANWXML_Parser parser, const ANWXML_Char *base);

XMLPARSEAPI(const ANWXML_Char *)
ANWXML_GetBase(ANWXML_Parser parser);

/* Returns the number of the attribute/value pairs passed in last call
   to the ANWXML_StartElementHandler that were specified in the start-tag
   rather than defaulted. Each attribute/value pair counts as 2; thus
   this correspondds to an index into the atts array passed to the
   ANWXML_StartElementHandler.
*/
XMLPARSEAPI(int)
ANWXML_GetSpecifiedAttributeCount(ANWXML_Parser parser);

/* Returns the index of the ID attribute passed in the last call to
   ANWXML_StartElementHandler, or -1 if there is no ID attribute.  Each
   attribute/value pair counts as 2; thus this correspondds to an
   index into the atts array passed to the ANWXML_StartElementHandler.
*/
XMLPARSEAPI(int)
ANWXML_GetIdAttributeIndex(ANWXML_Parser parser);

/* Parses some input. Returns ANWXML_STATUS_ERROR if a fatal error is
   detected.  The last call to ANWXML_Parse must have isFinal true; len
   may be zero for this call (or any other).

   Though the return values for these functions has always been
   described as a Boolean value, the implementation, at least for the
   1.95.x series, has always returned exactly one of the ANWXML_Status
   values.
*/
XMLPARSEAPI(enum ANWXML_Status)
ANWXML_Parse(ANWXML_Parser parser, const char *s, int len, int isFinal);

XMLPARSEAPI(void *)
ANWXML_GetBuffer(ANWXML_Parser parser, int len);

XMLPARSEAPI(enum ANWXML_Status)
ANWXML_ParseBuffer(ANWXML_Parser parser, int len, int isFinal);

/* Stops parsing, causing ANWXML_Parse() or ANWXML_ParseBuffer() to return.
   Must be called from within a call-back handler, except when aborting
   (resumable = 0) an already suspended parser. Some call-backs may
   still follow because they would otherwise get lost. Examples:
   - endElementHandler() for empty elements when stopped in
     startElementHandler(), 
   - endNameSpaceDeclHandler() when stopped in endElementHandler(), 
   and possibly others.

   Can be called from most handlers, including DTD related call-backs,
   except when parsing an external parameter entity and resumable != 0.
   Returns ANWXML_STATUS_OK when successful, ANWXML_STATUS_ERROR otherwise.
   Possible error codes: 
   - ANWXML_ERROR_SUSPENDED: when suspending an already suspended parser.
   - ANWXML_ERROR_FINISHED: when the parser has already finished.
   - ANWXML_ERROR_SUSPEND_PE: when suspending while parsing an external PE.

   When resumable != 0 (true) then parsing is suspended, that is, 
   ANWXML_Parse() and ANWXML_ParseBuffer() return ANWXML_STATUS_SUSPENDED. 
   Otherwise, parsing is aborted, that is, ANWXML_Parse() and ANWXML_ParseBuffer()
   return ANWXML_STATUS_ERROR with error code ANWXML_ERROR_ABORTED.

   *Note*:
   This will be applied to the current parser instance only, that is, if
   there is a parent parser then it will continue parsing when the
   externalEntityRefHandler() returns. It is up to the implementation of
   the externalEntityRefHandler() to call ANWXML_StopParser() on the parent
   parser (recursively), if one wants to stop parsing altogether.

   When suspended, parsing can be resumed by calling ANWXML_ResumeParser(). 
*/
XMLPARSEAPI(enum ANWXML_Status)
ANWXML_StopParser(ANWXML_Parser parser, ANWXML_Bool resumable);

/* Resumes parsing after it has been suspended with ANWXML_StopParser().
   Must not be called from within a handler call-back. Returns same
   status codes as ANWXML_Parse() or ANWXML_ParseBuffer().
   Additional error code ANWXML_ERROR_NOT_SUSPENDED possible.   

   *Note*:
   This must be called on the most deeply nested child parser instance
   first, and on its parent parser only after the child parser has finished,
   to be applied recursively until the document entity's parser is restarted.
   That is, the parent parser will not resume by itself and it is up to the
   application to call ANWXML_ResumeParser() on it at the appropriate moment.
*/
XMLPARSEAPI(enum ANWXML_Status)
ANWXML_ResumeParser(ANWXML_Parser parser);

enum ANWXML_Parsing {
  ANWXML_INITIALIZED,
  ANWXML_PARSING,
  ANWXML_FINISHED,
  ANWXML_SUSPENDED
};

typedef struct {
  enum ANWXML_Parsing parsing;
  ANWXML_Bool finalBuffer;
} ANWXML_ParsingStatus;

/* Returns status of parser with respect to being initialized, parsing,
   finished, or suspended and processing the final buffer.
   XXX ANWXML_Parse() and ANWXML_ParseBuffer() should return ANWXML_ParsingStatus,
   XXX with ANWXML_FINISHED_OK or ANWXML_FINISHED_ERROR replacing ANWXML_FINISHED
*/
XMLPARSEAPI(void)
ANWXML_GetParsingStatus(ANWXML_Parser parser, ANWXML_ParsingStatus *status);

/* Creates an ANWXML_Parser object that can parse an external general
   entity; context is a '\0'-terminated string specifying the parse
   context; encoding is a '\0'-terminated string giving the name of
   the externally specified encoding, or NULL if there is no
   externally specified encoding.  The context string consists of a
   sequence of tokens separated by formfeeds (\f); a token consisting
   of a name specifies that the general entity of the name is open; a
   token of the form prefix=uri specifies the namespace for a
   particular prefix; a token of the form =uri specifies the default
   namespace.  This can be called at any point after the first call to
   an ExternalEntityRefHandler so longer as the parser has not yet
   been freed.  The new parser is completely independent and may
   safely be used in a separate thread.  The handlers and userData are
   initialized from the parser argument.  Returns NULL if out of memory.
   Otherwise returns a new ANWXML_Parser object.
*/
XMLPARSEAPI(ANWXML_Parser)
ANWXML_ExternalEntityParserCreate(ANWXML_Parser parser,
                               const ANWXML_Char *context,
                               const ANWXML_Char *encoding);

enum ANWXML_ParamEntityParsing {
  ANWXML_PARAM_ENTITY_PARSING_NEVER,
  ANWXML_PARAM_ENTITY_PARSING_UNLESS_STANDALONE,
  ANWXML_PARAM_ENTITY_PARSING_ALWAYS
};

/* Controls parsing of parameter entities (including the external DTD
   subset). If parsing of parameter entities is enabled, then
   references to external parameter entities (including the external
   DTD subset) will be passed to the handler set with
   ANWXML_SetExternalEntityRefHandler.  The context passed will be 0.

   Unlike external general entities, external parameter entities can
   only be parsed synchronously.  If the external parameter entity is
   to be parsed, it must be parsed during the call to the external
   entity ref handler: the complete sequence of
   ANWXML_ExternalEntityParserCreate, ANWXML_Parse/ANWXML_ParseBuffer and
   ANWXML_ParserFree calls must be made during this call.  After
   ANWXML_ExternalEntityParserCreate has been called to create the parser
   for the external parameter entity (context must be 0 for this
   call), it is illegal to make any calls on the old parser until
   ANWXML_ParserFree has been called on the newly created parser.
   If the library has been compiled without support for parameter
   entity parsing (ie without ANWXML_DTD being defined), then
   ANWXML_SetParamEntityParsing will return 0 if parsing of parameter
   entities is requested; otherwise it will return non-zero.
   Note: If ANWXML_SetParamEntityParsing is called after ANWXML_Parse or
      ANWXML_ParseBuffer, then it has no effect and will always return 0.
*/
XMLPARSEAPI(int)
ANWXML_SetParamEntityParsing(ANWXML_Parser parser,
                          enum ANWXML_ParamEntityParsing parsing);

/* If ANWXML_Parse or ANWXML_ParseBuffer have returned ANWXML_STATUS_ERROR, then
   ANWXML_GetErrorCode returns information about the error.
*/
XMLPARSEAPI(enum ANWXML_Error)
ANWXML_GetErrorCode(ANWXML_Parser parser);

/* These functions return information about the current parse
   location.  They may be called from any callback called to report
   some parse event; in this case the location is the location of the
   first of the sequence of characters that generated the event.  When
   called from callbacks generated by declarations in the document
   prologue, the location identified isn't as neatly defined, but will
   be within the relevant markup.  When called outside of the callback
   functions, the position indicated will be just past the last parse
   event (regardless of whether there was an associated callback).
   
   They may also be called after returning from a call to ANWXML_Parse
   or ANWXML_ParseBuffer.  If the return value is ANWXML_STATUS_ERROR then
   the location is the location of the character at which the error
   was detected; otherwise the location is the location of the last
   parse event, as described above.
*/
XMLPARSEAPI(int) ANWXML_GetCurrentLineNumber(ANWXML_Parser parser);
XMLPARSEAPI(int) ANWXML_GetCurrentColumnNumber(ANWXML_Parser parser);
XMLPARSEAPI(long) ANWXML_GetCurrentByteIndex(ANWXML_Parser parser);

/* Return the number of bytes in the current event.
   Returns 0 if the event is in an internal entity.
*/
XMLPARSEAPI(int)
ANWXML_GetCurrentByteCount(ANWXML_Parser parser);

/* If ANWXML_CONTEXT_BYTES is defined, returns the input buffer, sets
   the integer pointed to by offset to the offset within this buffer
   of the current parse position, and sets the integer pointed to by size
   to the size of this buffer (the number of input bytes). Otherwise
   returns a NULL pointer. Also returns a NULL pointer if a parse isn't
   active.

   NOTE: The character pointer returned should not be used outside
   the handler that makes the call.
*/
XMLPARSEAPI(const char *)
ANWXML_GetInputContext(ANWXML_Parser parser,
                    int *offset,
                    int *size);

/* For backwards compatibility with previous versions. */
#define ANWXML_GetErrorLineNumber   ANWXML_GetCurrentLineNumber
#define ANWXML_GetErrorColumnNumber ANWXML_GetCurrentColumnNumber
#define ANWXML_GetErrorByteIndex    ANWXML_GetCurrentByteIndex

/* Frees the content model passed to the element declaration handler */
XMLPARSEAPI(void)
ANWXML_FreeContentModel(ANWXML_Parser parser, ANWXML_Content *model);

/* Exposing the memory handling functions used in Expat */
XMLPARSEAPI(void *)
ANWXML_MemMalloc(ANWXML_Parser parser, size_t size);

XMLPARSEAPI(void *)
ANWXML_MemRealloc(ANWXML_Parser parser, void *ptr, size_t size);

XMLPARSEAPI(void)
ANWXML_MemFree(ANWXML_Parser parser, void *ptr);

/* Frees memory used by the parser. */
XMLPARSEAPI(void)
ANWXML_ParserFree(ANWXML_Parser parser);

/* Returns a string describing the error. */
XMLPARSEAPI(const ANWXML_LChar *)
ANWXML_ErrorString(enum ANWXML_Error code);

/* Return a string containing the version number of this expat */
XMLPARSEAPI(const ANWXML_LChar *)
ANWXML_ExpatVersion(void);

typedef struct {
  int major;
  int minor;
  int micro;
} ANWXML_Expat_Version;

/* Return an ANWXML_Expat_Version structure containing numeric version
   number information for this version of expat.
*/
XMLPARSEAPI(ANWXML_Expat_Version)
ANWXML_ExpatVersionInfo(void);

/* Added in Expat 1.95.5. */
enum ANWXML_FeatureEnum {
  ANWXML_FEATURE_END = 0,
  ANWXML_FEATURE_UNICODE,
  ANWXML_FEATURE_UNICODE_WCHAR_T,
  ANWXML_FEATURE_DTD,
  ANWXML_FEATURE_CONTEXT_BYTES,
  ANWXML_FEATURE_MIN_SIZE,
  ANWXML_FEATURE_SIZEOF_ANWXML_CHAR,
  ANWXML_FEATURE_SIZEOF_ANWXML_LCHAR
  /* Additional features must be added to the end of this enum. */
};

typedef struct {
  enum ANWXML_FeatureEnum  feature;
  const ANWXML_LChar       *name;
  long int              value;
} ANWXML_Feature;

XMLPARSEAPI(const ANWXML_Feature *)
ANWXML_GetFeatureList(void);


/* Expat follows the GNU/Linux convention of odd number minor version for
   beta/development releases and even number minor version for stable
   releases. Micro is bumped with each release, and set to 0 with each
   change to major or minor version.
*/
#define ANWXML_MAJOR_VERSION 1
#define ANWXML_MINOR_VERSION 95
#define ANWXML_MICRO_VERSION 8

#ifdef __cplusplus
}
#endif

#endif /* not XmlParse_INCLUDED */
