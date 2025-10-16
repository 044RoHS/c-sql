#include "client_manager.h"
#include <pqxx/zview>
#include <unordered_map>

ClientManager::ClientManager(const std::string& conn_str) : conn(conn_str) {
    if (!conn.is_open()) {
        throw std::runtime_error("Не удалось подключиться к базе данных");
    }
}

void ClientManager::create_tables() {
    pqxx::work txn(conn);
    txn.exec(pqxx::zview{ R"(
        CREATE TABLE IF NOT EXISTS clients (
            id SERIAL PRIMARY KEY,
            first_name TEXT NOT NULL,
            last_name TEXT NOT NULL,
            email TEXT UNIQUE
        );
    )" });
    txn.exec(pqxx::zview{ R"(
        CREATE TABLE IF NOT EXISTS phones (
            id SERIAL PRIMARY KEY,
            client_id INTEGER REFERENCES clients(id) ON DELETE CASCADE,
            phone TEXT NOT NULL
        );
    )" });
    txn.commit();
    std::cout << "Таблицы созданы.\n";
}

int ClientManager::add_client(const std::string& first_name, const std::string& last_name, const std::string& email) {
    pqxx::work txn(conn);
    pqxx::params params;
    params.append(first_name);
    params.append(last_name);
    params.append(email);
    auto res = txn.exec(
        pqxx::zview{ "INSERT INTO clients (first_name, last_name, email) VALUES ($1, $2, $3) RETURNING id;" },
        params
    );
    int id = res[0][0].as<int>();
    txn.commit();
    std::cout << "Клиент добавлен с ID: " << id << "\n";
    return id;
}

void ClientManager::add_phone(int client_id, const std::string& phone) {
    pqxx::work txn(conn);
    pqxx::params params;
    params.append(client_id);
    params.append(phone);
    txn.exec(
        pqxx::zview{ "INSERT INTO phones (client_id, phone) VALUES ($1, $2);" },
        params
    );
    txn.commit();
    std::cout << "Телефон добавлен для клиента ID: " << client_id << "\n";
}

void ClientManager::update_client(int client_id, const std::string& first_name, const std::string& last_name, const std::string& email) {
    pqxx::work txn(conn);
    pqxx::params params;
    params.append(first_name);
    params.append(last_name);
    params.append(email);
    params.append(client_id);
    txn.exec(
        pqxx::zview{ "UPDATE clients SET first_name = $1, last_name = $2, email = $3 WHERE id = $4;" },
        params
    );
    txn.commit();
    std::cout << "Данные клиента ID " << client_id << " обновлены.\n";
}

void ClientManager::delete_phone(int phone_id) {
    pqxx::work txn(conn);
    pqxx::params params;
    params.append(phone_id);
    txn.exec(
        pqxx::zview{ "DELETE FROM phones WHERE id = $1;" },
        params
    );
    txn.commit();
    std::cout << "Телефон ID " << phone_id << " удалён.\n";
}

void ClientManager::delete_client(int client_id) {
    pqxx::work txn(conn);
    pqxx::params params;
    params.append(client_id);
    txn.exec(
        pqxx::zview{ "DELETE FROM clients WHERE id = $1;" },
        params
    );
    txn.commit();
    std::cout << "Клиент ID " << client_id << " удалён.\n";
}

std::vector<Client> ClientManager::find_client(
    const std::optional<std::string>& first_name,
    const std::optional<std::string>& last_name,
    const std::optional<std::string>& email,
    const std::optional<std::string>& phone
) {
    std::vector<std::string> conditions;
    pqxx::params params;

    if (first_name) {
        conditions.push_back("c.first_name ILIKE $");
        params.append(*first_name);
    }
    if (last_name) {
        conditions.push_back("c.last_name ILIKE $");
        params.append(*last_name);
    }
    if (email) {
        conditions.push_back("c.email ILIKE $");
        params.append(*email);
    }
    if (phone) {
        conditions.push_back("p.phone ILIKE $");
        params.append(*phone);
    }

    std::string where_clause = "WHERE 1=1";
    for (const auto& cond : conditions) {
        where_clause += " AND " + cond;
    }

    std::string base_query = R"(
        SELECT c.id, c.first_name, c.last_name, c.email, p.id AS phone_id, p.phone
        FROM clients c
        LEFT JOIN phones p ON c.id = p.client_id
    )";
    std::string full_query = base_query + where_clause + " ORDER BY c.id;";

    pqxx::work txn(conn);
    pqxx::result res;

    if (params.size() == 0) {
        res = txn.exec(pqxx::zview{ full_query });
    }
    else {
        res = txn.exec(pqxx::zview{ full_query }, params);
    }

    std::vector<Client> clients;
    std::unordered_map<int, size_t> client_index_map;

    for (const auto& row : res) {
        int client_id = row["id"].as<int>();
        auto it = client_index_map.find(client_id);
        if (it == client_index_map.end()) {
            clients.emplace_back();
            auto& c = clients.back();
            c.id = client_id;
            c.first_name = row["first_name"].as<std::string>();
            c.last_name = row["last_name"].as<std::string>();
            c.email = row["email"].as<std::string>();
            client_index_map[client_id] = clients.size() - 1;
        }

        if (!row["phone"].is_null()) {
            Phone p;
            p.id = row["phone_id"].as<int>();
            p.number = row["phone"].as<std::string>();
            clients[client_index_map[client_id]].phones.push_back(p);
        }
    }

    return clients;
}