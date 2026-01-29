#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>// lock용 해더

vector<int32> v;
//Mutual Exclusion : 상호 배타적
mutex m;//일종의 자물쇠

//RAII 패턴 (Resource Acquisition Is Initialization)

void Push(){
	for (int32 i = 0; i < 10000; i++) {
		m.lock();//자물쇠 잠금
		//m.lock(); 일반 뮤텍스는 재귀적 불가 데드락발생
		std::unique_lock<std::mutex> uniq_lock(m, std::defer_lock);// unique_lock : 만들자 마자 잠기지 않고 잠기는 시점을 정할 수 있음
		v.push_back(i);

		if (i == 5000) {
			m.unlock();
			break;
		}

		m.unlock();//자물쇠 해제
		//m.unlock();
	}
}

int main()
{
	std::thread t1(Push);
	std::thread t2(Push);

	t1.join();
	t2.join();

	cout << v.size() << endl;
	
}
