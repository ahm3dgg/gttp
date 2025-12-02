#pragma once

#include "Types.hpp"
#include "Defines.hpp"

/// Singly Linked List Queue

#define QueueEntry(type) struct { type* next; }
#define Queue(type) struct { type* head = nullptr; type* tail = nullptr; u64 length = 0; }
#define QueuePush(qhead, field, elem)			\
do {											\
	if (qhead.head == nullptr) {				\
		qhead.head = qhead.tail = (elem);		\
	} else {									\
		qhead.tail->field.next = (elem);		\
		qhead.tail = (elem);					\
	}											\
	(elem)->field.next = nullptr;				\
	qhead.length++;								\
} while(0)								

#define QueuePop(qhead, field) qhead.head; if (qhead.head != nullptr) { qhead.head = qhead.head->field.next; qhead.length--; }
#define QueueLength(qhead)	(qhead.length)
#define QueueEmpty(qhead) (qhead.length == 0)
#define QueueIter(type, qhead, elem) for(type* elem = qhead.head; elem != nullptr; elem = elem->next)

/// Singly Linked List

#define Sll_N(name, type) struct name { type* head = nullptr; type* tail = nullptr; u64 length; }
#define Sll(type) struct { type* head = nullptr; type* tail = nullptr; u64 length; }
#define SllPush(shead, field, elem)				\
do {											\
	if (shead.head == nullptr) {				\
		shead.head = shead.tail = elem;			\
	} else {									\
		shead.tail->field = elem;				\
		shead.tail = elem;						\
	}											\
	elem->field = nullptr;						\
	shead.length++;								\
} while(0)

#define SllLength(shead) (shead->length);
#define SllIter(shead, field, elem)	for(typeof(shead.head) elem = shead.head; elem != nullptr; elem = elem->field)
#define SllEmpty(shead) (shead->length == 0)

/// Doubly Linked List

#define DllEntry(type) struct { type* prev; type* next; }
#define Dll(type) struct { DllEntry(type) head = {nullptr, (type*)&head}; type* tail = nullptr; size_t length = 0; }
#define DllInit(dhead)													\
	(dhead).head.prev = nullptr;										\
	(dhead).head.next = rcast<typeof((dhead).head.next)>(&dhead.head);	\
	(dhead).tail = nullptr; (dhead).length = 0;							\

#define DllPush(dhead, field, entry)																				\
do {																												\
	if((dhead).head.next == rcast<typeof((dhead).head.next)>( &((dhead).head) )) {									\
		(dhead).head.next = (entry);																				\
		(entry)->field.prev = rcast<typeof((dhead).head.prev)>( &((dhead).head) );									\
		(entry)->field.next = rcast<typeof((dhead).head.next)>( &((dhead).head) );									\
		(dhead).tail = (entry);																						\
	} else {																										\
		(entry)->field.next = (dhead).tail->field.next;																\
		(entry)->field.prev = (dhead).tail;																			\
		(dhead).tail->field.next = (entry);																			\
		(dhead).tail = (entry);																						\
	}																												\
	(dhead).length++;																								\
} while (0)

#define DllRemove(dhead, field, entry)								\
do {																\
	(entry)->field.prev->field.next = (entry)->field.next;			\
	(entry)->field.next->field.prev = (entry)->field.prev;			\
	((entry)->field.next == rcast<typeof((entry)->field.next)>( &((dhead).head) )) ? (dhead).tail = (entry)->field.prev : (dhead).tail = (dhead).tail;	\
	(dhead).length--;		\
} while(0)

#define DllIter(dhead, field, entry) for(typeof((dhead).head.next) (entry) = (dhead).head.next; (entry) != rcast<typeof( entry )>( &((dhead).head) ); (entry) = (entry)->field.next)
#define DllEmpty(dhead) ( (dhead).head.next == rcast<typeof((dhead).head.next)>( &((dhead).head) ) )
#define DllPop(dhead, field) (dhead).tail; DllRemove(dhead, field, (dhead).tail)

#define Array(name, type) struct name { type* entries; size_t pos; size_t length; }
#define ArrayAppend(list, entry) do { (list).entries[(list).pos] = (entry); (list).pos++; (list).length++; } while (0)
#define ArrayGet(list, index) (list).entries[index]
#define ArrayLength(list) (list).length