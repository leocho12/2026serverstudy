#pragma once
#include <mutex>

template <typename T>
class LockStack {
public:
	//생성자는 아무것도 안함
	LockStack() {}
	//복사시도 차단
	LockStack(const LockStack&) = delete;
	LockStack& operator=(const LockStack&) = delete;

	void Push(T value) {
		//락 잠금
		lock_guard<mutex> lock(_mutex);
		//스택에 넣을때 이동해서 넣을 수 있으면 이동으로 넣어줌
		_stack.push(std::move(value));
		_condVar.notify_one();
	}

	bool TryPop(T& value) {
		lock_guard<mutex>lock(_mutex);
		if (_stack.empty())
			return false;

		value = std::move(_stack.top());
		_stack.pop();
		return true;
	}
	//멀쓰에서는 empty가 큰 의미 없음
	/*
	void Empty() {
		lock_guard<mutex> lock(_mutex);
		return _stack.empty();
	}
	*/

	void WaitPop(T& value) {
		//내부적으로 락을 풀고 걸어야 하기 때문에 unique_lock 사용
		unique_lock<mutex> lock(_mutex);
		//락을 걸고 들어가서 조건에 맞지 않으면 락을 풀고 잠듦
		_condVar.wait(lock, [this] {return _stack.empty() == false; });
		//조건이 맞으면 다시 락을 걸고 아래로 진행
		value = std::move(_stack.top());
		_stack.pop();
	}

private:
	stack<T> _stack;
	mutex _mutex;
	condition_variable _condVar;
};