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
	struct Node {
		Node(const T& value) : data(value), next(nullptr){

		}
		T data;
		Node* next;
	};

public:
	void Push(const T& value) {
		Node* node = new Node(value);
		node->next = _head;

		/*if (_head == node->next) {
			_head = node;
			return;
		}
		else {
			node->next = _head;
			return false;
		}*/
		//이걸 원자적으로 한번에 실행하는게 compare_exchange_weak
		while(_head.compare_exchange_weak(node->next, node)==false) {

		}
	}

	bool TryPop(T& value) {
		++_popCount;
		Node* oldHead = _head;

		while(oldHead && _head.compare_exchange_weak(oldHead, oldHead->next)==false) {
			
		}
		if (oldHead == nullptr) {
			--_popCount;
			return false;
		}
		value = oldHead->data;
		
		TryDelete(oldHead);
		
		return true;
	}

	//1.데이터 분리
	//2. count체크
	//3. 혼자면 삭제
	//4. 여러명이면 pendingList에 추가
	void TryDelete(Node* oldHead) {
		//나 이외에	pop을 실행중인 쓰레드가 없으면
		if (_popCount == 1) {
			//나 혼자만 실행중

			//pendingList에 남아있는 노드들 삭제
			Node* node = _pendingList.exchange(nullptr);//값 빼오기
			if (--_popCount == 0) {
				//끼어든	쓰레드 없음->삭제
				//다른 쓰레드가 끼어들어도 데이터는 분리되어있음
				DeleteNodes(node);
			}
			else if(node){
				//다른 쓰레드가 끼어들었음->pendingList에 다시 연결
				ChainPendingNodeList(node);
			}

			//내 데이터 삭제
			delete oldHead;
		}
		else {
			//나 이외에 pop 실행중인 쓰레드가 있음->pendingList에 내 데이터 연결
			ChainPendingNodeList(oldHead);
			--_popCount;
		}
	}

	void ChainPendingNodeList(Node* first, Node* last) {
		last->next = _pendingList;

		while(_pendingList.compare_exchange_weak(last->next, first) == false) {

		}
	}

	void ChainPendingNodeList(Node* node) {
		Node* last = node;
		while(last->next)
			last = last->next;

		ChainPendingNodeList(node, last);
	}

	void ChainPendingNode(Node* node) {
		ChainPendingNodeList(node, node);
	}

	static void DeleteNodes(Node* node) {
		while (node) {
			Node* next = node->next;
			delete node;
			node = next;
		}
	}

private:
	atomic<Node*> _head;

	atomic<uint32>_popCount = 0;// pop을 실행중인 쓰레드 개수
	atomic<Node*> _pendingList;// 삭제 되어야 할 노드들(첫번째 노드만 기억)
};