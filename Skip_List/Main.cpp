#include <vector>
#include <iostream>
#include <memory>
#include <list>
#include <utility>
#include <unordered_map>
#include <ctime>
#include <shared_mutex>
#include <thread>
#include <chrono>
#include <limits>
#include <mutex>
//================================================================================================================
struct Node
{
	Node(Node *n, Node *d, int k)
		: next(n), down(d), key(k)
	{
	}

	Node *next;			// link to next element lvl
	Node *down;			// link to element down

	int key;			// element key
};
//================================================================================================================
class Skip_List
{
public:
	Skip_List(Node *h = nullptr, Node *t = nullptr)
		: head(h), tail(t)
	{
		if (!head && !tail) {				// Создание границ для head/tail
			auto h_node = std::make_unique<Node>(nullptr, nullptr, INT_MIN);
			auto t_node = std::make_unique<Node>(nullptr, nullptr, INT_MAX);
			head = h_node.get();
			tail = t_node.get();
			head->next = tail;
			storage.push_back(std::move(h_node));
			storage.push_back(std::move(t_node));
		}
	}

	Node *search(int k) const;
	Node *insert_elem(int k);
	Node *erase_elem(int k);
	// Запрет копирования
	Skip_List(const Skip_List &) = delete;
	Skip_List &operator=(const Skip_List &) = delete;
	// Разрешение перемещения
	Skip_List(Skip_List &&) noexcept = default;
	Skip_List &operator=(Skip_List &&) noexcept = default;

	~Skip_List() = default;

private:
	int max_lvl = 32;
	Node *head;										// Сырые указатели
	Node *tail;										// Сырые указатели
	std::list<std::unique_ptr<Node>> storage;		// Единственный владелец всех узлов(Хранилище)
	
	using storage_it = std::list<std::unique_ptr<Node>>::iterator;
	std::unordered_multimap<int, storage_it> index;	// Хранит итераторы(только наблюдатели)

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

	while (current && current != tail) {			// Цикл пока не пришли в конец списка

		if (!current->next) {
			if (current->down)
				current = current->down;
			else
				return nullptr;
			continue;
		}

		if (current->next->key > k) {				// Проверяем значение следующего элемента на текущем уровне
			if (!current->down)
				return nullptr;
			current = current->down;				// Двигаем поиск на уровень ниже
		}

		else if (current->next == tail) {			// Проверяем не является ли следующим элементом конец Списка
			if (!current->down)
				return nullptr;
			current = current->down;				// Двигаем поиск на уровень ниже
		}

		else if (current->next->key == k)			// Следующий элемент соответствует искомому
			return current->next;					// Вернули ссылку на найденный элемент

		else if (current->next->key < k)
			current = current->next;				// Двигаемся к следующему элементу на текущем уровне

		else return nullptr;
	}
}
//================================================================================================================
Node *Skip_List::insert_elem(int k)
{
	std::unique_lock<std::shared_mutex> lock(mtx_);	// Блокировка на запись

	const unsigned long long m = 1ULL << 31;		// Константа для LCG

	Node *current = head;

	Node *update[32] = {};							// Хранилище: после каких узлов строится башня=(next)  Сырые указатели
	Node *tower[32] = {};							// Хранилище: для строительства этажей башни===(down)  Сырые указатели

	if (!current)
		return nullptr;

	int height = 1;
	while (height < max_lvl && (randint() & 1))		// Генерация высоты
		++height;

	for (int i = max_lvl - 1; i >= 0; --i) {		// Поиск + заполнение update
		if (!current) return nullptr;

		while (current->next && current->next != tail && current->next->key < k)
			current = current->next;

		if (i < height)
			update[i] = current;

		if (current->down)
			current = current->down;
	}
	// Создать все узлы

	Node *n = nullptr;
	Node *d = nullptr;
	auto node = std::make_unique<Node>(n, d, k);	// Создаёт Node(n,d,k) и заворачивает в unique_ptr
	Node *ptr = node.get();							// raw для навигации
	
	auto iter = storage.insert(storage.end(), std::move(node));// Начинаем заполнять хранилище узлами
	index.insert({ k, iter });						// Добавляем пару ключ-итератор
	tower[0] = ptr;									// Добавляем узел на 0 ур

	for (int j = 1; j < height; ++j) {				// Создаём узлы с 1 ур выше
		auto temp_ptr = std::make_unique<Node>(n, d, k);
		Node *raw = temp_ptr.get();					// Для навигации
		auto temp_iter = storage.insert(storage.end(), std::move(temp_ptr));// Продолжаем заполнять хранилище, передаем право владения в storage при помощи move-семантики
		index.insert({ k, temp_iter });				// Добавляем пару ключ-итератор
		tower[j] = raw;								// Заполяняется башня уровнями
		raw->down = tower[j - 1];					// Строим *down связи (снизу-вверх) башни
	}

	for (int i = 0; i < height; ++i) {				// Вставить все узлы ч/з update
		if (update[i]) {
			tower[i]->next = update[i]->next;		// Строим next связи на вставляемый объект
			update[i]->next = tower[i];
		}
	}

	return tower[0];
}
//================================================================================================================
Node *Skip_List::erase_elem(int k)
{
	std::unique_lock<std::shared_mutex> lock(mtx_);	// Блокировка на запись

	Node *update[32] = {};							// Сырые указатели
	Node *current = head;							// Сырой указатель

	if (!current) return nullptr;

	for (int i = max_lvl - 1; i >= 0; --i) {		// Заполнение update[] - Для всех урвоней
		if (!current) break;

		while (current->next && current->next != tail && current->next->key < k)
			current = current->next;				// Двигаемся вправо по списку пока следующий элемент меньше чем целевой

		update[i] = current;						// Запоминаем позицию перед спуском
		if (current->down) current = current->down;
	}

	Node *del_node = update[0]->next;				

	if (!del_node || del_node == tail || del_node->key != k)
		return nullptr;

	std::vector<Node *> deling;

	for (int i = 0; i < max_lvl; ++i) {
		if (update[i]->next && update[i]->next->key == k)// Разрыв связей для последующего удаления
		{
			Node *deli = update[i]->next;
			update[i]->next = deli->next;
			deling.push_back(deli);
		}
		else break;
	}

	auto map_it = index.equal_range(k);

	for (auto it = map_it.first; it != map_it.second; ++it) {
		Node *ptr = it->second->get();					// Возвращает сырой указатель на управляемый объект
		bool is_deling = false;

		for (Node *s : deling)
			if (s == ptr) {
				is_deling = true;
				break;
			}

		if (is_deling)
			storage.erase(it->second);				// Уничтожается unique_ptr, вызывается деструктор Node, память свободна
	}
	index.erase(k);									// Удаление всех пар с ключом

	return del_node;								// Для возможности использовать bool - узнать статус удаления
}
//================================================================================================================
