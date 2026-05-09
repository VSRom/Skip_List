#include <iostream>
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
	Skip_List(Node *h = 0, Node *t = 0)
		: head(h), tail(t)
	{
	}

	//							Skip_List build_lvl(Skip_List lvl) const;
	//							Skip_List skip_list(Skip_List l) const;
	Node *search(int k);
	Node *insert(int k);

private:
	Node *head;			// start list
	Node *tail;			// end list
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
Node *Skip_List::search(int k)
{
	Node *current = head;

	if (current == nullptr)
		return nullptr;

	while (current)
	{
		if (current->next == nullptr)
			return nullptr;

		else if (current->next->key > k)		// Проверяем значение следующего элемента на текущем уровне
		{
			if (current->down == nullptr)
				return nullptr;
			else
				current = current->down;		// Двигаем поиск на уровень ниже
		}

		 else if (current->next == tail)		// Проверяем не является ли следующим элементом конец Списка
		{
			if (current->down == nullptr)
				return nullptr;
			else
			current = current->down;			// Двигаем поиск на уровень ниже
		}

		else if (current->next->key == k)		// Следующий элемент соответствует искомому
			return current->next;				// Вернули ссылку на найденный элемент

		else if (current->next->key < k)
			current = current->next;			// Двигаемся к следующему элементу на текущем уровне

		else return nullptr;
	}
}
//================================================================================================================
Node *Skip_List::insert(int k)
{
	bool result;
	int count = 1;
	Node *current = head;

	Node *n = nullptr;
	Node *d = nullptr;

	Node *ptr = new Node(n, d, k);

	if (current == nullptr)
		return nullptr;

	while (count < max_lvl)
	{
		result = (randint() < m / 2) ? 0 : 1;

		if (current->next == nullptr)
			return nullptr;

		else if (current->next->key > k)
		{
			if (current->down == nullptr)
			{

				current->next = ptr->next;
				ptr->next = 
			}
			else
				current = current->down;		// Двигаем поиск на уровень ниже
		}

		else if (current->next == tail)			// Проверяем не является ли следующим элементом конец Списка
		{
			if (current->down == nullptr)
				return nullptr;
			else
				current = current->down;		// Двигаем поиск на уровень ниже
		}

		else if (current->next->key == k)		// Следующий элемент соответствует искомому
			return nullptr;						// Вернули ссылку на нyль

		else if (current->next->key < k)
			current = current->next;			// Двигаемся к следующему элементу на текущем уровне



		else return nullptr;
	}
}
//================================================================================================================
//			Skip_List Skip_List::build_lvl(Skip_List lvl) const
//			{
//				Skip_List next_lvl = {};
//				next_lvl.head->down = lvl.head;
//				next_lvl.tail->down = lvl.tail;
//			
//				Node *i = lvl.head->next->next;
//				Node *cur = next_lvl.head;
//			
//				while (i != nullptr && i->next != nullptr)
//					cur->next = static_cast<Node *>(key, i, cur->next);
//			
//				cur = cur->next;
//				i = i->next->next;
//			
//				return next_lvl;
//			}
//================================================================================================================
//			Skip_List Skip_List::skip_list(Skip_List l) const
//			{
//				Skip_List lvl;
//				Node *i = l.head;
//				Node *j = lvl.head;
//			
//				while (j != l.tail)
//				{
//					i->next = static_cast<Node *>(j->key, 0, j->next);
//					i = i->next;
//					j = j->next;
//				}
//			
//				while (lvl.size() > 2)
//					lvl = build_lvl(lvl);
//			
//				return lvl;
//			}

//================================================================================================================
int main()
{

}
//================================================================================================================
//11. Разберитесь, что собой представляет список с пропусками(skip list), и реализуйте эту разновидность списка.
// Это не простое упражнение.
//================================================================================================================