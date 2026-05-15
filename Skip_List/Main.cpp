#include <vector>
#include <memory>
#include <utility>
#include <ctime>
#include <shared_mutex>
#include <algorithm>
#include <mutex>
#include <climits>
#include <iostream>
#include <chrono>
//================================================================================================================
struct Node
{
	Node(int k, int lvl)
		: moving(lvl + 1, nullptr), key(k)
	{
	}

	std::vector<Node *> moving;
	int key;
};
//================================================================================================================
class Skip_List
{
public:
	Skip_List(Node *h = nullptr, Node *t = nullptr)
		: head(h), tail(t)
	{
		if (!head && !tail) {				// Создание границ для head/tail
			auto h_node = std::make_unique<Node>(INT_MIN, max_lvl);
			auto t_node = std::make_unique<Node>(INT_MAX, max_lvl);
			head = h_node.get();
			tail = t_node.get();
			for (int i = 0; i < max_lvl; ++i)
				head->moving[i] = tail;

			storage.push_back(std::move(h_node));
			storage.push_back(std::move(t_node));
		}
	}

	Node *search(int k) const;
	Node *insert_elem(int k);
	bool erase_elem(int k);
	// Запрет копирования
	Skip_List(const Skip_List &) = delete;
	Skip_List &operator=(const Skip_List &) = delete;
	// Разрешение перемещения
	Skip_List(Skip_List &&) noexcept = default;                     // noexcept - функция не выдаст исключение!
	Skip_List &operator=(Skip_List &&) noexcept = default;

	~Skip_List() = default;

private:
	Node *head;										// Сырые указатели
	Node *tail;										// Сырые указатели
	int max_lvl = 32;
	std::vector<std::unique_ptr<Node>> storage;

	mutable std::shared_mutex mtx_;					// mutable для блокировки в const foo
};
//================================================================================================================
long long randint()
{
	const unsigned long long m = 1ULL << 31;
	const unsigned long long a = 1'103'515'245;
	const unsigned long long c = 12'345;


	static unsigned long long state;

	if (state == 0)
	{
		unsigned long long seed = static_cast<unsigned long long>(time(nullptr));
		state = seed;
	}

	state = (a * state + c) % m;
	return state;
}
//================================================================================================================
Node *Skip_List::search(int k) const
{
	std::shared_lock<std::shared_mutex> lock(mtx_);	// Блокировка на чтение mutable для const в Skip_List.h

	Node *current = head;

	if (!current)
		return nullptr;

		for (int i = max_lvl - 1; i >= 0; --i)
		{
			while (current->moving[i] && current->moving[i] != tail && current->moving[i]->key < k)
				current = current->moving[i];

			if (current->moving[i] && current->moving[i] != tail && current->moving[i]->key == k)	// Следующий элемент соответствует искомому
				return current->moving[i];			// Вернули ссылку на найденный элемент
		}
	return nullptr;
}
//================================================================================================================
Node *Skip_List::insert_elem(int k)
{
	std::unique_lock<std::shared_mutex> lock(mtx_);	// Блокировка на запись
	const unsigned long long m = 1ULL << 31;		// Константа для LCG
	Node *current = head;
	Node *update[32] = {};							// Хранилище: Сырые указатели

	if (!current)
		return nullptr;

	int height = 1;
	while (height < max_lvl && (randint() & 1))		// Генерация высоты
		++height;

	for (int i = max_lvl - 1; i >= 0; --i) {		// Поиск + заполнение update
		if (!current) return nullptr;

		while (current->moving[i] && current->moving[i] != tail && current->moving[i]->key < k)
			current = current->moving[i];

		if (i < height)
			update[i] = current;
	}
	if ((current->moving[0] && current->moving[0] != tail) && current->moving[0]->key == k)
		return nullptr;

	auto node = std::make_unique<Node>(k, height);	// Создаёт Node(k, height) и заворачивает в unique_ptr
	Node *ptr = node.get();							// raw_ptr для навигации

	for (int i = 0; i < height; i++) {
		ptr->moving[i] = update[i]->moving[i];		// Новый узел, смотрит туда же, куда смотрел предыдущий
		update[i]->moving[i] = ptr;					// Предыдущий узел, теперь твой сосед — это новый узел
	}
	storage.push_back(std::move(node));

	return ptr;
}
//================================================================================================================
bool Skip_List::erase_elem(int k)
{
	std::unique_lock<std::shared_mutex> lock(mtx_);	// Блокировка на запись

	Node *update[32] = {};
	Node *current = head;

	if (!current) return false;

	for (int i = max_lvl - 1; i >= 0; --i) {		// Заполнение update[] - Для всех урвоней
		if (!current) break;

		while (current->moving[i] && current->moving[i] != tail && current->moving[i]->key < k)
			current = current->moving[i];				// Двигаемся вправо по списку пока следующий элемент меньше чем целевой

		update[i] = current;						// Запоминаем позицию перед спуском
	}

	Node *del_node = update[0]->moving[0];				

	if (!del_node || del_node == tail || del_node->key != k)
		return false;

	for (int i = 0; i < max_lvl; ++i) {
		if (update[i]->moving[i] && update[i]->moving[i] == del_node)// Разрыв связей для последующего удаления
			update[i]->moving[i] = update[i]->moving[i]->moving[i];
		else break;
	}

	auto ptr_del = std::remove_if(storage.begin(), storage.end(), [del_node](auto &ptr) { return ptr.get() == del_node; });

	storage.erase(ptr_del, storage.end());

	return true;
}
//================================================================================================================
