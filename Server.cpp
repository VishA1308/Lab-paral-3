#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <functional>
#include <future>
#include <random>
#include <cmath>
#include <iomanip>
std::mutex out_mutex;
template<typename T>
class Server
{
public:
    Server() : stop_signal(false), th_count(0) {}

    void start()
    {
        if (th_count == 0)
        {
            server_thread = std::thread(&Server::process_tasks, this);
            th_count++;
        }
    }

    void stop()
    {
        if (th_count != 0)
        {
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                stop_signal = true;
                condition.notify_all();
            }
            server_thread.join();
            th_count = 0;
        }
    }

    size_t add_task(std::function<T()> task)
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        size_t task_id = next_id++;
        tasks_queue.push(task_id);
        futures[task_id] = std::async(std::launch::async, task);
        condition.notify_one();
        return task_id;
    }

    T request_result(size_t id_res)
    {
        std::unique_lock<std::mutex> lock(result_mutex);
        condition.wait(lock, [this, id_res] { return results.find(id_res) != results.end(); });
        T res = results[id_res];
        results.erase(id_res);
        return res;

    }

private:
    void process_tasks()
    {
        while (true) {
            size_t task_id;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                condition.wait(lock, [this] { return !tasks_queue.empty() || stop_signal; });
                if (stop_signal && tasks_queue.empty())
                {
                    break;
                }
                task_id = tasks_queue.front();
                tasks_queue.pop();
            }

            T result = futures[task_id].get();
            {
                std::lock_guard<std::mutex> lock(result_mutex);
                results[task_id] = result;
                condition.notify_all(); // Уведомляем о наличии нового результата
            }
        }
    }

    int th_count; // Изменено на int
    std::thread server_thread;
    std::mutex queue_mutex;
    std::mutex result_mutex;
    std::condition_variable condition;
    std::queue<size_t> tasks_queue;
    std::unordered_map<size_t, std::future<T>> futures;
    std::unordered_map<size_t, T> results;
    size_t next_id = 0;
    bool stop_signal;
};

double random_double(double min, double max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <mutex>
#include <cmath>
#include <chrono>
#include <functional>

// Предполагается, что класс Server уже определен
// Пример: template<typename T> class Server { ... };

// Глобальная переменная для синхронизации вывода
std::mutex out_mutex;

// Функция для генерации случайного числа (например)
double random_double(double min, double max) {
    return min + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / (max - min)));
}

// Функция клиента для вычисления синуса
void client_sin(Server<double>& server, int n) {
    std::ofstream outfile("sin_results.txt");
    for (int i = 0; i < n; ++i) {
        double arg = random_double(0, 2 * 3.14);
        size_t id = server.add_task([arg] { return std::sin(arg); });
        double result = server.request_result(id);

        {
            std::lock_guard<std::mutex> lock(out_mutex);
            std::cout << std::fixed << std::setprecision(5);
            std::cout << " sin(" << arg << ") = " << result << "\n";
            outfile << arg << " " << result << "\n"; // Сохранение в файл
        }
    }
    outfile.close(); // Закрытие файла после завершения
}

// Функция клиента для вычисления квадратного корня
void client_sqrt(Server<double>& server, int n) {
    std::ofstream outfile("sqrt_results.txt");
    for (int i = 0; i < n; ++i) {
        double arg = random_double(0, 100);
        size_t id = server.add_task([arg] { return std::sqrt(arg); });
        double result = server.request_result(id);

        {
            std::lock_guard<std::mutex> lock(out_mutex);
            std::cout << std::fixed << std::setprecision(5);
            std::cout << " sqrt(" << arg << ") = " << result << "\n";
            outfile << arg << " " << result << "\n"; // Сохранение в файл
        }
    }
    outfile.close(); // Закрытие файла после завершения
}

// Функция клиента для возведения в степень
void client_pow(Server<double>& server, int n) {
    std::ofstream outfile("pow_results.txt");
    for (int i = 0; i < n; ++i) {
        double base = random_double(1, 10);
        double exponent = random_double(1, 5);
        size_t id = server.add_task([base, exponent] { return std::pow(base, exponent); });
        double result = server.request_result(id);

        {
            std::lock_guard<std::mutex> lock(out_mutex);
            std::cout << std::fixed << std::setprecision(5);
            std::cout << " pow(" << base << ", " << exponent << ") = " << result << "\n";
            outfile << base << " " << exponent << " " << result << "\n"; // Сохранение в файл
        }
    }
    outfile.close(); // Закрытие файла после завершения
}

int main() {
    const int N = 10000;
    Server<double> server;

    // Запускаем сервер и замеряем время
    auto start_server = std::chrono::high_resolution_clock::now();
    server.start();
    auto end_server = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> server_start_duration = end_server - start_server;
    
    std::cout << "Время запуска сервера: " << server_start_duration.count() << " секунд" << std::endl;

    // Запускаем клиенты и замеряем общее время выполнения
    auto start_clients = std::chrono::high_resolution_clock::now();

    std::thread client1(client_sin, std::ref(server), N);
    std::thread client2(client_sqrt, std::ref(server), N);
    std::thread client3(client_pow, std::ref(server), N);

    client1.join();
    client2.join();
    client3.join();

    auto end_clients = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> clients_duration = end_clients - start_clients;

    // Останавливаем сервер и замеряем время
    auto start_stop_server = std::chrono::high_resolution_clock::now();
    server.stop();
}
