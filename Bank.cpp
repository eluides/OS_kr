#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>

class BankAccount {
private:
    double balance;
    std::mutex mtx;
    std::condition_variable cv;

public:
    BankAccount(double initialBalance) : balance(initialBalance) {}

    void modifyBalance(double amount) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this, amount]() { return balance + amount >= 0; });

        std::this_thread::sleep_for(std::chrono::milliseconds(50 + rand() % 51)); 

        balance += amount;
        std::cout << "Было=" << balance - amount << ", операция " << (amount > 0 ? "+" : "-") << std::abs(amount) << ", стало=" << balance << std::endl;
        lock.unlock();
        cv.notify_all();
    }

    double getBalance() const {
        return balance;
    }
};

void performOperations(BankAccount& account, const std::vector<double>& operations) {
    for (double amount : operations) {
        bool success = false;
        int tryCount = 0;
        while (!success && tryCount < 3) {
            try {
                account.modifyBalance(amount);
                success = true;
            }
            catch (const std::exception& e) {
                std::cerr << "Ошибка: " << e.what() << std::endl;
                ++tryCount;
            }
        }
        if (!success) {
            std::cerr << "Операция не удалась после нескольких попыток. Продолжаем со следующей операцией." << std::endl;
        }
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    srand(time(nullptr));

    BankAccount account(100.0);

    std::vector<double> scenario1 = { -120, 10, 15 };
    std::vector<double> scenario2 = { -40, -30, 90 };

    std::thread thread1(performOperations, std::ref(account), scenario1);
    std::thread thread2(performOperations, std::ref(account), scenario2);

    thread1.join();
    thread2.join();

    std::cout << "Итоговый баланс: " << account.getBalance() << std::endl;

    return 0;
}
