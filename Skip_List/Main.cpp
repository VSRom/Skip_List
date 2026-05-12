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
	Node *head;										// start list
	Node *tail;										// end list
	std::list<std::unique_ptr<Node>> storage;		// хранилище уникальных указателей на узлы (из vector в list для erase)
													// Итератор на unique_ptr<Node> в list
	using storage_it = std::list<std::unique_ptr<Node>>::iterator;
	std::unordered_multimap<int, storage_it> index;	// Хеш-таблтца ключ для итератора в storage (для erase)

	mutable std::shared_mutex mtx_;					// mutable для блокировки в const foo
};
//================================================================================================================
Node *Skip_List::search(int k) const
{
	std::shared_lock<std::shared_mutex> lock(mtx_);	// Блокировка на чтение

	Node *current = head;

	if (!current)
		return nullptr;

	while (current && current != tail) {	// Цикл пока не пришли в конец списка

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
				current = current->down;			// Двигаем поиск на уровень ниже
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

	const unsigned long long m = 1ULL << 31;

	Node *current = head;

	Node *update[32] = {};							// Хранилище: после каких узлов строится башня=(next)
	Node *tower[32] = {};							// Хранилище: для строительства этажей башни===(down)

	if (!current)
		return nullptr;

	// Генерация высоты
    int height = 1;
	while (height < max_lvl && (randint() & 1))
            ++height;

	// Поиск + заполнение update

    for (int i = max_lvl - 1; i >= 0; --i) {
        if (!current) return nullptr;

        while (current->next && current->next != tail && current->next->key < k)
            current = current->next;

        // Проверяем не является ли следующий элемент концом списка ИЛИ следующий элемент больше чем целевой
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
            // Начинаем заполнять хранилище
            auto iter = storage.insert(storage.end(), std::move(node));
            index.insert({ k, iter });						// Добавляем пару ключ-итератор
            tower[0] = ptr;									// Добавляю в памяти "на 0 этаж" вставляемый объект

		for (int j = 1; j < height; ++j) {
			// Создаём узел на 1 ур выше
			auto temp_ptr = std::make_unique<Node>(n, d, k);
			Node *raw = temp_ptr.get();				// Для навигации
			// Продолжаем заполнять хранилище
			auto temp_iter = storage.insert(storage.end(), std::move(temp_ptr));
			index.insert({ k, temp_iter });
			tower[j] = raw;							// Заполяняется башня уровнями
			raw->down = tower[j - 1];				// Строим *down связи (снизу-вверх) башни
		}

		// Вставить все узлы ч/з update

        for (int i = 0; i < height; ++i) {
            if (update[i]) {
                // проверим что не разыименовываем nullptr
                tower[i]->next = update[i]->next;	// Строим Next связи на вставляемый объект
                update[i]->next = tower[i];
            }
        }

	return tower[0];
}
//================================================================================================================
Node *Skip_List::erase_elem(int k)
{
    std::unique_lock<std::shared_mutex> lock(mtx_);	// Блокировка на запись

    Node *update[32] = {};							// Хранилище: пути поиска
    Node *current = head;

    if (!current) return nullptr;

    // Заполнение update[] - Для всех урвоней
    for (int i = max_lvl - 1; i >= 0; --i) {
        // Двигаемся вправо по списку пока следующий элемент меньше чем целевой
        if (!current) break;

        while (current->next && current->next != tail && current->next->key < k)
            current = current->next;

        update[i] = current;

        if (current->down) current = current->down;
    }

    Node *del_node = update[0]->next;		//Ищем удаляемый узел

    if (!del_node || del_node == tail || del_node->key != k)
        return nullptr;

    // Удаление элемента на всех уровнях списка

    std::vector<Node *> deling;

    for (int i = 0; i < max_lvl; ++i) {
        if (update[i]->next && update[i]->next->key == k)
        {
            Node *deli = update[i]->next;
            update[i]->next = deli->next;
            deling.push_back(deli);
        }
        else break;
    }

    auto map_it = index.equal_range(k);					// Получение итератора

    // Удаление из storage  Находим все итераторы для нашего ключа и чистим

    for (auto it = map_it.first; it != map_it.second; ++it) {
        Node *ptr = it->second->get();
        bool is_deling = false;

    for (Node *s : deling)
        if (s == ptr) {
            is_deling = true;
            break;
        }

    if (is_deling)
        storage.erase(it->second);
}
	index.erase(k);									// Удаление всех пар с ключом

	return del_node;								// Для возможности использовать bool - узнать статус удаления
}
//================================================================================================================
int main()
{
	return 0;
}
//================================================================================================================
