#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace std;

mutex mtx; // инициализация мютекса
condition_variable cv; // для уведомления потока-потребителя о наступившем событии
int ready = 0; // флаг, который сигнализирует о том, наступило событие или нет. 
bool finish = false; // для завершения алгоритма
//Класс для теста, чтобы прередавать его экземпляр по указателю.
class Data {
public:
    int num;
};

Data* data_1 = new Data;

void provide(int num_itr){
    for (int i = 1; i <= num_itr; i++) {
        this_thread::sleep_for(chrono::seconds(1)); // задержка в одну секунду
        lock_guard <mutex> lock(mtx); // блокировка мютекса 

        data_1 = new Data{i};
        ready = 1; // событие наступило
        cout << "Message sent " << i << endl;

        cv.notify_one(); // уведомление потока о наступившем событии
    }
    // для завершения алгоритма
    lock_guard<mutex> lock(mtx);
    finish = true;
    cv.notify_one();
}

void consume() {
    while (true) {
        unique_lock <mutex> lock(mtx);
        cv.wait (lock, [ ] {return ready == 1 || finish;}); // Проверка наступления события + Ожидание наступления события с временным освобождением мьютекса.

        if (finish && ready == 0) break; // проверка условия завершения алгоритма

        cout << "Message received " << data_1->num << endl;
        delete data_1;
        ready = 0;
    }
}

int main() {
    int num_itr_main = 25;

    thread t1(provide, num_itr_main); 
    thread t2(consume); 

    t1.join();
    t2.join();
}