
#include "client_manager.h"

int main() {
    try {
        ClientManager cm("host=localhost port=5432 dbname=NetologyNew user=postgres password=Qscxzsedwa1.");
        cm.create_tables();

       
        int client1_id = cm.add_client("Иван", "Иванов", "ivan@example.com");
        cm.add_phone(client1_id, "+79001234567");
        cm.add_phone(client1_id, "+79007654321");

        int client2_id = cm.add_client("Мария", "Петрова", "maria@example.com");
        cm.add_phone(client2_id, "+79101112233");

        
        auto found = cm.find_client(std::nullopt, std::nullopt, "ivan@example.com");
        for (const auto& c : found) {
            std::cout << "Найден: " << c.first_name << " " << c.last_name << " (" << c.email << ")\n";
            for (const auto& p : c.phones) {
                std::cout << "  Телефон: " << p.number << " (ID: " << p.id << ")\n";
            }
        }

        
        auto by_phone = cm.find_client(std::nullopt, std::nullopt, std::nullopt, "+79101112233");
        if (!by_phone.empty()) {
            std::cout << "Клиент по телефону: " << by_phone[0].first_name << "\n";
        }

        cm.update_client(client1_id, "Иван", "Сидоров", "ivan.sidorov@example.com");

       
        auto updated = cm.find_client(std::nullopt, std::nullopt, "ivan.sidorov@example.com");
        if (!updated.empty() && !updated[0].phones.empty()) {
            cm.delete_phone(updated[0].phones[0].id); 
            std::cout << "Первый телефон клиента удалён.\n";
        }

        
        cm.delete_client(client2_id);
        std::cout << "Клиент Мария удалён.\n";

    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Все операции выполнены успешно.\n";
    return 0;
}