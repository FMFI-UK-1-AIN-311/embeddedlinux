#include <stdio.h>
#include <string>
#include <stdexcept>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <chrono>
#include <thread>
#include "music.h"

#define DIR 23
#define STEP 24

#define GPIO_SYS "/sys/class/gpio/"

bool fileExists(const std::string &fname)
{
	struct stat buffer;
	return stat(fname.c_str(), &buffer) == 0;
}

template<typename T>
void writeFile(const std::string &fname, const T &what)
{
	std::ofstream f(fname);
	f << what;
	if (!f.good())
		throw(std::runtime_error(std::string("Can't write to ") + fname));
}

template<typename T>
void readFromFile(const std::string &fname, T &what)
{
	std::ifstream f(fname);
	f >> what;
	if (!f.good())
		throw(std::runtime_error(std::string("Can't read from ") + fname));
}

class Gpio {
	const int pin;
	std::string valuePath;
	std::fstream valueFile;
	std::ifstream valueIn;
public:
	enum Dir { In, Out };
	const char *toString(Dir dir)
	{
		switch (dir) {
			case In: return "in";
			case Out: return "out";
			default: return "unknown";
		}
	}
	Gpio(int pin, Dir dir)
		: pin(pin)
	{
		auto path = std::string(GPIO_SYS "/gpio") + std::to_string(pin);
		auto dirPath = path + "/direction";
		valuePath = path + "/value";

		std::cerr << "path " << path << " valuePath " << valuePath << std::endl;
		if (!fileExists(path)) {
			std::cerr << "Exporting pin " << pin << std::endl;
			writeFile(GPIO_SYS "/export", pin);
			usleep(100000);
		}

		std::cerr << "Configuring pin " << pin << " as " << toString(dir) << std::endl;
		writeFile(dirPath, toString(dir));

		valueIn.rdbuf()->pubsetbuf(0,0);
		valueFile.open(valuePath);
		if (!valueFile.good())
			throw(std::runtime_error(std::string("Can't open ") + valuePath));
		valueIn.open(valuePath);
		if (!valueIn.good())
			throw(std::runtime_error(std::string("Can't open ") + valuePath));
	}

	void set(int val) {
//		writeFile(valuePath, val);
		valueFile << val;
		valueFile.flush();
		if (!valueFile.good())
			throw(std::runtime_error(std::string("Can't write ") + valuePath));
	}

	int get() {
		int val;
		valueIn.seekg(0, std::ios::beg);
		valueIn >> val;
		if (!valueIn.good())
			throw(std::runtime_error(std::string("Can't read ") + valuePath));
		return val;
	}

	void toggle() {
		int old = get();
		set(!old);
	}
};

class Player {
	Gpio step;
	Gpio dir;
public:
	Player(int stepPin, int dirPin)
		: step(stepPin, Gpio::Out)
		, dir(dirPin, Gpio::Out)
	{
	}

	template<typename D>
	void oneStep(D pause)
	{
			step.set(1);
			std::this_thread::sleep_for(pause);
			step.set(0);
			std::this_thread::sleep_for(pause);
	}

	/**
	 * Move the head to a reasonable position (middle).
	 * Move 80 steps in one direction, then 40 back.
	 * There are 80 tracks, so 80 steps total.
	 * Note: some older drives like to get stuck at one end ;(
	 */
	void init()
	{
		dir.set(0);
		auto d = std::chrono::milliseconds(5);
		for (auto steps : {80, 40}) {
			for (int i = 0; i <steps; ++i) {
				oneStep(d);
			}
			dir.toggle();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	void play(int freq, int duration)
	{
		using clock = std::chrono::high_resolution_clock;
		auto d = std::chrono::milliseconds(duration);
		if (freq == 0) {
			std::this_thread::sleep_for(d);
			return;
		}

		auto start = clock::now();
		auto end = start + d;
		auto halfPeriod = std::chrono::microseconds(1000000 / freq / 2);

		std::cerr << "Playing " << freq << "Hz for " << duration << "ms" << std::endl;
		while (clock::now() < end) {
			dir.toggle();
			oneStep(halfPeriod);
		}
	}
	void play(const Song &song) {
		for (const auto &note : song) {
			play(note.freq, note.duration);
			std::this_thread::sleep_for(std::chrono::milliseconds(note.duration/16));
		}
	}
};



int main(int argc, char *argv[])
{
	Player p(STEP, DIR);

	p.init();
/*
	p.play(261*2, 500);
	p.play(293*2, 500);
	p.play(329*2, 500);
*/

	p.play(PrsiPrsi);

	return 0;
}

/* vim: set sw=4 sts=4 ts=4 noet : */
