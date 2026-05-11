#include <iostream>
#include <memory>
#include <list>
#include <utility>
#include <unordered_map>
#include <ctime>
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
class Skip_List
{
public:
	Skip_List(Node *h = nullptr, Node *t = nullptr)
		: head(h), tail(t)
	{
	}

	Node *search(int k);
	Node *insert_elem(int k);
	Node *erase(int k);

private:
	int max_lvl = 32;
	Node *head;										// start list
	Node *tail;										// end list
	std::list<std::unique_ptr<Node>> storage;		// хранилище всех узлов (из vector в list для erase)
													// Итератор на unique_ptr<Node> в list
	using storage_it = std::list<std::unique_ptr<Node>>::iterator;
	std::unordered_map<int, storage_it> index;		// Ключ для итератора в storage
};
//================================================================================================================
struct Node
{
	Node(Node *n, Node *d, int k)
		: next(n), down(d), key(k)
	{ }

	Node *next;			// link to next element lvl
	Node *down;			// link to element down

	int key;			// element key
};
//================================================================================================================
Node *Skip_List::search(int k)
{
	Node *current = head;

	if (!current)
		return nullptr;

	while (current != nullptr && current != tail)	// Цикл пока не пришли в конец списка
	{
		if (!current->next)
			return nullptr;

		if (current->next->key > k)					// Проверяем значение следующего элемента на текущем уровне
		{
			if (!current->down)
				return nullptr;
				current = current->down;			// Двигаем поиск на уровень ниже
		}

		 else if (current->next == tail)			// Проверяем не является ли следующим элементом конец Списка
		{
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
	return nullptr;
}
//================================================================================================================
Node *Skip_List::insert_elem(int k)
{
	const unsigned long long m = 1ULL << 31;

	bool result = 1;
	int height = 1;
	Node *current = head;

	Node *update[32] = {};							// Хранилище: после каких узлов строится башня=(next)
	Node *tower[32] = {};							// Хранилище: для строительства этажей башни===(down)

	Node *n = nullptr;
	Node *d = nullptr;

	auto node = std::make_unique<Node>(n, d, k);	// Создаёт Node(n,d,k) и заворачивает в unique_ptr
	Node *ptr = node.get();							// raw для навигации
													// Начинаем заполнять хранилище
	auto iter = storage.insert(storage.end(), std::move(node));
	index[k] = iter;								// Ключ k теперь ведет на итератор index, создавая новую запись
	tower[0] = ptr;									// Добавляю в памяти "на 0 этаж" вставляемый объект

	if (!current)
		return nullptr;

	// Генерация высоты

	while (result != 0 && height < max_lvl)
	{
		result = (randint() < m / 2) ? 0 : 1;		// Получаем рандом для строительства уровней
		if (result != 0)
			height++;								// Записываем количество получившихся этажей
		else break;
	}

	int lvls = height - 1;

	// Поиск + заполнение update

	for (int i = height, j = lvls; i != 0; i--)
	{
		if (!current->next)
			return nullptr;

		else if (current->next->key > k)
		{
			update[j] = current;					// Вносим в хранилище куда будем вставлять узел на текущем уровне
			
			if (current->down)
			{
				current = current->down;			// Двигаем поиск на уровень ниже
				j--;
			}
		}

		else if (current->next == tail)				// Проверяем не является ли следующим элементом конец Списка
		{
			update[j] = current;					// Добавляем в хранилище

			if (current->down)
			{
				current = current->down;
				j--;
			}
		}

		else if (current->next->key == k)			// Следующий элемент соответствует искомому
			return nullptr;							// Вернули ссылку на нyль(избегаем дублей)

		else if (current->next->key < k)
		{
			current = current->next;				// Двигаемся к следующему элементу на текущем уровне
			update[j] = current;					// Добавляем в хранилище элемент к которому перешли
		}
		else
			return nullptr;
	}

	// Создать все узлы 

	for (int j = 1; j < height; j++)
	{												// Создаём узел на 1 ур выше
			auto temp_ptr = std::make_unique<Node>(n, d, k);
			Node *raw = temp_ptr.get();				// Для навигации
													// Продолжаем заполнять хранилище
			auto temp_iter = storage.insert(storage.end(), std::move(temp_ptr));
			tower[j] = raw;							// Заполяняется башня уровнями
			raw->down = tower[j - 1];				// Строим *down связи (снизу-вверх) башни
	}

	// Вставить все узлы ч/з update

	while (lvls != -1)
	{
		if (update[lvls])							// проверим что не разыименовываем nullptr
		{
			tower[lvls]->next = update[lvls]->next;	// Строим Next связи на вставляемый объект
			update[lvls]->next = tower[lvls];
		}
		lvls--;
	}

	return tower[0];
}
//================================================================================================================
Node *Skip_List::erase(int k)
{
	Node *update[32] = {};							// Хранилище: пути поиска

	// Поиск + update[]



	// Разрыв горизонтальных линий

	// Удаление из storage

	// Коррекция потолка

	return nullptr;
}
//================================================================================================================
int main()
{
	return 0;
}
//================================================================================================================
//11. Разберитесь, что собой представляет список с пропусками(skip list), и реализуйте эту разновидность списка.
// Это не простое упражнение.
//================================================================================================================