#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>// lock용 해더
#include <windows.h>
#include <chrono>


//이벤트


mutex m;
queue<int32> q;
HANDLE handle;

void Producer() {
	while (true) {
		unique_lock<mutex> lock(m);
		q.push(100);
	}
	::SetEvent(handle);

	this_thread::sleep_for(10000ms);
}

void Consumer() {
	while (true) {
		::WaitForSingleObject(handle, INFINITE);//signal,nonsignal 상태에 따라 더이상 내려가지 않고 대기
		//::ResetEvent(handle); //수동리셋이라면 여기서 리셋

		unique_lock<mutex> lock(m);
		if (q.empty() == false) {
			int32 data = q.front();
			q.pop();
			cout << data << endl;
		}
	}
}

int main()
{
	// 커널 오브젝트(운체의 커널이 관리하는 객체)
	// Usage Count(몇명이 이 오브젝트를 사용하고있나)
	// signal (파란불)/ Nonsignal (빨간불) << bool
	// auto / manual << bool
	handle = ::CreateEvent(NULL/*보안속성*/, FALSE/*수동리셋여부*/, FALSE/*이벤트초기상태*/,NULL/*이름*/);

	thread t1(Producer);
	thread t2(Consumer);

	t1.join();
	t2.join();

	::CloseHandle(handle);
}
