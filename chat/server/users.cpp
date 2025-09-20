
#include "users.hpp"

#include "client/user.hpp"
#include "spdlog/spdlog.h"

namespace server {

std::string UserDB_Record::ToString() const {
    return fmt::format(
        "[ name: \"{}\", id: {}, password: \"{}\", online: {}, creation date: "
        "{} ]",
        name_, id_, password_, online_ ? "true" : "false", create_date_);
}

UsersHandler::UsersHandler() {
    GetDB([](SQLite::Database& db) {
        try {
            db.exec(R"-(
            CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            online INTEGER NOT NULL,
            name VARCHAR()-" +
                    std::to_string(shared::user_name_max_length) +
                    R"-() NOT NULL UNIQUE,
            password VARCHAR()-" +
                    std::to_string(shared::user_password_max_length) +
                    R"-() NOT NULL,
            create_date DATETIME DEFAULT CURRENT_TIMESTAMP
            ))-");

            db.exec("UPDATE users SET online = 0");

        } catch (std::exception& err) {
            SPDLOG_ERROR("Failed to initialize users table. Error: {}",
                         err.what());
            throw;
        }
    });
}

UserDB_Record UsersHandler::GetUserByID(shared::id_t id) {
    UserDB_Record record{0};
    GetDB([&](SQLite::Database& db) {
        try {
            SQLite::Statement getting_user(
                db,
                "SELECT online, name, password, "
                "create_date FROM users WHERE id = ?");
            getting_user.bind(1, id);

            if (getting_user.executeStep()) {
                bool online = getting_user.getColumn(0).getInt();
                std::string name = getting_user.getColumn(1);
                std::string password = getting_user.getColumn(2);
                std::string date = getting_user.getColumn(3);
                record = {id, online, std::move(name), std::move(password),
                          std::move(date)};
            }
        } catch (std::exception& err) {
            SPDLOG_ERROR("Failed to get user by id. Error: {}", err.what());
            throw;
        }
    });
    return record;
}
shared::id_t UsersHandler::GetUserIDByName(std::string_view name) {
    shared::id_t ret_id = 0;
    GetDB([&](SQLite::Database& db) {
        try {
            SQLite::Statement getting_user(
                db, "SELECT id FROM users WHERE name = ?");
            getting_user.bind(1, name.data());

            if (getting_user.executeStep()) {
                shared::id_t id = getting_user.getColumn(0);
                ret_id = id;
            }
        } catch (std::exception& err) {
            SPDLOG_ERROR("Failed to get user id by name. Error: {}",
                         err.what());
            throw;
        }
    });
    return ret_id;
}
UsersHandler::UserRegisteringResult UsersHandler::RegisterUser(
    std::string_view name, std::string_view password) {
    if (!shared::CheckUserNameSyntax(name))
        return {0, UserRegisteringResult::ResultType::IncorrectNameFormat};
    if (!shared::CheckUserPasswordSyntax(password))
        return {0, UserRegisteringResult::ResultType::IncorrectPasswordFormat};
    if (GetUserIDByName(name))
        return {0, UserRegisteringResult::ResultType::NameAlreadyUsed};

    shared::id_t id;
    GetDB([&](SQLite::Database& db) {
        try {
            SQLite::Statement insert_user(
                db,
                "INSERT INTO users (online, name, password) VALUES (?, ?,?)");
            insert_user.bind(1, 0);
            insert_user.bind(2, name.data());
            insert_user.bind(3, password.data());
            insert_user.exec();
            id = db.getLastInsertRowid();
        } catch (std::exception& err) {
            SPDLOG_ERROR("Failed to register user. Error: {}", err.what());
            throw;
        }
    });

    SPDLOG_INFO("Added new user \"{}\" to database.", name);

    return {id, UserRegisteringResult::ResultType::NoErrors};
}

auto UsersHandler::MarkUserAsOnline(shared::id_t id) -> MarkUserResult {
    MarkUserResult res;
    GetDB([&](SQLite::Database& db) {
        try {
            SQLite::Statement mark_user(
                db, "UPDATE users SET online = 1 WHERE id = ?");
            mark_user.bind(1, id);
            int rows_affected = mark_user.exec();
            if (rows_affected == 0)
                res = MarkUserResult::IDDoesNotExist;
            else if (rows_affected == 1)
                res = MarkUserResult::NoErrors;
            else {
                SPDLOG_ERROR(
                    "Failed to mark user as online. Unexpected result from "
                    "database, {} rows affected.",
                    rows_affected);
                throw std::exception();
            }
        } catch (std::exception& err) {
            SPDLOG_ERROR("Failed to mark user as online. Error: {}",
                         err.what());
            throw;
        }
    });
    return res;
}
auto UsersHandler::MarkUserAsOffline(shared::id_t id) -> MarkUserResult {
    MarkUserResult res;
    GetDB([&](SQLite::Database& db) {
        try {
            SQLite::Statement mark_user(
                db, "UPDATE users SET online = 0 WHERE id = ?");
            mark_user.bind(1, id);
            int rows_affected = mark_user.exec();
            if (rows_affected == 0)
                res = MarkUserResult::IDDoesNotExist;
            else if (rows_affected == 1)
                res = MarkUserResult::NoErrors;
            else {
                SPDLOG_ERROR(
                    "Failed to mark user as offline. Unexpected result from "
                    "database, {} rows affected.",
                    rows_affected);
                throw std::exception();
            }
        } catch (std::exception& err) {
            SPDLOG_ERROR("Failed to mark user as offline. Error: {}",
                         err.what());
            throw;
        }
    });
    return res;
}

std::unique_ptr<client::Base> UsersHandler::_ClientFactory(
    events::EventsProcessor::ClientRawDescriptor desc) {
    return std::make_unique<client::UserHandler>(this, desc);
}
}  // namespace server
