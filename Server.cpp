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
#include <fstream>
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

void client_sin(Server<double>& server, int n, std::ofstream& out_file) {
    for (int i = 0; i < n; ++i) {
        double arg = random_double(0, 2 * 3.14);
        size_t id = server.add_task([arg] { return std::sin(arg); });
        double result = server.request_result(id);

        {
            std::lock_guard<std::mutex> lock(out_mutex);
            out_file << std::fixed << std::setprecision(5);
            out_file << " sin(" << arg << ") = " << result << "\n";
        }
    }
}

void client_sqrt(Server<double>& server, int n, std::ofstream& out_file) {
    for (int i = 0; i < n; ++i) {
        double arg = random_double(0, 100);
        size_t id = server.add_task([arg] { return std::sqrt(arg); });
        double result = server.request_result(id);

        {
            std::lock_guard<std::mutex> lock(out_mutex);
            out_file << std::fixed << std::setprecision(5);
            out_file << " sqrt(" << arg << ") = " << result << "\n";
        }
    }
}

void client_pow(Server<double>& server, int n, std::ofstream& out_file) {
    for (int i = 0; i < n; ++i) {
        double base = random_double(1, 10);
        double exponent = random_double(1, 5);
        size_t id = server.add_task([base, exponent] { return std::pow(base, exponent); });
        double result = server.request_result(id);

        {
            std::lock_guard<std::mutex> lock(out_mutex);
            out_file << std::fixed << std::setprecision(5);
            out_file << " pow(" << base << ", " << exponent << ") = " << result << "\n";
        }
    }
}

int main() {
    const int N = 5;
    Server<double> server;

    server.start();

    
    std::ofstream out_file("results.txt");
    if (!out_file.is_open()) {
        std::cerr << "Ошибка открытия файла!" << std::endl;
        return 1;
    }

    std::thread client1(client_sin, std::ref(server), N, std::ref(out_file));
    std::thread client2(client_sqrt, std::ref(server), N, std::ref(out_file));
    std::thread client3(client_pow, std::ref(server), N, std::ref(out_file));

    client1.join();
    client2.join();
    client3.join();

    server.stop();

    
    out_file.close();

    return 0;
}
