// DO NOT EDIT THIS FILE.  IT IS AUTOMATICALLY GENERATED.
// longbow-generate-about 1.0.20160121.477d634e 2016-01-22T04:52:21Z

#ifndef athena_About_h
#define athena_About_h
/**
 * Embedded string containing information for the what(1) command.
 *
 */
extern const char *athena_What;

/**
 * Return the name as a C string.
 *
 * @return The name as a C string.
 */
const char *athenaAbout_Name(void);

/**
 * Return the version as a C string.
 *
 * @return The version as a C string.
 */
const char *athenaAbout_Version(void);

/**
 * Return the About text as a C string.
 *
 * @return The About text as a C string.
 */
const char *athenaAbout_About(void);

/**
 * Return the minimum copyright notice as a C string.
 *
 * @return The minimum copyright notice as a C string.
 */
const char *athenaAbout_MiniNotice(void);

/**
 * Return the short copyright notice as a C string.
 *
 * @return The short copyright notice as a C string.
 */
const char *athenaAbout_ShortNotice(void);

/**
 * Return the long copyright notice as a C string.
 *
 * @return The long copyright notice as a C string.
 */
const char *athenaAbout_LongNotice(void);

#endif // athena_About_h
