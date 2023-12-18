#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <random>
#include <unistd.h>
#include <fstream>

std::random_device rd;
std::mt19937 rng(rd());

// Рандомайзер, принимающий границы, который возвращает рандомное значение между границами
int RandomInt(int min, int max) {
    std::uniform_int_distribution<int> uni(min, max);
    return uni(rng);
}

int generate_guest; // Скорость генерации гостя

pthread_mutex_t rooms[25]; // Номера в отеле
int guestNumber = 0; // Номер гостя
bool taken_rooms[25]{false}; // Записывает какие номера заняты

// Проверка на наличие свободного номера определенного типа
bool ChooseRoom(int type, int& room) {
    bool res = false;
    if (type == 1) {
        for (int i = 0; i < 10; ++i) {
            if (!taken_rooms[i]) {
                res = true;
                taken_rooms[i] = true;
                room = i + 1;
                return res;
            }
        }
    } else if (type == 2) {
        for (int i = 10; i < 20; ++i) {
            if (!taken_rooms[i]) {
                res = true;
                taken_rooms[i] = true;
                room = i + 1;
                return res;
            }
        }
    } else if (type == 3) {
        for (int i = 20; i < 25; ++i) {
            if (!taken_rooms[i]) {
                res = true;
                taken_rooms[i] = true;
                room = i + 1;
                return res;
            }
        }
    }
    std::cout << "There are no rooms of the right category" << "\n";
    return res;
}

// Поток гостя
void* guest(void* args) {
    int number = ++guestNumber;
    int money = RandomInt(1, 10000); // Рандомная генерация количества денег

    std::cout << "A guest with " << money << " comes in with " << money << " gold" << "\n";

    int roomType; // Тип комнаты

    if (money >= 6000) {
        roomType = RandomInt(1, 3);
    } else if (money >= 4000) {
        roomType = RandomInt(1, 2);
    } else if (money >= 2000) {
        roomType = 1;
    } else {
        roomType = 0;
    }

    int roomNum = 0; // Номер комнаты
    if (roomType != 0) { // Если гость не может позволить себе комнату, то сразу уходит
        std::cout << "Guest number " << number << " wants a room of " << roomType * 2000 << " cost" << "\n";

        if (ChooseRoom(roomType, roomNum)) { // Если клиент удачно выбирает свободный номер
            pthread_mutex_lock(&rooms[roomNum]); // Блокирует ресурс
            std::cout << "Guest number " << number << " has taken room number " << roomNum << "\n";
            sleep(20); // Занимает ресурс на 20 секунд
            taken_rooms[roomNum - 1] = false; // Освобождает комнату
            std::cout << "Guest number " << number << " has left room number " << roomNum << "\n";
            pthread_mutex_unlock(&rooms[roomNum]);
        }
    } else {
        std::cout << "Guest number " << number << " doesn't have enough money for a room" << "\n";
    }

    std::cout << "Guest number " << number << " leaves the hotel" << "\n";

    return NULL;
}

int hotel_work_time; // Время работы отеля
bool working = true; // Работает ли отель
pthread_mutex_t hotel_work; // Ресурс администрации

// Поток администрации
void* admin(void* args) {
    pthread_mutex_lock(&hotel_work); // Блокирует ресурс администрации, чтобы другие инициализации не могли его использовать
    sleep(hotel_work_time);
    working = false;
    pthread_mutex_unlock(&hotel_work);

    return NULL;
}

int main(int argc, char* argv[]) { // Принимает входные значения с командной строки
    for (int i = 0; i < 25; ++i) {
        pthread_mutex_init(&rooms[i], NULL); // Инизиализация ресурсов номеров отеля
    }

    pthread_mutex_init(&hotel_work, NULL); // Инициализация ресурса администрации

    hotel_work_time = std::stoi(argv[1]);
    generate_guest = std::stoi(argv[2]);

//    std::cout << "Input hotel work seconds: \n";
//    std::cin >> hotel_work_time;
//    std::cout << "Input guest generate time: \n";
//    std::cin >> generate_guest;
//    std::cout << "\n";

    pthread_t hotel;

    while (working) {
        pthread_create(&hotel, NULL, admin, NULL);
        pthread_create(&hotel, NULL, guest, NULL);

        sleep(generate_guest);
    }

    pthread_join(hotel, NULL);
    std::cout << "\n----The hotel closes----";
}