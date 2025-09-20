#include "base.hpp"

namespace server {

Base::Base() : db_("data.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {}

void Base::GetDB(std::function<void(SQLite::Database&)> const& callback) {
    std::lock_guard LG(mutex_);
    callback(db_);
}

}  // namespace server
