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

template <typename T>
class LockFreeStack {

	struct Node;

	//참조 횟수를 세는 포인터
	struct CountedNodePtr {
		int32 externalCount=0;
		Node* ptr=nullptr;
	};

	struct Node {
		Node(const T& value) : data(make_shared<T>(value)){

		}
		shared_ptr<T> data;
		atomic<int32> internalCount = 0;
		CountedNodePtr next;
	};

public:
	void Push(const T& value) {
		CountedNodePtr node;
		node.ptr = new Node(value);
		node.externalCount = 1;

		node.ptr->next = _head;
		while (_head.compare_exchange_weak(node.ptr->next, node) == false) {

		}
	}

	shared_ptr<T> TryPop() {
		CountedNodePtr oldHead = _head;
			while (true) {
				//참조권 획득
				IncreaseHeadCount(oldHead);
				//최소한 externalCount >=2니까 삭제X
				Node* ptr = oldHead.ptr;

				//데이터 없음
				if(ptr==nullptr)
					return shared_ptr<T>();

				//소유권 획득(ptr->next로 head를 바꿔치기 한 애가 이김)
				if (_head.compare_exchange_strong(oldHead, ptr->next)) {
					shared_ptr<T> res;
					res.swap(ptr->data);

					//삭제를 위한 단계
					//나 말고 다른 놈이 참조하고 있는지 확인
					const int32 countIncrease = oldHead.externalCount - 2;

					if (ptr->internalCount.fetch_add(countIncrease) == -countIncrease)
						delete ptr;

					return res;
				}
				else if(ptr->internalCount.fetch_sub(1)==1){
					//참조권은 얻었으나 소유권은 실패->뒷수습
					delete ptr;
				}
			}
	}

private:

	void IncreaseHeadCount(CountedNodePtr& oldCounter) {
		while (true) {
			CountedNodePtr newCounter = oldCounter;
			newCounter.externalCount++;

			if (_head.compare_exchange_strong(oldCounter, newCounter)) {
				oldCounter.externalCount = newCounter.externalCount;
				break;
			}
		}
	}
	
private:
	atomic<CountedNodePtr> _head;
};