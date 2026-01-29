#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>

//atomic : all-or-nothing operation 전부 작동하거나 아예 작동하지 않거나
atomic<int32> sum = 0;

void Add() {
	for (int32 i = 0; i < 1000000; i++) {
		sum.fetch_add(1);//atomic클래스 함수-특정 변수의 값을 가져온 후에 해당 변수에 인자로 전달된 값을 더하고, 그 결과를 반환
		/*
		//sum++; //<--이게 실제로는 아래와 같이 동작
		int32 eax = sum;
		eax = eax + 1;
		sum = eax;
		두 쓰레드가 우선순위 없이 동시에 실행되어 값이 일관되지 않음
		*/
	}
}

void Sub() {
	for(int32 i=0;i<1000000; i++) {
		sum.fetch_add(-1);
		/*
		//sum--;
		int32 eax = sum;
		eax = eax - 1;
		sum = eax;
		*/
	}
}

int main()
{
	Add();
	Sub();

	cout << "Final sum: " << sum << endl;

	std::thread t1(Add);
	std::thread t2(Sub);
	t1.join();
	t2.join();
	cout << "Final sum after threads: " << sum << endl;
	
}
