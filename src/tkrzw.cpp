// Tkrzw
#ifdef TKRZW
#include "common.h"
#include <string_view>
#include <tkrzw_dbm_poly.h>

tkrzw::PolyDBM db;

bool DbOpen(void) {
  return db.Open("kvtest.tkh", true, tkrzw::File::OPEN_TRUNCATE) == tkrzw::Status::SUCCESS;
}

bool RecordAdd(const uint160_t &k, const uint32_t v) {
  return db.Set(string_view((const char *) &k, sizeof(k)), string_view((const char *)&v, sizeof(v))) == tkrzw::Status::SUCCESS;
}

bool RecordGet(const uint160_t &k) {
  return db.Get(string_view((const char *) &k, sizeof(k)), nullptr)  == tkrzw::Status::SUCCESS;
}

int RecordGetOrAdd(const uint160_t &k, const uint32_t v) {
    /*
     * Returns:
     * -1 - found
     * +1 - added
     *  0 - not found nor added
     */
    if (RecordGet(k))
        return -1;
    return int(RecordAdd(k, v));
}

int main(int argc, char *argv[]) {
  return mainloop(argc, argv, DbOpen, RecordAdd, RecordGet, RecordGetOrAdd);
}
#endif
