#pragma once
#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include "pqxx/pqxx"
#include <vector>
#include <string>
#include <iostream>
#include <optional>
#include<unordered_map>


struct Phone {
    int id = 0;
    std::string number;
};

struct Client {
    int id = 0;
    std::string first_name;
    std::string last_name;
    std::string email;
    std::vector<Phone> phones;
};


class ClientManager {
public:
    explicit ClientManager(const std::string& conn_str);
    ~ClientManager() noexcept(false) = default;

    void create_tables();
    int add_client(const std::string& first_name, const std::string& last_name, const std::string& email);
    void add_phone(int client_id, const std::string& phone);
    void update_client(int client_id, const std::string& first_name, const std::string& last_name, const std::string& email);
    void delete_phone(int phone_id);
    void delete_client(int client_id);

    std::vector<Client> find_client(
        const std::optional<std::string>& first_name = std::nullopt,
        const std::optional<std::string>& last_name = std::nullopt,
        const std::optional<std::string>& email = std::nullopt,
        const std::optional<std::string>& phone = std::nullopt
    );

private:
    const pqxx::connection conn;
};

#endif // CLIENT_MANAGER_H