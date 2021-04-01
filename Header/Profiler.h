#pragma once

#ifdef _DEBUG
	#include <QElapsedTimer.h>
	#include <QString>
	#include <iostream>
	#define TIC(val) Profiler::getInstance().startTimer(val);
	#define TOC(val) Profiler::getInstance().takeTime(val);

	class Profiler
	{
	private:
		Profiler() { timer.start(); };

		QElapsedTimer timer;

	public:
		~Profiler() {};

		static Profiler& getInstance() {
			static Profiler instance;
			return instance;
		}

		void startTimer(QString position = "") {
			std::cout << "from: " << position.toStdString() << std::endl;
			timer.restart();
		};

		void takeTime(QString position = "") {
			std::cout << "to: "<< position.toStdString() << " time elapsed: " << timer.elapsed() << std::endl;
		};
	};

#else
	#define TIC(val)
	#define TOC(val)
#endif
