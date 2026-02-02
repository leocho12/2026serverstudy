#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>// lock용 해더
#include <windows.h>
#include <chrono>


//컨디션 변수


mutex m;
queue<int32> q;
HANDLE handle;

condition_variable cv;
// 참고) 컨디션 변수는 유저레벨 오브젝트

void Producer() {

	while (true) {
	// 1. Lock을 잡고
		unique_lock<mutex> lock(m);
	// 2. 공유변수 값 수정
		q.push(100);
	}
	// 3. Lock 해제(자동)

	// 4. 컨디션 변수로 다른 쓰레드에 통지
	cv.notify_one(); // wait중인 쓰레드 하나를 깨운다.

	//this_thread::sleep_for(10000ms);
}

void Consumer() {
	while (true) {
		
		// 1. Lock을 잡고
		unique_lock<mutex> lock(m);

		// 2. 조건확인
		cv.wait(lock, [](){return q.empty() == false;});
		// -만족X -> Lock 해제 + 대기

		// -만족O -> Lock 유지 + 진행
		//while (q.empty() == false) 컨디션 변수 안에 루프가 포함되어 있음
		{
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
