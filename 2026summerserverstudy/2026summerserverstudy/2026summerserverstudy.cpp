#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include <chrono>

//TLS
// 각 스레드마다 가지는 고유한 저장공간
// 힙에서 자기가 사용할 데이터를 충분히 꺼내 TLS에 저장하고 거기서 꺼내 사용
// 데이터에 접근하기 위해 경합하는 시간을 줄인다
// 스책은 기본적으로 함수를 위한 공간이고 데이터를 저장해 놓기에는 불안정하다

thread_local int32 LThreadid = 0;

void ThreadMain(int32 threadid) {
	LThreadid = threadid;
	while (true) {
		cout << "thread id : " << LThreadid << endl;
		this_thread::sleep_for(1s);
	}
}

int main() {

	vector<thread> threads;

	for (int32 i = 0; i < 10; i++) {
		int32 threadid = i + 1;
		threads.push_back(thread(ThreadMain, threadid));
	}
	for (thread& t : threads)
		t.join();
}