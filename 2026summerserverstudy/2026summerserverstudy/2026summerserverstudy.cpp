#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>// lock용 해더
#include <future>

//Future

int64 result;

int64 Calculate() {
	int64 sum = 0;

	for (int32 i = 0; i < 100000; i++) {
		sum += i;
	}
	result = sum;

	return sum;
}

void PromiseWorker(std::promise<string>&& promise) {
	promise.set_value("secret message");
}

void TaskWorker(std::packaged_task<int64(void)>&& task) {
	task();
}

int main()
{
	// 동기(synchronous)실행 : 호출하는 순간 실행
	//int64 sum =Calculate();
	//cout << sum << endl;


	// std::future
	{
		// 언젠가 완료될 작업의 결과물을 약속하는 객체
		// 비동기(asynchronous)실행 : 호출하는 순간 바로 리턴
		// 1) deferred(지연) : 호출하는 순간 실행하지 않고, get()호출하는 순간 실행
		// 2) async(비동기) : 호출하는 순간 별도의 스레드에서 실행
		// 3) deffered | async : 호출하는 순간 상황에 따라 실행
		std::future<int64> future = std::async(std::launch::async, Calculate);

		// 작업수행

		int64 sum = future.get(); // 결과물이 이제 필요하다!

		// 멤버변수를 호출하는 경우
		{
			class Knight {
			public:
				int64 GetHp() { return 100; }
			};
			Knight knight;
			std::future<int64> future2 = std::async(std::launch::async, &Knight::GetHp, knight);
		}
	}


	// std::promise
	{
		// 미래(std::future)에 결과물을 반환할거라고 약속
		// promise로 값을 설정하고, future로 값을 얻어옴
		std::promise<string> promise;
		std::future<string> future = promise.get_future();

		thread t(PromiseWorker, std::move(promise));

		string message = future.get();
		cout << message << endl;

		t.join();
	}


	// std::packaged_task
	{
		// packaged_task로 값을 설정하고, future로 값을 얻어옴
		std::packaged_task<int64(void)> task(Calculate);
		// 결과물 자체를 future객체를 통해 받아올 수 있음
		std::future<int64> future = task.get_future();

		std::thread t(TaskWorker, std::move(task));

		int64 sum = future.get();
		cout << sum << endl;
		t.join();
	}


	// 요약)
	// mutex, condition_variable까지 사용하지 않고 단순한 작업ㅇ르 처리할수있는 방법들이다
	// 일회성으로 한번만 일어나는 이밴트에 대해 효과적
	// 1. future-async
	// 원하는 함수를 비동기적으로 실행
	// 2. promise
	// promise객체로 값을 설정하고, future객체로 값을 얻어옴
	// 3. packaged_task
	// 특정 작업을 나중에 실행하고, future객체로 값을 얻어옴
}
