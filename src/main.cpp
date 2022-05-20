#include "pros/misc.hpp"
#include "pros/rtos.hpp"
#include "rdata/export.hpp"
#include <iostream>

#ifdef WRITE_TO_SD
#include <fstream>
#endif

using namespace rdata;

#ifdef WRITE_TO_SD
std::ofstream outstream("/usd/output.csv", std::ios::out | std::ios::bin | std::ios::trunc);
void initialize() {
	pros::Controller controller(pros::E_CONTROLLER_MASTER);
	while (!pros::usd::is_installed()) {
		controller.set_text(0, 0, "Missing MicroSD!");
		pros::delay(500);
	}
	controller.clear_line(2);
}

#endif//WRITE_TO_SD

void write_data(int delay, std::function<double(void)> func, int* time) {
#ifdef WRITE_TO_SD
	outstream << *time << "," << func() << std::endl;
#else
	std::cout << *time << "," << func() << std::endl;
#endif
	*time += delay;
}

void flush_data() {
#ifdef WRITE_TO_SD
	outstream.flush();
#else
	std::cout.flush();
#endif
}

void _internal_task(void* arg) {
	TaskData* data = (TaskData*)arg;
	int delay = (*data).delay;
	std::function<double(void)> func = (*data).func;

	int time;
#ifndef WRITE_TO_SD
	std::cout << "\06" << std::endl;
#endif
    while (true) {
		write_data(delay, func, &time);
        if (pros::Task::notify_take(true, delay) > 0) {
			flush_data();
			break;
		}
    }
}

pros::Task create_export_task(int delay, std::function<double(void)> func) {
    TaskData data;
    data.delay = delay;
    data.func = func;
    return pros::Task(_internal_task, (void*)(&data), "Data Export Task");
}
