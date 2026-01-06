/*
 * SD Card Tester - Nyx Stubs
 * Provides stub implementations for Hekate/Nyx specific symbols
 * that are referenced by the BDK but not needed by SD Card Tester.
 */

#include <mem/minerva.h>
#include <utils/util.h>

/* Static storage for nyx_str - provides minimal required fields */
static nyx_storage_t _nyx_storage_stub = {0};

/* The volatile pointer that BDK code references */
volatile nyx_storage_t *nyx_str = &_nyx_storage_stub;

/*
 * launch_payload - Not supported in SD Card Tester
 * Returns -1 to indicate failure
 */
int launch_payload(char *path) {
  (void)path;
  return -1;
}
