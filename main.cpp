#include <condition_variable>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <cstdlib>

#include "factory.h"
#include "figures.h"
#include "subscriber.h"

int main(int argc, char *argv[]) {
	if (argc != 2) {//проверка на правильность аргументов
		std::cout << "Please start program with argument (./OOP8 {size})" << std::endl;
		return 1;
	}

	size_t vector_size = std::atoi(argv[1]);
	Factory factory;//создание фабрики фигур

	Subscriber subscriber;//создание подписчика
	subscriber.buffer.reserve(vector_size);
	subscriber.processes.push_back(std::make_shared<Console_process>());
	subscriber.processes.push_back(std::make_shared<File_process>());

	std::thread subscriber_thread(std::ref(subscriber));//выделение потока для подписчика

	std::string cmd;//выбор команды
	std::cout << "OOP lab #8. Author: Velesov Daniil M8O - 208B - 19." << std::endl;
    std::cout << "Hello there. It is a Figure Buffer.\nAdd Figures to Figures Vector until it will be full.\nAfter that, program will calculate information about figures and show it on screen and in file." << std::endl;
	std::cout << "Do you want add figure or close program?" << std::endl;
	std::cout << "a - add figure" << std::endl;
	std::cout << "q - exit" << std::endl;
	while (std::cin >> cmd) {
		std::unique_lock<std::mutex> main_lock(subscriber.mtx);//работа с потоком "подписчика"
		if (cmd == "q") {//Выход из программы
			subscriber.end = true;
			subscriber.cv.notify_all();
			break;
		} else if (cmd == "a") {//Добавление фигур
			std::string figure_type; //выбор типа фигуры

			for (size_t id = 0; id < vector_size; id++) {
                std::cout << "Which figure you want to add?" << std::endl;
                std::cout << "t - Triangle" << std::endl;
                std::cout << "r - Rectangle" << std::endl;
                std::cout << "s - Square" << std::endl;
				std::cin >> figure_type;
				if (figure_type == "t") {//создание треугольника
					std::pair<double, double> *vertices = new std::pair<double, double>[3];
					for (int i = 0; i < 3; i++) {
						std::cin >> vertices[i].first >> vertices[i].second;
					}
					try {
						subscriber.buffer.push_back(factory.FigureCreate(TRIANGLE, vertices, id));
					} catch (std::logic_error &e) {
						std::cout << e.what() << "\n";
						id--;
					}
				} else if (figure_type == "s") {//создание квадрата
					std::pair<double, double> *vertices = new std::pair<double, double>[4];
					for (int i = 0; i < 4; i++) {
						std::cin >> vertices[i].first >> vertices[i].second;
					}
					try {
						subscriber.buffer.push_back(factory.FigureCreate(SQUARE, vertices, id));
					} catch (std::logic_error &e) {
						std::cout << e.what() << "\n";
						id--;
					}
				} else if (figure_type == "r") {//создание прямоугольника
					std::pair<double, double> *vertices = new std::pair<double, double>[4];
					for (int i = 0; i < 4; i++) {
						std::cin >> vertices[i].first >> vertices[i].second;
					}
					try {
						subscriber.buffer.push_back(factory.FigureCreate(RECTANGLE, vertices, id));
					} catch (std::logic_error &e) {
						std::cout << e.what() << "\n";
						id--;
					}
				}
			}

			if (subscriber.buffer.size() == vector_size) {//проверка на заполнение буффера
				subscriber.cv.notify_all();
				subscriber.cv.wait(main_lock, [&subscriber]() {
					return subscriber.success == true;
				});
				subscriber.success = false;
			}
		}
	}

	subscriber_thread.join();

	return 0;
}
