/**
 * \file
 * Debug helpers.
 *
 * Define DEBUG_ON before this file is included to enable debug printing. Do not define it to disable it.
 * Call DEBUG() to print.
 *
 * Implementation note:
 *  Purpusfully not in an include guard as it should only be included by c files.
 *  This will cause a compile time error if it is included more than once.
 *
 * \author
 *  Arthur Fabre    <af1g12@ecs.soton.ac.uk>
 */

#ifdef DEBUG_ON
#define DEBUG_VAL 1
#else
#define DEBUG_VAL 0
#endif

/**
 * DEBUG Printing.
 *
 * Implementation note:
 *  A c and not a macro if statement is used to ensure that the print statement is passed to the compiler.
 *  This ensures that the DEBUG statement is always valid, even when debugging is disabled.
 *  If debugging is disabled this will be optimized out by the compiler.
 *  The do while loop makes the macro exapand to a proper statement (and not a conditional).
 */
#define DEBUG(fmt, ...) \
    do { if (DEBUG_VAL) printf("%s:%d:%s():\t" fmt, __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__); } while(0)
