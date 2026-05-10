#include <iostream>
#include <memory>
#include <vector>
#include <utility>
const unsigned long long m = 1ULL << 31;
//================================================================================================================
long long randint()
{
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

	Node *search(int k) const;
	Node *insert(int k);

private:
	Node *head;											// start list
	Node *tail;											// end list
	std::vector<std::unique_ptr<Node>> storage;			// хранилище всех узлов
	int max_lvl = 32;
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
Node *Skip_List::search(int k) const
{
	Node *current = head;

	if (current == nullptr)
		return nullptr;

	while (current != nullptr && current != tail)	// Цикл пока не пришли в конец списка
	{
		if (current->next == nullptr)
			return nullptr;

		if (current->next->key > k)				// Проверяем значение следующего элемента на текущем уровне
		{
			if (current->down == nullptr)
				return nullptr;
				current = current->down;			// Двигаем поиск на уровень ниже
		}

		 else if (current->next == tail)			// Проверяем не является ли следующим элементом конец Списка
		{
			if (current->down == nullptr)
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
Node *Skip_List::insert(int k)
{
	bool result = 1;
	int height = 1;
	Node *current = head;

	Node *update[32] = {};					// Хранилище: после каких узлов строится башня=(next)
	Node *tower[32] = {};					// Хранилище: для строительства этажей башни===(down)

	Node *n = nullptr;
	Node *d = nullptr;

	auto node = std::make_unique<Node>(n, d, k);// Создаёт Node(n,d,k) и заворачивает в unique_ptr

	Node *ptr = node.get();
	storage.push_back(std::move(node));

	tower[0] = ptr;							// Добавляю в памяти "на 0 этаж" вставляемый объект

	if (current == nullptr)
		return nullptr;

	// Генерация высоты

	while (result != 0 && height < max_lvl)
	{
		result = (randint() < m / 2) ? 0 : 1;	// Получаем рандом для строительства уровней
		if (result != 0)
			height++;									// Записываем количество получившихся этажей
		else break;
	}

	int lvls = height - 1;

	// Поиск + заполнение update

	for (int i = height, j = lvls; i != 0; i--)
	{
		if (current->next == nullptr)
			return nullptr;

		else if (current->next->key > k)
		{
			update[j] = current;				// Вносим в хранилище куда будем вставлять узел на текущем уровне
			
			if (current->down != nullptr)
			{
				current = current->down;			// Двигаем поиск на уровень ниже
				j--;
			}
		}

		else if (current->next == tail)				// Проверяем не является ли следующим элементом конец Списка
		{
			update[j] = current;				// Добавляем в хранилище

			if (current->down != nullptr)
			{
				current = current->down;
				j--;
			}
		}

		else if (current->next->key == k)		// Следующий элемент соответствует искомому
			return nullptr;						// Вернули ссылку на нyль(избегаем дублей)

		else if (current->next->key < k)
		{
			current = current->next;			// Двигаемся к следующему элементу на текущем уровне
			update[j] = current;				// Добавляем в хранилище элемент к которому перешли
		}

		else
			return nullptr;
	}

	// Создать все узлы 

	for (int j = 1; j < height; j++)
	{												// Создаём узел на 1 ур выше
			auto temp_ptr = std::make_unique<Node>(n, d, k);
			Node *raw = temp_ptr.get();
			storage.push_back(std::move(temp_ptr));
			tower[j] = raw;					// Заполяняется башня уровнями
			raw->down = tower[j - 1];	// Строим *down связи (снизу-вверх) башни
	}

	// Вставить все узлы ч/з update

	while (lvls != -1)
	{
		if (update[lvls] != nullptr)				// проверим что не разыименовываем nullptr
		{
			tower[lvls]->next = update[lvls]->next;	// Строим Next связи на вставляемый объект
			update[lvls]->next = tower[lvls];
		}
		lvls--;
	}

	return tower[0];
}
//================================================================================================================
int main()
{

}
//================================================================================================================
//11. Разберитесь, что собой представляет список с пропусками(skip list), и реализуйте эту разновидность списка.
// Это не простое упражнение.
//================================================================================================================