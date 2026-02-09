#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>

//메모리 모델(메모리 정책)
// 어떤 식으로 메모리를 접근하고 동기화 할건가?
// 1.sequentially consistent (seq_cst)
// -가장 엄격 = 컴파일러 최적화 여지 적음 = 직관적
// -가시성 문제, 코드 재배치 문제 없음
// -명시 안하면 이걸로 적용
// 2.acquire-release
// -중간 정도 엄격
// -release 명령 이전의 메모리 병령들이 해당 명령 이후 재배치 되는것을 금지
// -acquire로 같은 변수를 읽는 쓰레드가 있다면
// -release 이전의 명령들이 -> acquire 하는 순간에 관찰 가능	
// 3.relaxed
// -가장 느슨 = 컴파일러 최적화 여지 큼 = 직관적이지 않음	
// -가시성 문제, 코드 재배치 문제 발생 가능
// -가장 기본 조건(동일 객체에 대한 동일 관전 순서만 보장)

/*
atomic<bool> flag;

int main()
{
	flag = false;

	//flag = true;
	flag.store(true, memory_order::memory_order_seq_cst);

	//bool val = flag;
	bool val = flag.load(memory_order::memory_order_seq_cst);

	//이전 값을 prev에 넣고 flag값을 수정
	{
		//대입에 두단계를 거치지 않고 atomic한 연산으로 한번에 처리
		bool prev = flag.exchange(true);
	}

	// CAS (Compare And Swap) 조건부 수정
	{
		bool expected = false;
		bool desired = true;

		flag.compare_exchange_strong(expected, desired);
		//expected : flag의 이전값
		//desired : flag에 새로 넣을 값

		if (flag == expected) {
			//다른 쓰레드의 방해를 받아 중간에 실패할 수 있음
			//if (fail)
				//return false;

			//expected==flag
			flag = desired;
			return true;
		}
		else {
			expected = flag;
			return false;
		}
		
		while (true) {
			bool expected = false;
			bool desired = true;
			flag.compare_exchange_weak(expected, desired);
		}

		// compare_exchange_strong과 compare_exchange_weak의 차이
		// strong
		// -값이 같으면 반드시 성공
		// -가짜 실패(spurious failure)없음
		// -실패할 경우 성공할때 까지 알아서 반복
		// -weak보다 무거움

		// weak
		// -값이 같아도 가짜 실패(spurious failure) 때문에 실패할 수 있음
		// -무조건 while문과 함께 사용

	}

}
*/


