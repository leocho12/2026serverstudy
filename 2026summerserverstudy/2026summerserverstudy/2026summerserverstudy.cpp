#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>// lock용 해더


//Sleep

class SpinLock {
public:
	void lock() {
		// CAS (Compare-And-Swap)

		bool expected = false;
		bool desired = true;
		
		while (_locked.compare_exchange_strong(expected, desired) == false) {
			expected = false;

			//this_thread::sleep_for(std::chrono::milliseconds(100));
			//this_thread::sleep_for(0ms);//지정한 시간까지 스레드를 일시정지
			this_thread::yield();//다른 스레드에게 실행 기회를 양보
		}
	}

	void unlock() {
		_locked.store(false);
	}
private:
	atomic<bool> _locked = false;//동시 락 획득을 방지하기 위해 atomic으로 선언
};

int32 sum = 0;
mutex m;
SpinLock spinLock;

void Add() {
	for (int32 i = 0; i < 100000; i++) {
		//lock_guard<사용할 락의 타입>락가드의 변수명(잠글 락 객체)
		lock_guard<SpinLock>guard(spinLock);
		sum++;
	}
}

void Sub() {
	for (int32 i = 0; i < 100000; i++) {
		lock_guard<SpinLock>guard(spinLock);
		sum--;
	}
}

int main()
{
	thread t1(Add);
	thread t2(Sub);

	t1.join();
	t2.join();

	cout << sum << endl;


}
