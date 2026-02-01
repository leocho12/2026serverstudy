#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>// lock용 해더


//스핀 락
// 락을 획득할 때 까지 계속 반복해서 시도
// CPU 점유율이 높음
// 컨텍스트 스위칭이 없음
// 빠른 작업에 적합

class SpinLock {
public:
	void lock() {
		// CAS (Compare-And-Swap)

		bool expected = false;
		bool desired = true;
		//CAS 의사코드
		/*
		if (_locked == expected) {
			expected = _locked;
			_locked = desired;
			return true;
		}
		else {
			expected = _locked;
			return false;
		}
		*/
		//이 코드를 아래와 같이 한번에 묶어서 사용할 수	있음
		while (_locked.compare_exchange_strong(expected, desired) == false) {
			expected = false;
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
