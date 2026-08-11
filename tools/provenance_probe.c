/* Small/fast executable used only by tools/provenance_regression.sh. */
#include "../radiobase.c"

int main(void) {
    if (getenv("RADIO_PROBE_INIT") != NULL) init();
    return 0;
}
