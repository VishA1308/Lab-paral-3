#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <functional>
#include <future>
#include <random>
#include <fstream>
#include <vector>

#include <cmath>
#define M_PI
template<typename T>
class Server
{
public:
    Server() : stop_signal(false) {}

    void start()
    {
        if (th_count== 0)
        {
            server_thread = std::thread(&Server::process_tasks, this);
            th_count++;
  
        }
        
    }


    void stop()
    {
        
        if (th_count!= 0)
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            stop_signal = true;
            condition.notify_all();
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
        std::lock_guard<std::mutex> lock(queue_mutex);
        return futures[id_res].get();
    }       

private:
    void process_tasks() 
    {
        while (true) {
            size_t task_id;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                condition.wait(lock, [this] { return !tasks_queue.empty() || stop_signal; });
                if (stop_signal)
                {
                    break;
                }
                task_id = tasks_queue.front();
                tasks_queue.pop();
            }

            
            T result = futures[task_id].get();
            std::lock_guard<std::mutex> lock(result_mutex);
            results[task_id] = result;
        }
    }
    std::int16_t th_count;
    std::thread server_thread ; // обработка задач
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

void client_sin(Server<double>& server, int n) 
{
    for (int i = 0; i < n; ++i) {
        double arg = random_double(0, 2 * 3.14);
        size_t id = server.add_task([arg] { return std::sin(arg); });
        double result = server.request_result(id);
        std::cout << "ID: " << id << ", sin(" << arg << ") = " << result << "\n";
    }
}

void client_sqrt(Server<double>& server, int n) 
{
    for (int i = 0; i < n; ++i) {
        double arg = random_double(0, 100);
        size_t id = server.add_task([arg] { return std::sqrt(arg); });
        double result = server.request_result(id);

       
        std::cout << "ID: " << id << ", sqrt(" << arg << ") = " << result << "\n";
    }
}

void client_pow(Server<double>& server, int n)
{
    for (int i = 0; i < n; ++i)
    {
        double base = random_double(1, 10);
        double exponent = random_double(1, 5);
        size_t id = server.add_task([base, exponent] { return std::pow(base, exponent); });
        double result = server.request_result(id);

        
        std::cout << "ID: " << id << ", pow(" << base << ", " << exponent << ") = " << result << "\n";
    }
}
int main() {
    Server<double> server;
    server.start();

   
    std::cout << "Testing sin function:\n";
    for (int i = 0; i < 5; ++i) {
        double arg = random_double(0, 2 * 3.14);
        size_t id = server.add_task([arg] { return std::sin(arg); });
        double result = server.request_result(id);
        std::cout << "ID: " << id << ", sin(" << arg << ") = " << result << "\n";
    }

    
    std::cout << "\nTesting sqrt function:\n";
    for (int i = 0; i < 5; ++i) {
        double arg = random_double(0, 100);
        size_t id = server.add_task([arg] { return std::sqrt(arg); });
        double result = server.request_result(id);
        std::cout << "ID: " << id << ", sqrt(" << arg << ") = " << result << "\n";
    }

    
    std::cout << "\nTesting pow function:\n";
    for (int i = 0; i < 5; ++i) {
        double base = random_double(1, 10);
        double exponent = random_double(1, 5);
        size_t id = server.add_task([base, exponent] { return std::pow(base, exponent); });
        double result = server.request_result(id);
        std::cout << "ID: " << id << ", pow(" << base << ", " << exponent << ") = " << result << "\n";
    }

    
    server.stop();

};
